# Fix triage for the 2026-10-03 derivation audit

This table sorts every finding in `2026-10-03-derivation-audit.md` into the fix it belongs to. It covers findings 1 to 352 and round-7 entries R7.1 to R7.44, which is 396 rows. The seven round-7 notes R7.45 to R7.51 are not findings; they were used to decide what is still open.

Three agents built it on 2026-10-03 at HEAD 02976c1, each taking a third of the report. They did not re-verify the findings. They only checked whether the 32 commits since the audit snapshot changed each one. Nothing in the code was edited.

## The short version

362 findings are still open as written. 32 are still open, but their code moved or gained copies since the audit. One is half fixed (227) and one is fixed (269).

Almost half the findings are duplicates that agree today: 182 of them. Folding those changes nothing you see. The engine-facts step has 24 findings, the parser step 21 and display formatting 49. Docs have 44 and the shell path-length owner has 1. Another 75 are real fixes that fit none of the audit's steps. They are in the store, audio, CLI, tools and search rules.

The full rows are also in `2026-10-03-fix-triage.json` next to this file, for scripts.

133 findings need a call from you before their fix can be planned. 33 of those are weighty: the answer changes scores, ratings, paths or stored results, or needs checking against Clone Hero. The rest are wording, colour or edge-case choices. Each of those will come with a recommended answer when its plan is written.

15 fixes would change stored analysis results, so records would need re-analysis, and 21 more might. Every other fix leaves stored results alone.

The existing plan `2026-10-03-one-squeeze-rating-rule.md` fully covers findings 24, 25, 26, 137, 303 and partly covers 33, 94, 127, 144, 259. It does not fix 27, 28 or 29, which are the other step-1 squeeze facts.

## Weighty decisions, by the step they block

These are the questions to settle before planning. Each line gives the finding number and the question.

### Step 1: let the engine store facts the displays guess

- **28** (Cap-clamped windows measure the ratio from the activation, not the collecting note): When the SP cap clamps a window, should the squeeze scale be measured from the collecting note (as the video proof implies) rather than the activation?
- **32** (Replay re-decides which chords SP pays; doubles a squeezed-out 0-SP chord): Should the replay's own walk stay as an allowed second scorer, and should a squeezed-out chord that earns 0 SP show the doubled multiplier disc?
- **90** (A path folded mid-SP keeps a stale SP end, clamp note and phrases): When a tied path is folded into its leader while Star Power is still running, should the engine stop folding there, or give the variant the leader's closed activation?
- **332** (Unknown transfer ratios are silently stored as x1.00): Should a stored record be able to say 'transfer scales unknown' (as ADR 0011 says) or keep storing x1.00 and ADR 0011 gets corrected?
- **R7.10** (For SP that outlasts the chart, graph and display apply the 500 ms window from different ): For the last activation of a path whose Star Power outlasts the chart, should backend rows be limited to within 500 ms of the SP end (as the display does) or of the last note (as the graph does)?

### Step 2: parser drifts

- **11** (Disco flip at Hard and below follows Expert's disco markers): Does Clone Hero apply disco flip per difficulty (mix 2 for Hard) or only from Expert's markers?
- **21** (.chart and .mid parsers disagree on when a Star Power phrase ends): Which edge does Clone Hero use to end a Star Power phrase: a chord exactly on the end tick, a zero-length phrase, a phrase past the last note?
- **53** (Generated fill length reads the meter one change late with two changes between chords): When two meter changes fall between two chords, which meter does Clone Hero use for the generated fill length: the current one or the one before the latest change?
- **64** (A late dynamics tag leaves early notes plain while the tab says dynamics are on): If [ENABLE_CHART_DYNAMICS] appears after some notes, does Clone Hero price the earlier notes as ghost or accent, or only the notes after the tag?
- **141** (.chart solo and disco events match raw text; section names strip quotes): How does Clone Hero read a quoted event such as E "solo": as a solo, or as a plain event?
- **314** (A beat is always a quarter note in every meter, with no named rule): Does Clone Hero count fill beats as quarter notes in compound meters like 6/8 and 7/8, as Hydra assumes?
- **315** (Fill placement tie-breaks, truncation and silent drops have no user decision): Confirm the fill rules Hydra applies today: ties go to the later chord, distances are cut to whole ticks, one authored fill stops fill generation, and a fill ending before its start is dropped.
- **320** (Dynamics: .chart files always count ghosts and accents and report dynamics on): Does Clone Hero score .chart ghost and accent notes without any enabling tag, as Hydra assumes?
- **R7.6** (Lookups assume ms never fall as ticks rise, and a negative .chart tempo breaks that): Should a .chart tempo of zero or below be rejected at load (with a clear error) or clamped to a positive value?

### Step 3: one owner per display rule

- **7** ('Optimal' means one path in the report but every tied path in Paths and Preview): When two paths tie for the top score, are both 'optimal' (as in Paths and Preview) or only the first (as in the report)?
- **306** (Backend rating bands start at a fixed 10 ms nobody chose): Should the inner rating-band edges stay at a fixed 10 ms, or follow the hit window or the leeway setting?
- **333** (Timing tier ladder shape has no recorded user decision): Is the timing tier ladder right: a 2 ms floor, then W/2, W, 3W/2 and 2W, then Beyond?

### Other real fixes (store, audio, CLI, tools, search rules)

- **22** (Free squeezes still show a timing figure, though docs promise nothing): Is a squeeze at exactly 0.0 ms free or does it need earning, and should a free squeeze show a timing figure at all?
- **23** (Squeeze filter and Path limit also count required early fills; docs say squeezes only): Should 'hardest timing' for the squeeze filter and the Path limit include required early fills, or squeezes only?
- **37** (Deact edge rebuilds the SP extension without the cap, hiding squeeze-outs): When a collected phrase fills the meter past the cap, should the SP end be clamped to the cap ceiling for the squeeze-out edge too (as extend_deacts already does)?
- **49** (Multiplier squeeze check uses its own combo list and misses 4- and 5-note chords): Should the Multiplier squeeze list include 4- and 5-note chords that straddle a multiplier step, or stay limited to the fixed combo list?
- **65** (Results made under other rules are deleted, though the docs promise they come back): Should results made under other rules stay in the database so switching the rules back brings them back, or should they be deleted and the docs rewritten?
- **77** (Probe scripts disagree on the total hit-window cap: 171.43 ms vs 170 ms): What is Clone Hero's real whole hit-window cap: 171.43 ms (as measured) or 2 x the back window (170 ms)?
- **95** (Tie limit counts per score and limit side, not per score as the guide says): Should max_tied_paths limit all tied paths at one score together (drop the 'over the Path limit' bit from the tie key), or per side with the guide corrected?
- **222** ('100 means base speed' typed four times, and the status line ignores it): Does a playback speed other than 100% change a Clone Hero score, and should an off-speed run count as 'above optimal' on the leaderboard page?
- **304** (A note exactly 3.0 ms after SP end counts as uncounted, with no decision): Should a backend note sitting exactly on the 3 ms leeway edge still count (inclusive), or not (as now)?
- **307** (An exact tie with the cap ceiling is not counted as a clamp): When the cap ceiling and the plain +2 measures land on the same tick, should that count as an overfill clamp (as the squeeze anchor sees it)? Today it does not.
- **313** (60 ms early-fill window has no recorded user decision for its value): Is 60 ms the right early-fill window (it was narrowed from 85 using a corpus check, hardest real E0 is 57.7 ms)?
- **316** (Tied paths merge without comparing when their meter reached 2 bars): Should the path-merge key include the moment the meter reached 2 bars (sp_ready_ms), or stay as now with a recorded decision?
- **330** (Path limit keeps an over-limit path when it ties the optimal score): Should a path that ties the optimal score be kept even when its hardest squeeze is over the Path limit (as now)? If yes, the User Guide just needs to say so.
- **343** (Stored tempo map is written once and never refreshed or stamped): When a chart is re-analyzed, should the stored tempo map be rewritten every time, or given its own version stamp?
- **344** (Results stamp's bump rule leaves out the parser; the dynamics rule includes it): Should a change in src/parse that alters chart output require bumping kResultsStamp (so records re-analyze), like the dynamics stamp already does?

### Step 4: docs, comments and recorded decisions

- **R7.31** (Path limit is on at 10 ms and backend display limit defaults to 50 ms, with no recorded d): Do you confirm the defaults of a 10 ms Path limit (on) and a 50 ms backend display limit (off until ticked), so they can be recorded?

## Fix groups by step

Each row is one coherent fix. Findings that one change would fix share a row. "Screen" counts the findings whose fix changes something you see. "Results" counts the ones that would change stored results (yes, or unsure in brackets). "Calls" counts the decisions needed, with weighty ones in brackets.

### Step 1: let the engine store facts the displays guess (24 findings, 12 groups)

| Group | Findings | Screen | Results | Calls |
|---|---|---|---|---|
| `squeeze-rating-one-rule` | 24, 25, 26, 137, 144, 259, 303 | 5 |  |  |
| `tied-variant-own-state` | 89, 90, 97 | 3 | 3 | 1 (1) |
| `gauge-phrase-collection` | 5, 147 | 2 | 1 (1) |  |
| `pre-sqin-sp-end-stored` | 27, 156 | 1 | 1 (1) |  |
| `replay-sp-paid-chords` | 1, 32 | 2 |  | 2 (1) |
| `sqout-offset-stored-once` | 127, 128 |  |  (2) |  |
| `backend-sqout-claim` | 29 | 1 | 1 |  |
| `cap-clamp-anchor` | 28 | 1 | 1 | 1 (1) |
| `offered-fills-stored` | 52 | 1 | 1 |  |
| `replay-sqout-chord` | 145 |  |  |  |
| `squeeze-window-500ms` | R7.10 | 1 | 1 | 1 (1) |
| `transfer-scale-unknown` | 332 | 1 |  (1) | 1 (1) |

### Step 2: parser drifts (21 findings, 16 groups)

| Group | Findings | Screen | Results | Calls |
|---|---|---|---|---|
| `lower-diff-dynamics` | 10, 11, 64, 255 | 3 | 2 | 2 (2) |
| `time-signature-zero` | 100, 258, 319 | 2 |  | 2 |
| `chart-dynamics-gate` | 320 | 1 |  (1) | 1 (1) |
| `chart-event-normaliser` | 141 | 1 |  (1) | 1 (1) |
| `chart-modifier-no-note` | 331 | 1 |  | 1 |
| `container-entry-format` | 61 | 1 |  | 1 |
| `disco-flip-gate` | 250 |  |  |  |
| `dynamics-totals` | 12 | 1 |  | 1 |
| `fill-length-meter-walk` | 53 | 1 | 1 | 1 (1) |
| `fill-placement-rules` | 315 | 1 |  (1) | 1 (1) |
| `negative-tempo` | R7.6 | 1 |  (1) | 1 (1) |
| `offset-delay-parse` | R7.5 | 1 |  | 1 |
| `practice-sections-sorted` | R7.7 | 1 |  |  |
| `sp-phrase-end-parsers` | 21 | 1 | 1 | 1 (1) |
| `ticks-per-beat` | 314 | 1 |  (1) | 1 (1) |
| `unknown-artist-owner` | 60 | 1 |  | 1 |

### Step 3: one owner per display rule (49 findings, 35 groups)

| Group | Findings | Screen | Results | Calls |
|---|---|---|---|---|
| `rating-tiers` | 2, 34, 94, 306, 333 | 5 |  | 5 (2) |
| `ms-rounding` | 4, 18, 36, 154 | 4 |  | 3 |
| `sp-colours` | 16, 136, 218 | 3 |  | 3 |
| `percent-rounding` | 51, 302 | 2 |  | 1 |
| `preview-stale-chart-mode` | 20, 249 | 1 |  |  |
| `rich-text-strip` | 8, 111 | 2 |  |  |
| `stale-message` | 13, R7.4 | 2 |  |  |
| `time-format` | 318, R7.1 | 2 |  | 1 |
| `avg-multiplier-empty` | 310 | 1 |  | 1 |
| `count-noun-helper` | 14 | 1 |  |  |
| `dm-statuses` | 309 | 1 |  | 1 |
| `drum-note-names` | 17 | 1 |  | 1 |
| `early-fill-badge` | 305 | 1 |  | 1 |
| `ellipsis-rule` | 70 | 1 |  |  |
| `empty-report-reason` | 105 | 1 |  |  |
| `fill-compare-label` | 312 | 1 |  | 1 |
| `fill-rule-names` | 55 | 1 |  | 1 |
| `ms-to-tick` | 57 | 1 |  |  |
| `no-notes-message` | 71 | 1 |  |  |
| `number-grouping` | 99 | 1 |  | 1 |
| `optimal-meaning` | 7 | 1 |  | 1 (1) |
| `preview-beat-grid-end` | 329 | 1 |  | 1 |
| `preview-gauge-cap` | 308 | 1 |  | 1 |
| `progress-fraction` | 69 | 1 |  | 1 |
| `report-search-rule` | 67 | 1 |  | 1 |
| `report-tile-label` | 35 | 1 |  | 1 |
| `search-active-rule` | 19 | 1 |  |  |
| `song-length` | 9 | 1 |  | 1 |
| `song-panel-stale-result` | 123 | 1 |  |  |
| `status-words` | 87 | 1 |  |  |
| `struck-at-now` | 15 | 1 |  | 1 |
| `timeline-orange-rule` | 3 | 1 |  | 1 |
| `typed-error-kinds` | R7.20 | 1 |  |  |
| `ui-timings` | R7.33 | 1 |  | 1 |
| `uncounted-tag` | 33 | 1 |  | 1 |

### Other real fixes (store, audio, CLI, tools, search rules) (75 findings, 58 groups)

| Group | Findings | Screen | Results | Calls |
|---|---|---|---|---|
| `ch-probe-tools` | 59, 77, 78, 79, 80, 81, 82, 83, 84, 85, 174 | 2 |  | 8 (1) |
| `stamp-rules` | 343, 344, 345 | 3 |  (1) | 3 (2) |
| `multiplier-squeeze-rule` | 49, 50 | 2 |  | 1 (1) |
| `settings-validation` | 311, 322 | 2 |  | 2 |
| `squeeze-window-500ms` | 30, 138 | 2 |  (1) | 2 |
| `stem-length-contract` | R7.8, R7.9 | 2 |  | 2 |
| `store-columns-vs-blob` | 132, 133 | 2 |  | 1 |
| `audio-magic-bytes` | 74 | 1 |  |  |
| `auto-open-report` | R7.2 | 1 |  | 1 |
| `backend-limit-normaliser` | 31 | 1 |  |  |
| `batch-counts` | 142 | 1 |  |  |
| `batch-settings-snapshot` | 124 | 1 |  |  |
| `batch-skip-count-one-source` | 125 | 1 |  |  |
| `bench-dump-paths` | 108 | 1 |  |  |
| `can-scan-owner` | R7.3 | 1 |  | 1 |
| `cap-clamp-tie` | 307 | 1 |  (1) | 1 (1) |
| `chartmode-key-from-parsed-difficulty` | 134 |  |  |  |
| `deact-edge-cap-ceiling` | 37 | 1 | 1 | 1 (1) |
| `depth-mode-normalise` | 56 | 1 |  |  |
| `dm-base-speed` | 222 | 1 |  | 1 (1) |
| `dm-report-rules` | 170 | 1 |  |  |
| `duplicate-md5-name` | 63 | 1 |  | 1 |
| `early-fill-window` | 313 | 1 |  (1) | 1 (1) |
| `every-path-sentinel` | 86 | 1 |  | 1 |
| `fill-rule-stamp` | 54 | 1 |  | 1 |
| `free-squeeze-rule` | 22 | 1 |  (1) | 1 (1) |
| `hardest-timing-meaning` | 23 | 1 |  (1) | 1 (1) |
| `hit-window-type` | 140 | 1 |  | 1 |
| `ini-line-reader` | 66 | 1 |  | 1 |
| `leeway-edge` | 304 | 1 |  | 1 (1) |
| `library-query-squeeze` | 323 | 1 |  | 1 |
| `library-row-identity` | 143 | 1 |  | 1 |
| `measure-position-meter-tick` | 58 | 1 |  | 1 |
| `no-path-best-score` | 98 | 1 |  | 1 |
| `parse-bool` | 68 | 1 |  | 1 |
| `path-limit-optimal-tie` | 330 | 1 |  (1) | 1 (1) |
| `preview-chart-md5-check` | 126 | 1 |  | 1 |
| `preview-transport-play-state` | 73 | 1 |  |  |
| `preview-volume-range` | 72 | 1 |  |  |
| `ready-no-paths` | 88 | 1 |  | 1 |
| `record-decode-helper` | 116 |  |  |  |
| `replay-defaults-from-settings` | 198 | 1 |  |  |
| `replay-json-missing-fields` | 38 |  |  | 1 |
| `replay-legacy-fills-key` | 103 | 1 |  |  |
| `report-file-name` | 202 | 1 |  | 1 |
| `results-key-columns-one-source` | 130 |  |  |  |
| `retired-auto-cleanup` | 317 | 1 |  | 1 |
| `rules-file-ranges` | 325 | 1 |  | 1 |
| `rules-rows-purge` | 65 | 1 |  (1) | 1 (1) |
| `scan-metadata-cap` | 321 | 1 |  | 1 |
| `song-mix-stem-rule` | 101 | 1 |  | 1 |
| `songmeta-length-per-chartmode` | 62 | 1 |  (1) | 1 |
| `sp-cap-minimum` | 139 | 1 |  | 1 |
| `stars-cache-guard` | 129 |  |  | 1 |
| `tie-limit-key` | 95 | 1 | 1 | 1 (1) |
| `tie-merge-key` | 316 | 1 |  (1) | 1 (1) |
| `typed-error-kinds` | 193 | 1 |  | 1 |
| `uitest-idle-jobs` | 109 | 1 |  |  |

### Step 4: docs, comments and recorded decisions (44 findings, 6 groups)

| Group | Findings | Screen | Results | Calls |
|---|---|---|---|---|
| `adr-corrections` | 44, 45, 46, 47, 48, 92, 102, R7.37, R7.38, R7.39, R7.40, R7.41, R7.42, R7.43, R7.44 | 1 |  | 1 |
| `record-decisions` | 327, 336, 337, 338, 339, 348, 349, 351, R7.31, R7.32, R7.34, R7.36 | 2 |  (1) | 10 (1) |
| `user-guide` | 39, 40, 42, 43, 91, 93, 96, 104 | 8 |  | 1 |
| `cli-messages` | 41, 106, 107, 110, 326 | 5 |  | 2 |
| `stale-comments` | 114, 115, 118 |  |  |  |
| `tooltip-wording` | 6 | 1 |  |  |

### Shell path-length owner (1 findings, 1 groups)

| Group | Findings | Screen | Results | Calls |
|---|---|---|---|---|
| `shell-path-owner` | R7.12 |  |  | 1 |

### Last step: fold duplicates that agree today (182 findings, 106 groups)

| Group | Findings | Screen | Results | Calls |
|---|---|---|---|---|
| `tests-recompute-production` | 119, 120, 240, 263, 264, 265, 266, 267, 270, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 293, 294, 295, 296, 297, 300, 301, 341, 350, 352, R7.30 |  |  | 1 |
| `ch-probe-tools` | 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 271, 272, 273, 328, 342 | 12 |  | 2 |
| `allzero-one-definition` | 180, 243, 245, 261, 324 | 1 |  | 1 |
| `backend-offset` | 148, 149, 346, R7.35 |  |  |  |
| `track-state-spans` | 76, R7.13, R7.14, R7.15 |  |  |  |
| `ui-label-functions` | 289, 290, 291, 292 |  |  |  |
| `multiplier-squeeze-rule` | 247, 248, 335 |  |  | 1 |
| `path-difficulty-one-owner` | 151, 152, 153 |  |  |  |
| `store-sql-owners` | 252, 253, 299 |  |  |  |
| `audio-ms-frames` | 182, R7.21 |  |  |  |
| `chart-format-one-owner` | 187, R7.17 |  |  |  |
| `graph-build-cap` | 246, 260 |  |  |  |
| `ms-to-tick` | 183, 184 |  |  |  |
| `note-value-one-formula` | 160, 161 |  |  |  |
| `shared-test-equality-helpers` | 121, 122 |  |  |  |
| `store-blob-header` | 194, 195 |  |  |  |
| `app-identity-constants` | 257 |  |  |  |
| `ascii-string-helpers` | 201 |  |  |  |
| `bass2x-available` | 244 |  |  |  |
| `batch-running-owner` | 254 |  |  |  |
| `beyond-edge-one-owner` | 155 |  |  |  |
| `button-width-helper` | 213 |  |  |  |
| `chart-hash-normalise` | 192 |  |  |  |
| `combo-walk-one-owner` | 164 |  |  | 1 |
| `dead-sql-search` | 113 |  |  |  |
| `default-depth-constant` | 181 |  |  |  |
| `dpi-scale-one-owner` | 210 |  |  |  |
| `dynamic-names` | 241 |  |  |  |
| `dynamics-count-one-accessor` | 205 |  |  |  |
| `dynamics-keys` | 262 |  |  |  |
| `early-fill-window` | 176 |  |  |  |
| `file-check-interval` | 207 |  |  | 1 |
| `fill-attach-one-check` | 178 |  |  |  |
| `fill-rule-names` | 177 |  |  |  |
| `fill-rule-stamp` | R7.26 |  |  |  |
| `fits-on-line-helper` | 214 |  |  |  |
| `format-measure-owner` | R7.27 |  |  |  |
| `gain-clamp-owner` | 225 |  |  |  |
| `gauge-phrase-collection` | 159 |  |  |  |
| `highway-far-time` | 224 |  |  |  |
| `id3-sniff` | R7.23 |  |  |  |
| `job-fail-helper` | 212 |  |  |  |
| `keep-everything-band` | R7.29 |  |  |  |
| `lane-flag-predicate` | 190 |  |  |  |
| `library-count-one-source` | 131 |  |  |  |
| `library-share-valid` | 211 |  |  |  |
| `midi-pitch-table` | R7.19 |  |  |  |
| `mp3-reservoir` | R7.24 |  |  |  |
| `notes-file-pick` | 186 |  |  |  |
| `obj-fan-helper` | 226 |  |  |  |
| `opus-rate-constant` | 227 |  |  |  |
| `parent-folder-helper` | 251 |  |  |  |
| `path-activation-order` | 268 |  |  |  |
| `paths-count-one-method` | 172 |  |  |  |
| `preview-config-one-source` | 220 |  |  |  |
| `preview-file-size` | R7.25 |  |  |  |
| `preview-gauge-cap` | 347 |  |  |  |
| `preview-jump-constants` | 215 |  |  |  |
| `preview-overlay-key` | 340 |  |  |  |
| `preview-overlay-sizes` | R7.18 |  |  |  |
| `preview-playhead-clamp` | 185 |  |  |  |
| `preview-scene-timing-one-copy` | 135 |  |  |  |
| `preview-sp-curve-predicate` | 216 |  |  |  |
| `preview-sp-running-predicate` | 157 |  |  |  |
| `preview-taken-fill-index` | 175 |  |  |  |
| `preview-track-options` | R7.16 |  |  |  |
| `read-file-bytes-owner` | R7.11 |  |  |  |
| `record-key-identity` | 197 |  |  |  |
| `report-chart-count` | 242 |  |  |  |
| `report-path-order` | 169 |  |  |  |
| `report-status-one-owner` | 168 |  |  |  |
| `resource-folder-helper` | 209 |  |  |  |
| `rules-field-table` | 199 |  |  |  |
| `scan-item-conversion` | 256 |  |  |  |
| `search-term-applies` | 204 |  |  |  |
| `settings-description-helper` | 206 | 1 |  |  |
| `settings-key-table` | 200 |  |  |  |
| `shell-execute-ok` | 217 |  |  |  |
| `shell-path-owner` | 269 |  |  |  |
| `solo-bonus-helper` | 163 |  |  |  |
| `solo-sections-data` | 167 |  |  |  |
| `song-ini-pick` | 189 |  |  |  |
| `song-length` | 191 |  |  |  |
| `sp-activation-minimum` | 334 |  |  | 1 |
| `sp-cap-minimum` | 158 |  |  |  |
| `sp-multiplier-one-owner` | 162 |  |  |  |
| `sqout-cost-one-helper` | 150 |  |  |  |
| `sqout-position-one-test` | 146 |  |  |  |
| `squeeze-kind-name` | 203 |  |  |  |
| `srb-layout-one-walk` | 188 |  |  |  |
| `stale-reason-one-evaluation` | 196 | 1 |  |  |
| `stars-range-text` | 171 |  |  |  |
| `stars-with-solo` | 166 |  |  |  |
| `status-fade-constant` | 219 | 1 |  | 1 |
| `stem-converter-config` | R7.28 |  |  |  |
| `store-schema-number` | 298 |  |  | 1 |
| `test-fixture-sng-srb` | 117 |  |  |  |
| `tied-count-one-owner` | 179 |  |  |  |
| `toggle-on-helper` | 223 |  |  |  |
| `total-score-one-sum` | 165 |  |  |  |
| `track-rect-helper` | 221 |  |  |  |
| `ui-font-size-constant` | 208 |  |  |  |
| `widest-digits` | 112 | 1 |  |  |
| `within-n-band-test` | 173 |  |  |  |
| `worker-count` | R7.22 |  |  | 1 |
| `wrap-words-widest` | 75 |  |  |  |

## Every finding

Status is at HEAD 02976c1. Kind: drift, double reading, duplicate that agrees today, undecided assumption, or doc. Vis is the report's visibility rank, 1 being on screen today.

| Id | Kind | Vis | Step | Group | Status | Squeeze plan | Results | Title | Owner after the fix | What you'd see change | Call needed |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 1 | drift | 1 | 1 | `replay-sp-paid-chords` | changed | no | no | After SP ends, score box still shows doubled while drain box says idle | core/backend_value.h shared time-in-window predicate beside paid_by_sp_walk | Between the SP end and the next chord the score box would stop showing the doubled multiplier. | (low) Should the Preview score box drop back to the plain multiplier the moment Star Power ends, or keep the doubled value until the next chord? |
| 2 | drift | 1 | 3 | `rating-tiers` | open | no | no | Report footer says tiers match squeeze ratings; they use different ladders | core/squeeze_rating (one SqOut ladder) or a reworded footer in report.cpp | A 5 ms SqOut would read the same in the report and the Paths row, or the footer would stop saying they match. | (low) Should the report's SqOut tiers and the Paths-tab row labels use one shared ladder, or should the report footer just stop claiming they match? |
| 3 | drift | 1 | 3 | `timeline-orange-rule` | open | no | no | Timeline outlines any badged activation in orange; the row badge stays grey | ActivationRowView::difficult driving both colours (app/path_view) | SqIn and early-fill activations would lose the orange timeline outline (or gain an orange badge on the row). | (low) Should the Paths timeline outline go orange only for difficult activations (like the row badge), or for any badged one? |
| 4 | drift | 1 | 3 | `ms-rounding` | open | no | no | One squeeze reads 13 ms on the badge but 12 ms when copied | app/display_format (one whole-ms formatter) | Copied path text would show the same whole-ms number as the badge (or the reverse). | (low) Should whole-ms squeeze times round to nearest or cut off the fraction? |
| 5 | drift | 1 | 1 | `gauge-phrase-collection` | open | no | yes | SP gauge shows a full bar after a late SqIn; the engine gives less | engine: store the SP end after each collection (ScoreGraph::store_new_backend / extend_deacts) | On a late SqIn the gauge would read 0.875 bars instead of 1.0 and drain to the old SP end. |  |
| 6 | drift | 1 | 4 | `tooltip-wording` | open | no | no | Report tooltip calls the average multiplier 'points per note' | Path::avg_mult comment (core/model.h) and the report.cpp tooltip | The Avg multiplier column tooltip would stop saying 'points per note'. |  |
| 7 | drift | 1 | 3 | `optimal-meaning` | open | no | no | 'Optimal' means one path in the report but every tied path in Paths and Preview | core/model HydraRecord: one optimal-set rule | The report's 'Best path only' view would keep or drop tied variants to match the Paths tab. | (high) When two paths tie for the top score, are both 'optimal' (as in Paths and Preview) or only the first (as in the report)? |
| 8 | drift | 1 | 3 | `rich-text-strip` | open | no | no | Reports and the app strip Clone Hero text tags differently | app::strip_rich_tags in src/app/library_query.cpp; report::plain calls it | Reports would stop showing raw <b> and <size> tags in titles; a bare <color> tag would be cleaned in the app too. |  |
| 9 | drift | 1 | 3 | `song-length` | changed | no | no | Paths timeline and Preview scrubber use different song lengths | store::song_length_ms plus one named audio-tail rule in PreviewTransport; one fraction helper | Activation marks would sit at the same place on the Paths timeline and the Preview scrubber. | (low) Should the song's end be the last note or the end of the audio, for both the Paths timeline and the Preview scrubber? |
| 10 | drift | 1 | 2 | `lower-diff-dynamics` | changed | no | no | Dynamics tab reads Expert's 2x kicks at Hard, Medium and Easy | MidiParser in src/parse/song.cpp (Expert-only gate on pitch 95) | At Hard and below the Dynamics tab would lose its 2x kick row and show the true kick count. |  |
| 11 | drift | 1 | 2 | `lower-diff-dynamics` | changed | no | yes | Disco flip at Hard and below follows Expert's disco markers | src/parse/song.cpp (disco match built from the parser's one difficulty choice) | At Hard and below with Pro Drums, red and yellow gems would flip only where that difficulty's own marker says. | (high) Does Clone Hero apply disco flip per difficulty (mix 2 for Hard) or only from Expert's markers? |
| 12 | drift | 1 | 2 | `dynamics-totals` | open | no | no | Dynamics tab counts 2x kicks in one kick total and drops them in another | src/app/dynamics_breakdown.cpp: one kick total taking the 2x Bass setting, one DynamicsCounts addition | The Kicks table and the Totals box would agree on the kick count. | (low) With 2x Bass off, should the All kicks row on the Dynamics tab leave out 2x kicks like the Totals box does? |
| 13 | drift | 1 | 3 | `stale-message` | open | no | no | The Stale explanation is typed four times in three wordings | app/user_messages fed by store::stale_reasons | One wording for 'out of date' everywhere, naming whether the cause is the Hydra version or the rules file. |  |
| 14 | drift | 1 | 3 | `count-noun-helper` | open | no | no | Count-with-noun rule retyped many times; SP cap 1 shows '1 bars' | one counted() helper beside group_thousands (report::counted, count_label folded in) | '1 bars' becomes '1 bar' and large counts gain commas everywhere (1000 becomes 1,000), including hydra_batch output. |  |
| 15 | drift | 1 | 3 | `struck-at-now` | open | no | no | Note exactly at the playhead: score box says hit, highway says not yet | one app-level 'struck at now' predicate in src/app/preview_view | After jumping to an activation the highway gems and the score box agree. | (low) When the playhead sits exactly on a note, should the highway show it already hit (as the score box does) or not yet? |
| 16 | drift | 1 | 3 | `sp-colours` | open | no | no | Running Star Power is teal on the highway and gold in the drain box | one named SP colour constant (src/ui/preview_tab.cpp or theme.h) tied to 3d-config.json | The highway floor and the drain box would use one colour while SP runs. | (low) Which colour should mean 'Star Power is running' in the Preview: teal or gold? |
| 17 | drift | 1 | 3 | `drum-note-names` | open | no | no | Same drum note has two names: Paths and Preview versus Dynamics tab | one naming function in core beside color_str / ChordNote::str; dynamics_row_label built from it | A drum note would read the same on the Paths tab, the Preview and the Dynamics tab. | (low) Should the Paths and Dynamics tabs use the same words for a drum note (for example 'Green tom' or 'GreenTom'), or keep two vocabularies? |
| 18 | drift | 1 | 3 | `ms-rounding` | open | no | no | Hardest squeeze shown as whole ms on the badge, one decimal elsewhere | app/display_format (one formatter per approved style) | The badge would read 12.4 ms instead of 12 ms, or stay whole with a shared formatter behind it. | (low) Should the activation badge keep whole milliseconds, or show one decimal like the path button? |
| 19 | drift | 1 | 3 | `search-active-rule` | open | no | no | Analyze button says 'search' when the typed search filters nothing | LibraryQuery::empty() in src/app/library_query.cpp | Typing 'stars:9' would leave the button reading 'Analyze library' instead of 'Analyze search (N)'. |  |
| 20 | drift | 1 | 3 | `preview-stale-chart-mode` | open | no | no | Preview keeps the old chart mode's notes after a difficulty/Pro Drums/2x Bass change | PreviewController::open: one parsed-song key built from the same inputs as Settings::to_analysis_settings | Changing difficulty, Pro Drums or 2x Bass would reload the Preview with the new mode's notes. |  |
| 21 | drift | 1 | 2 | `sp-phrase-end-parsers` | open | no | yes | .chart and .mid parsers disagree on when a Star Power phrase ends | one phrase-end helper in src/parse/song.cpp that both parsers feed | Zero-length and past-the-end phrases on .chart (and .mid) files would be awarded or not, changing paths, scores and the SP gauge. | (high) Which edge does Clone Hero use to end a Star Power phrase: a chord exactly on the end tick, a zero-length phrase, a phrase past the last note? |
| 22 | drift | 1 | 7 | `free-squeeze-rule` | open | no | unsure | Free squeezes still show a timing figure, though docs promise nothing | src/core/model.h needs_timing/is_free predicate beside Path::difficulty | Free squeezes (negative or 0 ms) would show nothing on the path button, badge and report, as the docs say. | (high) Is a squeeze at exactly 0.0 ms free or does it need earning, and should a free squeeze show a timing figure at all? |
| 23 | drift | 1 | 7 | `hardest-timing-meaning` | open | no | unsure | Squeeze filter and Path limit also count required early fills; docs say squeezes only | Activation::difficulty in src/core/model.cpp; docs and help text restate it | Either the docs say 'squeeze or required early fill' everywhere, or squeeze<=N and Path limit stop dropping charts and paths because of an early fill. | (high) Should 'hardest timing' for the squeeze filter and the Path limit include required early fills, or squeezes only? |
| 24 | drift | 2 | 1 | `squeeze-rating-one-rule` | open | yes | no | Orange scale line and eff. figure use two different 'does the ratio move a figure' tests | core/squeeze_rating: rate_note and is_scaled | Any row whose multiplier isn't exactly 1 always shows its eff. figure; orange means a shown multiplier governs something. |  |
| 25 | drift | 2 | 1 | `squeeze-rating-one-rule` | open | yes | no | One SqOut chord is judged at two different SP ends | core/squeeze_rating rate_activation: SqOut rated only as its table row | The SqOut's scale-line orange stops depending on the earlier SP end. |  |
| 26 | drift | 2 | 1 | `squeeze-rating-one-rule` | open | yes | no | The view hides ratios within 0.005 of 1 even when the rating warns | core/squeeze_rating: is_scaled with 1e-9 noise tolerance; view only formats | The scale line prints extra decimals when two would read x1.00, so a figure never appears without its multiplier. |  |
| 27 | drift | 2 | 1 | `pre-sqin-sp-end-stored` | open | no | yes | The pre-SqIn SP end is rebuilt by stepping back two measures | engine: store the old SP end (or its ratio) at copy-out, like deact_tick (ADR 0011) | Song Details would print the SqIn scale clause and eff. figure from the true old SP end on charts with tempo or meter changes. |  |
| 28 | drift | 2 | 1 | `cap-clamp-anchor` | open | no | yes | Cap-clamped windows measure the ratio from the activation, not the collecting note | core/squeeze_rating frontend_transfer_scales reading the stored clamp_tick | On cap-clamped windows with a tempo change, the scale and eff. figures would be measured from the collecting note. | (high) When the SP cap clamps a window, should the squeeze scale be measured from the collecting note (as the video proof implies) rather than the activation? |
| 29 | drift | 2 | 1 | `backend-sqout-claim` | open | no | yes | Backend table rates a SqOut on a phrase note the engine never squeezes out | search/graph ScoreGraph::add_deact_edge: store which row it claimed; BackendSqueeze::summarystr reads it | The details table would stop offering a SqOut rating on a second phrase note no path can squeeze out. |  |
| 30 | drift | 2 | 7 | `squeeze-window-500ms` | open | no | unsure | The 500 ms squeeze window is tested in four places and typed as 500 twice | kSqueezeWindowMs plus one within_squeeze_window predicate in core/model.h | Tail rows the graph keeps would stop going missing from the Backends table; the limit box and help text would print the constant. | (low) Should tail rows (notes after the last activation, kept within 500 ms of the last note) be measured from the last note or from the final SP end? |
| 31 | drift | 2 | 7 | `backend-limit-normaliser` | open | no | no | Backend limit box and applied limit disagree on negative INI values | Settings in src/app/config.cpp: one normaliser on load and edit, bounded by kSqueezeWindowMs | A negative Backend limit in hydra_settings.ini would be clamped to 0 on load, so the box and the applied limit agree. |  |
| 32 | drift | 2 | 1 | `replay-sp-paid-chords` | changed | no | no | Replay re-decides which chords SP pays; doubles a squeezed-out 0-SP chord | core/backend_value.h: a named 'paid by SP' helper that replay_path reads | The Preview score box would stop showing x2 right after a squeezed-out one-note chord. | (high) Should the replay's own walk stay as an allowed second scorer, and should a squeezed-out chord that earns 0 SP show the doubled multiplier disc? |
| 33 | drift | 2 | 3 | `uncounted-tag` | open | partly | no | Uncounted Star Power phrase rows show 0 points but no '(uncounted)' tag | BackendSqueeze::summarystr in core/model.cpp (or the lead text in path_view) | Uncounted phrase rows would gain the tag (or the lead text would be reworded). | (low) Should uncounted Star Power phrase rows carry the '(uncounted)' tag like plain rows, or should the lead text stop implying every uncounted note is tagged? |
| 34 | drift | 2 | 3 | `rating-tiers` | open | no | no | Exactly 2.0 ms is Hard in the report but not warned in the GUI | core/model.h: one 'past the difficult floor' predicate read by is_difficult and timing_tiers | A path at exactly 2.0 ms would read the same in the GUI and the report. | (low) Is a squeeze of exactly 2.0 ms difficult (orange, Hard) or not? |
| 35 | drift | 2 | 3 | `report-tile-label` | open | no | no | Report tile 'Tightest squeeze' can show an early fill | src/app/report.cpp build_html | The tile would stop calling an early fill a squeeze. | (low) Should the 'Tightest squeeze' tile be renamed to match the Hardest ms column, or filtered to real squeezes? |
| 36 | drift | 2 | 3 | `ms-rounding` | open | no | no | Report page and app round exact half-way ms values differently | app/display_format: C++ side pre-formats the report payload | The report would show 12.2 where it shows 12.3 today for an exact half-way value, matching the app. |  |
| 37 | drift | 2 | 7 | `deact-edge-cap-ceiling` | open | no | yes | Deact edge rebuilds the SP extension without the cap, hiding squeeze-outs | ScoreGraph::extend_deacts in src/search/graph.cpp (edge and late-SqIn read its ext_map) | On very short-measure charts at a low cap, squeeze-out paths that silently vanish today would appear. | (high) When a collected phrase fills the meter past the cap, should the SP end be clamped to the cap ceiling for the squeeze-out edge too (as extend_deacts already does)? |
| 38 | drift | 2 | 7 | `replay-json-missing-fields` | open | no | no | Record reader and JSON reader treat a SqOut with no chord tick differently | core/replay windows_for_path; JSON reader shares the policy | none | (low) For an old JSON dump with a SqOut but no chord tick, should hydra_replay resolve the chord (as now) or refuse like the record reader? |
| 39 | drift | 2 | 4 | `user-guide` | open | no | no | UserGuide says 'full meter' follows the song; the box uses current tempo only | docs/UserGuide.md line 162 (or the box label); build_drain_box keeps the user-chosen rule | The guide (or box label) would say 'at the current tempo'. |  |
| 40 | drift | 2 | 4 | `user-guide` | open | no | no | User guide states the early-fill sign backwards | early_fill_difficulty / Activation::e_difficulty; guide describes it | The guide would say a positive early-fill number means hit early, and write 0.0ms like the screen. |  |
| 41 | drift | 2 | 4 | `cli-messages` | open | no | no | CLI messages and comments still say results don't carry their fill rule | store::Lens.legacy_fills; ADR 0010 and CONTEXT.md own the wording | hydra_batch and hydra_replay refusal messages would give the true reason (the guard is a kept choice). |  |
| 42 | drift | 2 | 4 | `user-guide` | open | no | no | User guide hard-codes 3 ms as the lower edge of the uncounted backend range | Rules::backend_leeway_ms (src/core/rules.h) | The guide would say 'from backend_leeway_ms (3 ms by default)'. |  |
| 43 | drift | 2 | 4 | `user-guide` | open | no | no | User guide says untagged MIDI charts still show dynamics counts; they show zeros | MidiParser::op_note (src/parse/song.cpp); fix the guide | The guide would say untagged MIDI charts show no ghost or accent counts. |  |
| 44 | drift | 2 | 4 | `adr-corrections` | open | no | no | ADR 0012 names a deleted function and a fixed kick pitch | MidiParser::optype and difficulty_base_pitch (src/parse/song.cpp) | none |  |
| 45 | drift | 2 | 4 | `adr-corrections` | open | no | no | ADR 0011 and a model.h comment describe the SP end without the cap clamp | ScoreGraph::extend_deacts (src/search/graph.cpp); ADR and comment defer to it | none |  |
| 46 | drift | 2 | 4 | `adr-corrections` | open | no | no | Cap-clamped squeeze doc points at three deleted functions | docs/cap-clamped-squeeze-frontend-anchor.md | none |  |
| 47 | drift | 2 | 4 | `adr-corrections` | open | no | no | model.h promises an old-record fallback the reader doesn't have | rebuild_record (src/store/path_codec.cpp); model.h comment and ADR 0014 describe it | none |  |
| 48 | drift | 2 | 4 | `adr-corrections` | open | no | no | ADR 0011 says backend rows exist only after the SP end | ScoreGraph::add_deact_edge; ADR 0011 needs a correcting note | none |  |
| 49 | drift | 2 | 7 | `multiplier-squeeze-rule` | open | no | no | Multiplier squeeze check uses its own combo list and misses 4- and 5-note chords | to_multiplier in src/core/timing.cpp; MultSqueeze::applies asks it | The Multiplier squeeze list on the Paths tab would gain 4- and 5-note chord entries. | (high) Should the Multiplier squeeze list include 4- and 5-note chords that straddle a multiplier step, or stay limited to the fixed combo list? |
| 50 | drift | 2 | 7 | `multiplier-squeeze-rule` | open | no | no | Shown multiplier-squeeze points are too low on 4-note chords | core/scoring category_scores per-note pricing (gain = best order minus worst order) | A 4-note chord at combo 7 would show (+30 pts) instead of (+15 pts), and the fold total rises to match. |  |
| 51 | drift | 2 | 3 | `percent-rounding` | open | no | no | Leaderboard page can show 100.00% for a score below optimal | collect_dm_rows in src/app/dm_report.cpp: round once | A score just short of optimal would read 99.99% instead of 100.00%. |  |
| 52 | drift | 2 | 1 | `offered-fills-stored` | changed | no | yes | Preview can light the wrong offered fill under the 1.0 fill rule | Engine::branch_activate: store the skipped fills' ticks on each Activation | Under CH 1.0 fills the dim 'offered' fill would sit on the fill the game actually showed. |  |
| 53 | drift | 2 | 2 | `fill-length-meter-walk` | open | no | yes | Generated fill length reads the meter one change late with two changes between chords | MeasureIndex::section_at in src/core/timing.cpp, called from Song::check_activations | Generated fills on such charts would change length, which can change the E label, legal activations and the best path. | (high) When two meter changes fall between two chords, which meter does Clone Hero use for the generated fill length: the current one or the one before the latest change? |
| 54 | drift | 2 | 7 | `fill-rule-stamp` | open | no | no | Four readers interpret the fill-rule stamp four ways; migration spells 'ch10' itself | store: one function mapping engine_mode stamp (and row count) to a fill rule, built on engine_mode_stamp | The CLI tools would give consistent fill-rule messages and stop leaving a 1.0 column silently empty. | (low) What should hydra_fillcompare and hydra_batch do with an unstamped file that has results, and should fillcompare warn when one hydra.db serves both rules? |
| 55 | drift | 2 | 3 | `fill-rule-names` | open | no | no | Each fill rule's name and explanation is typed separately in about eight places | one fill-rule label and description function in src/search/graph.h | The same rule would carry one name in the GUI confirm, hydra_batch, the reports and tooltips. | (low) What should each fill rule be called on screen, one name everywhere or a short and a long form? |
| 56 | drift | 2 | 7 | `depth-mode-normalise` | open | no | no | Out-of-range score-range setting read three ways; batch runs carry the result key twice | app::Settings in src/app/config.cpp: normalise depth_mode on load, one accessor and one label helper; run_batch derives its lens | A hand-edited depth_mode=2 would no longer file results under key 2 or show 'points' in the batch confirm while searching by scores. |  |
| 57 | drift | 2 | 3 | `ms-to-tick` | changed | no | no | Preview time box picks BPM by ms but meter, section and drain rate by the rounded tick | SongTiming / MsIndex::tps_at: one ms-to-display-tick function, BPM read on that tick | Within half a tick before a tempo change the BPM line would switch together with the meter and drain box. |  |
| 58 | drift | 2 | 7 | `measure-position-meter-tick` | open | no | no | Two measure-position functions read a meter-change tick differently | MeasureIndex::section_at; measures_at_tick_f calls it with the side explicit | The Preview SP gauge would show no kink at a mid-measure meter change inside an SP window. | (low) On the exact tick of a mid-measure meter change, does the measure position belong to the old meter or the new one? |
| 59 | drift | 2 | 7 | `ch-probe-tools` | open | no | no | play_chart.py keeps its own chart parser, MIDI reader and tempo math | hydra_replay JSON dump (tools/replay.cpp) read by tools/ch_probe/experiments/play_chart.py | none | (low) Should the auto-play tool read the notes from hydra_replay's JSON dump instead of parsing charts itself? |
| 60 | drift | 2 | 2 | `unknown-artist-owner` | open | no | no | Blank artist or charter shows blank for .ini/.sng but placeholder for .srb | artist_or_unknown / charter_or_unknown beside title_or_unknown in src/parse/song.h, applied once in discover_charts | Charts with a blank artist or charter would all show the same placeholder in the library and reports. | (low) When a chart's artist or charter is present but blank, should the library show '<unknown artist>' (as for .srb) or a blank? |
| 61 | drift | 2 | 2 | `container-entry-format` | open | no | no | A container's notes entry is recognised by name in .sng but by extension in .srb | src/parse/chart_files.cpp: one 'container entry name to chart format' rule | A .sng with a non-standard notes entry name would load instead of failing with 'No chart files found'. | (low) Should a .sng whose notes entry is not named notes.mid or notes.chart load, as an equivalent .srb does? |
| 62 | drift | 2 | 7 | `songmeta-length-per-chartmode` | open | no | unsure | songmeta keeps one song length per chart, overwritten by the last difficulty analyzed | RecordStore write policy; stored length keyed per chartmode or kept on the result row | An Expert record's timeline would stop ending at Easy's last note after Easy was analyzed last. | (low) Should the song length be kept once per song, or once per difficulty so each record reads its own last note? |
| 63 | drift | 2 | 7 | `duplicate-md5-name` | open | no | no | Duplicate chart copies are named by the first copy at scan, the last saved after analysis | RecordStore: one 'which copy names an md5' rule; run_batch dedupes by md5 | Reports would show a stable name for duplicate charts, and a batch would analyze each md5 once. | (low) When two folders hold the same chart file, which copy should name it: the first one scanned or the last one analyzed? |
| 64 | drift | 2 | 2 | `lower-diff-dynamics` | open | no | yes | A late dynamics tag leaves early notes plain while the tab says dynamics are on | MidiParser in src/parse/song.cpp: decide 'enabled' once per track | On MIDI charts with a late tag, early ghost and accent notes would be counted the same way as the flag shown on the Dynamics tab. | (high) If [ENABLE_CHART_DYNAMICS] appears after some notes, does Clone Hero price the earlier notes as ghost or accent, or only the notes after the tag? |
| 65 | drift | 2 | 7 | `rules-rows-purge` | open | no | unsure | Results made under other rules are deleted, though the docs promise they come back | RecordStore::write_row purge policy (src/store/record_store.cpp) | After editing hydra_rules.ini, re-analyzing a few songs and reverting, the old results would still be Ready (or the docs would say they are gone). | (high) Should results made under other rules stay in the database so switching the rules back brings them back, or should they be deleted and the docs rewritten? |
| 66 | drift | 2 | 7 | `ini-line-reader` | open | no | no | Two INI line readers treat comments and bad lines differently | core/strutil: one line splitter (trim, cut comment, split at '='); each file keeps its own error policy | A hand-edited 'view_difficulty=Hard # practice' would read as Hard instead of silently becoming Expert. | (low) Should hydra_settings.ini accept '#' comments after a value (like hydra_rules.ini does)? |
| 67 | drift | 2 | 3 | `report-search-rule` | open | no | no | Library search and report-page search match rows by different rules | fold_for_search in src/app/library_query.cpp (report payload carries pre-folded text and matches per word) | Typing 'beyonce' in a report page would find 'Beyonce' with the accent, as in the Library. | (low) Should the report pages' search follow the Library search rules (accent folding, word-by-word), or stay a simpler separate rule? |
| 68 | drift | 2 | 7 | `parse-bool` | open | no | no | hydra_replay accepts 'true' and 'yes' for flags; the settings file accepts only '1' | parse_bool in src/core/strutil | hydra_replay might stop accepting 'true'/'yes' for --prodrums and --bass2x. | (low) Should hydra_replay keep accepting 'true' and 'yes', or only '0' and '1' like its help text and the settings file? |
| 69 | drift | 2 | 3 | `progress-fraction` | changed | no | no | Progress bars disagree on how full a bar is when the total is 0 | src/ui/widgets.h: one done/total fraction helper | The batch strip and the Scanning modal would draw a 0-of-0 bar the same way. | (low) When a progress bar's total is 0, should it draw full (nothing to do) or empty (not reported yet)? |
| 70 | drift | 2 | 3 | `ellipsis-rule` | open | no | no | Two ellipsis rules cut text differently at the edge and around a trailing space | render::ellipsize in src/render/overlay_layout.cpp, extended to return the kept width | Cut labels would end the same way (with or without a space before the ellipsis) in the Library and the Preview path picker. |  |
| 71 | drift | 2 | 3 | `no-notes-message` | open | no | no | No-notes error is worded differently by the search and recognised by retyping its words | src/parse/song.cpp: no_notes_message plus its recogniser (or a typed error) and one load-and-check function | hydra_replay and bench would say 'No Expert Pro Drums notes in this chart.' instead of the generic 'No drum notes in this chart.'. |  |
| 72 | drift | 2 | 7 | `preview-volume-range` | open | no | no | Preview volume range 0-100 is enforced three ways; a typed slider value escapes it | app::Settings in src/app/config.h: one min/max pair or clamp function and one percent-to-gain helper | Typing 150 into the volume slider would clamp to 100 instead of saving 150 and falling back to 40% next launch. |  |
| 73 | drift | 2 | 7 | `preview-transport-play-state` | open | no | no | Jumping back after the audio ends leaves the highway moving in silence | PreviewTransport (src/ui/preview_transport.cpp): seek makes the playhead match the clock | On a chart that runs past its audio, jumping back while playing would bring the sound back. |  |
| 74 | drift | 2 | 7 | `audio-magic-bytes` | changed | no | no | Two magic-byte rules disagree on which bytes count as playable audio | audio::sniff_format in src/audio/decode.cpp; looks_like_audio calls it or goes | A .srb with an odd Ogg or RIFF stream would no longer lose its real audio. |  |
| 75 | drift | 2 | 6 | `wrap-words-widest` | open | no | no | widest_word re-walks the line breaks that wrap_words decides | render::wrap_words in src/render/overlay_layout.cpp; widest_word reads its widest line | none |  |
| 76 | drift | 2 | 6 | `track-state-spans` | changed | no | no | Draw code adds its own tie-break for which pad colours a taken fill | render::TrackState carries the pad on each span | none |  |
| 77 | drift | 2 | 7 | `ch-probe-tools` | open | no | no | Probe scripts disagree on the total hit-window cap: 171.43 ms vs 170 ms | tools/ch_probe/constants.py: one measured whole-window cap constant and tolerance | The probe tools would print the same clamp verdict for the same song; no Hydra screen changes. | (high) What is Clone Hero's real whole hit-window cap: 171.43 ms (as measured) or 2 x the back window (170 ms)? |
| 78 | drift | 2 | 7 | `ch-probe-tools` | open | no | no | Experiment scripts each rewrite the live-engine pick their own way | tools/ch_probe/engine_finder.py find_live_engine and MODULE_SPAN as one shared candidate filter | none | (low) Should the four older experiment scripts be repointed to find_live_engine, or deleted (the pending Task 21 question)? |
| 79 | drift | 2 | 7 | `ch-probe-tools` | open | no | no | Probe runners wait for song time with separate loops; play_chart presses up to 2 ms early | InputDriver.schedule_hit extended, or a helper beside walk_edges.SongClock | none | (low) Should the probe runners all fire at the target time exactly, or keep play_chart's 2 ms lead and the different stall limits (5 s vs 8 s)? |
| 80 | drift | 2 | 7 | `ch-probe-tools` | open | no | no | Starting mid-song, three runners skip different notes | one start-cursor helper shared by walk_edges, active_probe and play_chart | none | (low) When a probe starts mid-song, how far behind or ahead of the clock should the first note be? |
| 81 | drift | 2 | 7 | `ch-probe-tools` | open | no | no | Probe scripts report a hit's timing offset by two different formulas | walk_edges.Row.measured_ms or a shared function in experiments/live.py | none |  |
| 82 | drift | 2 | 7 | `ch-probe-tools` | open | no | no | Two functions find the hit/miss edge and disagree when results overlap | tools/ch_probe/experiments/analysis.py find_window_edge | none |  |
| 83 | drift | 2 | 7 | `ch-probe-tools` | open | no | no | test_attach attaches its own debugger and can kill Clone Hero on Ctrl+C | tools/ch_probe/debugger.py Debugger.attach | none |  |
| 84 | drift | 2 | 7 | `ch-probe-tools` | open | no | no | Five scripts use five tolerances to decide the window value changed | one 'window changed' helper with one stated tolerance (watch_window or analysis) | none | (low) What tolerance should count as the stored hit window having changed? |
| 85 | drift | 2 | 7 | `ch-probe-tools` | open | no | no | hit_detect presses chords its own way and plays 2x kick on O, not L | tools/ch_probe/input_driver.py press_chord, plus a 2x-kick entry in DEFAULT_BINDINGS if needed | none | (low) Which key should the probe press for a 2x kick: L (the normal kick, as play_chart does) or O? |
| 86 | drift | 2 | 7 | `every-path-sentinel` | open | no | no | Two constants decide when a report means every path | kEveryPathSentinel in src/app/report.h | hydra_report --paths 200000000 would read 'top 200000000 paths' instead of 'every path' (or the reverse). | (low) Should a very large --paths value (not just --all-paths) also be labelled 'every path' in the report subtitle? |
| 87 | drift | 2 | 3 | `status-words` | open | no | no | Test tool prints different status words than the app shows | one RecordStatus-to-label function in src/ui/library_model.cpp beside chip_of | The hydra_uitest state dump would say Analyzed, Stale, Not analyzed like the app (so docs/agents/ui-testing.md and any scripts matching 'current' or 'new' need updating). |  |
| 88 | drift | 2 | 7 | `ready-no-paths` | open | no | no | A Ready chart with no paths counts as analyzed in some places, not others | store RecordStatus (Ready) plus one explicit 'has a scored best path' flag decided once (for example in facts_of) | A zero-path chart would sit under one status in the library and the reports instead of two. | (low) Should a Ready record that holds zero paths count as analyzed (chip, stars: and squeeze<= filters, DM page, fill page)? |
| 89 | drift | 2 | 1 | `tied-variant-own-state` | open | no | yes | A tied path shows its leader's leftover SP, not its own | Engine (src/search/engine.cpp): emit each variant's own SP bank, or key finished paths by bank | A tied variant would show its own leftover SP (for example 3 bars, not 1) on the Paths tab. |  |
| 90 | drift | 2 | 1 | `tied-variant-own-state` | open | no | yes | A path folded mid-SP keeps a stale SP end, clamp note and phrases | Engine: the activation the engine actually closes (reduce_group / emit_variant / Path::prepare_variants) | Such variants would show the right SP end, gauge and score box instead of 'Score unavailable' and an early-stopping gauge. | (high) When a tied path is folded into its leader while Star Power is still running, should the engine stop folding there, or give the variant the leader's closed activation? |
| 91 | drift | 2 | 4 | `user-guide` | open | no | no | User Guide promises the overfill warning more often than the code shows it | rate_activation (cap_clamped); UserGuide.md and CONTEXT.md restate its conditions | The guide would say the overfill warning shows only when a window is clamped and has a squeeze or an uncounted row. |  |
| 92 | drift | 2 | 4 | `adr-corrections` | open | no | no | Docs say a clamped SP end can't move; the engine moves it | ScoreGraph::extend_deacts; ADR 0013 and CONTEXT.md reworded | none |  |
| 93 | drift | 2 | 4 | `user-guide` | open | no | no | Paths row's 'N bars' is the bank at activation, but docs call it bars spent | the engine record (sp_meter, collected_phrase_ticks, deact_tick); guide and sp_bars comment reworded | Either the guide says 'banked' or the row shows a larger number on activations that collect a phrase mid-SP. | (low) Should the Paths row's 'N bars' mean bars banked at activation (reword the docs) or bars actually spent (change the number)? |
| 94 | drift | 2 | 3 | `rating-tiers` | open | partly | no | The 2W squeeze budget is written three times with opposite edge rules | squeeze_budget_ms in src/core/squeeze_rating.cpp | At exactly 170.0 ms the report tier and the details view would agree. | (low) Is a gap of exactly 2W (170.0 ms at the default window) 'beyond' the budget or still inside it? |
| 95 | drift | 2 | 7 | `tie-limit-key` | open | no | yes | Tie limit counts per score and limit side, not per score as the guide says | Engine::reduce_group in src/search/engine.cpp; UserGuide line 249 and rules.h comment match the decision | The Paths tab Optimal group could list fewer tied paths than today, or the guide would state the real per-side limit. | (high) Should max_tied_paths limit all tied paths at one score together (drop the 'over the Path limit' bit from the tie key), or per side with the guide corrected? |
| 96 | drift | 2 | 4 | `user-guide` | open | no | no | Guide says fill slop decides whether a fill counts; code only picks its chord | fill_lands_on_chord in src/parse/song.cpp (the rule); UserGuide line 253 matches rules.h | The guide would say fill_land_slop_beats only chooses which chord an authored fill lands on. |  |
| 97 | drift | 2 | 1 | `tied-variant-own-state` | changed | no | yes | A tied variant shows its leader's skip count and E mark, not its own | Engine (src/search/engine.cpp) owns per-path skip state; Path::prepare_variants stops copying skips and e_offset | A tied variant's row and copied string would read its real notation (0 E2, not 0 1), and its early-fill line and badge would be right. |  |
| 98 | drift | 2 | 7 | `no-path-best-score` | open | no | no | No-path best score is 0 in hydra_replay and hydra_bench but blank in hydra_batch | src/store/record_store.cpp summarize_record (optional score) or one HydraRecord best-score helper | hydra_replay JSON and hydra_bench would stop printing a real-looking 0 for an empty record. | (low) When a record holds no paths, should hydra_replay and hydra_bench print no score (like hydra_batch's '-') instead of 0? |
| 99 | drift | 2 | 3 | `number-grouping` | open | no | no | Report pages group thousands by browser locale; the app always uses commas | src/core/model.cpp group_thousands (page data carries pre-formatted strings, or one fixed-comma JS formatter) | In a non-English browser the report numbers would read 1,234 instead of 1.234 or 1 234. | (low) Should report pages always use commas for thousands (as the app does), or follow the browser's locale? |
| 100 | drift | 2 | 2 | `time-signature-zero` | open | no | no | A zero time signature is skipped in .chart but crashes Hydra in .mid | apply_timesig in src/parse/song.cpp | A .mid with a zero numerator would no longer kill Hydra on Analyze, batch, Preview or Dynamics. | (low) For a time signature whose top number is 0, should every parser skip it (as .chart does now), or reject the chart with a clear error? |
| 101 | drift | 2 | 7 | `song-mix-stem-rule` | open | no | no | A .sng's preview clip is mixed into the song; a loose folder's is left out | one song-mix stem predicate in src/app/preview_source.cpp | A .sng holding a preview clip would no longer play it over the start of the song. | (low) Should a preview.* clip inside a .sng be left out of the Preview's song mix, as it is for loose folders? |
| 102 | drift | 2 | 4 | `adr-corrections` | open | no | no | ADR 0012 says a ghost kick draws narrowed; the Preview draws it full width | build_highway_draws in src/render/highway_draw.cpp; ADR 0012 points to it or records f3ff7ec | none |  |
| 103 | drift | 2 | 7 | `replay-legacy-fills-key` | open | no | no | hydra_replay dump can never read a stored 1.0-fills result | Settings::legacy_fills (src/app/config.cpp), set once in settings_from in tools/replay.cpp | hydra_replay dump --legacy-fills would read the stored 1.0 record like the GUI does, and drop its two hand overrides. |  |
| 104 | drift | 2 | 4 | `user-guide` | open | no | no | Path report lists every chart mode, but the docs say current mode only | collect_rows in src/app/report.cpp (user decision 2, 2026-09-26 plan); UserGuide, development.md and the subtitle follow it | The guide, development.md and the report subtitle would say: every mode at the current cap and lens, top N per chart and mode. |  |
| 105 | drift | 2 | 3 | `empty-report-reason` | open | no | no | Empty report says nothing is stored when records exist under other settings | generate_report / generate_fill_report in src/app, returning why there are zero rows | An empty report would say why (nothing stored, or nothing Ready under these settings) instead of telling the user to run hydra_batch again. |  |
| 106 | drift | 2 | 4 | `cli-messages` | open | no | no | Batch and report both say 'records across' but count different things | src/cli/batch.cpp closing line (name its scope) or RecordStore sharing generate_report's Ready scope | The batch line and the report subtitle would give the same record count, or the batch line would name its scope. | (low) Should hydra_batch's closing line count every stored row, or only the Ready records at the current settings like the report? |
| 107 | drift | 2 | 4 | `cli-messages` | open | no | no | hydra_replay dump's 'not analyzed' line echoes typed settings, not the ones it used | tools/replay.cpp cmd_dump: describe the parsed Settings with the app's settings wording | The stderr line would show the cap, ms and score range the lookup really used. |  |
| 108 | drift | 2 | 7 | `bench-dump-paths` | open | no | no | bench --dump-db writes absolute backslash paths; --scan --dump writes relative ones | tools/bench.cpp: one row-to-JSON function used by scan_mode and dump_db | hydra_bench dump files from a scan and from the database could be diffed without every row differing. |  |
| 109 | drift | 2 | 7 | `uitest-idle-jobs` | open | no | no | hydra_uitest wait-idle keeps its own job list and misses three background jobs | src/ui/app_state.cpp: one any_job_running() query next to batch_running/analyze_running | hydra_uitest wait-idle would wait for the Dynamics load, song-length backfill and Preview path-switch jobs. |  |
| 110 | drift | 2 | 4 | `cli-messages` | open | no | no | hydra_replay's squeeze-out warning assumes first_note even when whole_chord is chosen | src/core/scoring.cpp category_scores / CategoryScores::sqout_reduction; the warning takes the rules | With sqout_rule=whole_chord set, hydra_replay's warning would name the real overstatement for a multi-note chord. |  |
| 111 | drift | 2 | 3 | `rich-text-strip` | open | no | no | Chart titles are cleaned up for display in several places, each a different way | one display-title helper in src/parse/song.h next to title_or_unknown: strip, trim, fall back | A markup-only title would read '(unknown)' in the library, and tooltips, status lines and the batch dialog would stop showing raw colour tags. |  |
| 112 | drift | 3 | 6 | `widest-digits` | open | no | no | Score range box assumes 0 is the widest digit | widest_digits in src/ui/widgets.h; render_score_range calls widest_digits(6) | The Score range box would be about half a pixel wider. |  |
| 113 | drift | 3 | 6 | `dead-sql-search` | open | no | no | A second, unused library search in SQL matches different charts | src/app/library_query.cpp query_matches / term_matches; delete the SQL search parameter and unused BatchJob constructor | none |  |
| 114 | drift | 4 | 4 | `stale-comments` | open | no | no | The code reuses the glossary term 'frontend squeeze' for a different idea | CONTEXT.md glossary; rename has_frontend_squeeze in rate_activation | none |  |
| 115 | drift | 4 | 4 | `stale-comments` | changed | no | no | Preview comment says the ms index stops short; it covers every tick | MsIndex::at (src/core/timing.cpp); fix the comment in preview_view.cpp | none |  |
| 116 | drift | 4 | 7 | `record-decode-helper` | open | no | no | The same stored result decodes with two different fill-rule flags | RecordStore: one decode helper that sets legacy_fills from the row's lens | none |  |
| 117 | drift | 4 | 6 | `test-fixture-sng-srb` | open | no | no | .sng/.srb test fixtures are hand-encoded in many tests and disagree on a length field | one shared test fixture header beside tests/midi_util.h, built on src/parse/sng.h and srb.h constants | none |  |
| 118 | drift | 4 | 4 | `stale-comments` | open | no | no | One comment says the gem light sits at the bottom centre; code uses the top | light_for in src/render/highway_draw.cpp; fix the comment in src/render/preview_config.h | none |  |
| 119 | drift | 4 | 6 | `tests-recompute-production` | open | no | no | Test helpers match chart extensions case-sensitively while chart_format_of ignores case | chart_format_of in src/parse/chart_files.cpp; tests call it | none |  |
| 120 | drift | 4 | 6 | `tests-recompute-production` | open | no | no | Panel-split test re-derives the library width and drops one limit each time | a named minimum library width beside kMinSongPanelW, ideally one split-width function in library_view | none |  |
| 121 | drift | 4 | 6 | `shared-test-equality-helpers` | open | no | no | Codec test's PathSummary equality skips stars; store test's checks it | src/store/record_store.h: operator== beside PathSummary | none |  |
| 122 | drift | 4 | 6 | `shared-test-equality-helpers` | open | no | no | Two container-read tests define 'same Song' with different fields | one shared test helper (tests/song_equal.h) | none |  |
| 123 | double_reading | 1 | 3 | `song-panel-stale-result` | open | no | no | Song panel keeps an old result after a batch updates the same chart | src/ui/app_state.cpp: one 'store changed for chart X' hook | After a batch, the song panel shows the finished result instead of 'Not analyzed yet' or 'Out of date'. |  |
| 124 | double_reading | 2 | 7 | `batch-settings-snapshot` | open | no | no | Post-batch report reads live settings, not the batch's own snapshot | BatchJob: keep and expose its BatchRun for the report | none in normal use; closes a one-frame race that could list the wrong cap in the report. |  |
| 125 | double_reading | 2 | 7 | `batch-skip-count-one-source` | open | no | no | Batch confirm counts cached library chips; the batch re-asks the store | the store's Ready rule (analyzed_hashes), queried once | The confirm dialog's count always matches what the batch then skips, even if another tool wrote the database. |  |
| 126 | double_reading | 2 | 7 | `preview-chart-md5-check` | changed | no | no | Preview re-parses the chart file without checking it matches the viewed record | src/ui/preview_load_job.cpp / src/app/preview_source.cpp: compare md5 with viewed.hyhash | A chart edited after scanning no longer shows activations on the wrong notes in the Preview. | (low) If the chart file was edited after the scan, should the Preview refuse to open it, show a warning, or just re-read it as it does now? |
| 127 | double_reading | 3 | 1 | `sqout-offset-stored-once` | open | partly | unsure | The SqOut's offset is stored twice and the display reads both copies | engine copy-out / Activation: one stored SqOut offset | none |  |
| 128 | double_reading | 3 | 1 | `sqout-offset-stored-once` | open | no | unsure | Whether an activation squeezed out is stored twice and readers mix both | stored sqout_tick (ADR 0014), read through one accessor | none |  |
| 129 | double_reading | 3 | 7 | `stars-cache-guard` | open | no | no | Headline shows score from the record but stars from the stored cache | core/stars path_stars; results.stars stays a stamp-guarded cache | none | (low) Should the headline keep reading the stored star count (guarded by the results stamp), or work stars out live from the record like the Stars tab? |
| 130 | double_reading | 3 | 7 | `results-key-columns-one-source` | open | no | no | SP cap and ms limit stored in both key columns and the blob | results key columns written by RecordStore::write_row | none |  |
| 131 | double_reading | 3 | 6 | `library-count-one-source` | open | no | no | Library chart count cached in AppState and read from the model | LibraryModel::rows().size() | none |  |
| 132 | double_reading | 3 | 7 | `store-columns-vs-blob` | open | no | no | Best path score, notation and counts read from stored columns and from the blob | structure blob (Path::pathstring, totalscore, summarize_path); columns are a cache reindex always rewrites | none; reindex would also fix the Best path text column. |  |
| 133 | double_reading | 3 | 7 | `store-columns-vs-blob` | open | no | no | Hardest timing read from a stored column by the library but recomputed elsewhere | undecided: stored hardest_ms column, or summarize_path with a stamp-bump rule | none today; the library squeeze<=N filter and the report would never disagree after a rule change. | (low) Should the hardest squeeze time come from the stored column everywhere, or be worked out fresh from the path everywhere? |
| 134 | double_reading | 3 | 7 | `chartmode-key-from-parsed-difficulty` | open | no | no | Record key pastes the raw difficulty word; two writers clean it by hand | Settings::chartmode_key building from difficulty_name(difficulty()) | none |  |
| 135 | double_reading | 4 | 6 | `preview-scene-timing-one-copy` | changed | no | no | Preview scene stores tick resolution twice; production reads only one | SongTiming::tick_resolution() through PreviewScene::timing | none |  |
| 136 | duplicate | 1 | 3 | `sp-colours` | open | no | no | Preview shows activations in two different golds | src/ui/theme.h: a named SP gold beside kBestPathColor | The Preview scrubber marks and the next-activation header show the same gold. | (low) Should activation marks and the next-activation header use the best-path gold or the Star Power gold? |
| 137 | duplicate | 2 | 1 | `squeeze-rating-one-rule` | open | yes | no | Plain-row scale branch writes its own sign test instead of the owner's | core/squeeze_rating rate_note, branching on counted_without_squeeze / paid_by_sp_walk | Counted rows at the SP end or inside the leeway get the early multiplier and show their effective figure. |  |
| 138 | duplicate | 2 | 7 | `squeeze-window-500ms` | open | no | no | Backend and Path limit boxes type 500 instead of the engine window | kSqueezeWindowMs (src/core/model.h) | An out-of-range INI limit would load clamped, matching what typing the same number in the box does. | (low) Should an INI value outside the box's range (such as a Backend limit of -50 or a Path limit of 900) be clamped on load like the box does? |
| 139 | duplicate | 2 | 7 | `sp-cap-minimum` | open | no | no | Smallest SP cap of 1 checked in five places; replay's message types Clone Hero's 4 | a kMinSpCap constant beside kCloneHeroSpCap in src/core/model.h | none unless cap 1 is refused; replay's error text would print the constant. | (low) Should an SP cap of 1 be allowed at all, given it can never produce an activation? |
| 140 | duplicate | 2 | 7 | `hit-window-type` | open | no | no | Hit window default cast to int in three places; INI loader drops fractions | Settings::hit_window_ms (double seeded from kDefaultHitWindowMs); one constant in tools/ch_probe/constants.py | none at 85; an INI value like 85.5 would stop being rounded down to 85. | (low) Should the hit window setting accept fractions such as 85.5, or stay a whole number? |
| 141 | duplicate | 2 | 2 | `chart-event-normaliser` | open | no | unsure | .chart solo and disco events match raw text; section names strip quotes | ChartDataEntry (src/parse/song.cpp): one payload normaliser for every E-event match | A chart that quotes its solo or disco events would keep them in Hydra if Clone Hero does. | (high) How does Clone Hero read a quoted event such as E "solo": as a solo, or as a plain event? |
| 142 | duplicate | 2 | 7 | `batch-counts` | open | no | no | Batch analyzed and skipped counts rebuilt by each caller | run_batch in src/app/analysis.cpp: report analyzed and skipped in BatchProgress | The 'N analyzed' count no longer reads one too low for a frame after a chart fails. |  |
| 143 | duplicate | 2 | 7 | `library-row-identity` | open | no | no | Which library row is the selected one is decided in three places | one AppState helper for row identity by notespath | none for the helper; the two-folder behaviour only changes if the user picks differently. | (low) When the same chart sits in two folders and one copy is analyzing, should the other copy's panel show that progress or say 'Wait for the other to finish'? |
| 144 | duplicate | 3 | 1 | `squeeze-rating-one-rule` | open | partly | no | Whether a SqIn/SqOut is still to earn or already free is decided twice | core/model.h SPSqueeze: one 'still to be earned' predicate | none |  |
| 145 | duplicate | 3 | 1 | `replay-sqout-chord` | open | no | no | hydra_replay re-derives which phrase chord the engine squeezes out | search/graph: one exposed predicate or result next to kSqueezeWindowMs | none |  |
| 146 | duplicate | 3 | 6 | `sqout-position-one-test` | open | no | no | The 'before, on or past the squeezed-out chord' tick test is retyped outside sqout_pos... | core::sqout_position in src/core/backend_value.h; one new function for the backend offset | none |  |
| 147 | duplicate | 3 | 1 | `gauge-phrase-collection` | changed | no | unsure | Preview SP gauge counts banked bars itself, squeezed-out phrases by set difference | engine: store the bank after each window, or the gauge reads sqout_tick | none today; the gauge could no longer jump higher than the engine's bank. |  |
| 148 | duplicate | 3 | 6 | `backend-offset` | open | no | no | The graph writes its squeeze-note claim test twice | src/search/graph.cpp: one claim_squeeze_note helper | none |  |
| 149 | duplicate | 3 | 6 | `backend-offset` | changed | no | no | A row's distance from the SP end is computed in five places | backend_offset_ms(row_tc, end_tc) in core/backend_value.h | none |  |
| 150 | duplicate | 3 | 6 | `sqout-cost-one-helper` | open | no | no | Squeeze-out cost worked out twice on the Paths tab by undoing core's subtraction | core/backend_value.h: one squeeze-out cost helper plus sqout_position | none |  |
| 151 | duplicate | 3 | 6 | `path-difficulty-one-owner` | open | no | no | A path's hardest timing is computed three times | Path::difficulty in src/core/model.cpp (summarize_path calls it; engine shares the per-activation helper) | none |  |
| 152 | duplicate | 3 | 6 | `path-difficulty-one-owner` | open | no | no | Activation hardest timing written in both the model and the search engine | free function in core/model.h over (squeezes, e_offset, skips) | none |  |
| 153 | duplicate | 3 | 6 | `path-difficulty-one-owner` | open | no | no | Activation::is_difficult re-words the path's 'hardest above 2 ms' rule | Activation::is_difficult as difficulty() > kDifficultMs | none |  |
| 154 | duplicate | 3 | 3 | `ms-rounding` | open | no | no | Activation badge re-works out which part is hardest by matching numbers | core/model Activation: return the hardest part's kind with its value; a shared ms display helper | The badge and the notation text round the same ms the same way (162.7 would read the same in both). | (low) When a squeeze and an early fill tie for hardest, which should the badge name, and should its ms be rounded or truncated? |
| 155 | duplicate | 3 | 6 | `beyond-edge-one-owner` | open | no | no | Report page recomputes where Beyond starts and who is past it | beyond_edge_ms (edge) and tier_for (membership); the payload carries the edge | none |  |
| 156 | duplicate | 3 | 1 | `pre-sqin-sp-end-stored` | open | no | unsure | The unextended SP end is recomputed by hydra_replay and by test fixtures | ScoreGraph::add_act_edge, or a shared initial-end helper; fixtures take literal ticks | none |  |
| 157 | duplicate | 3 | 6 | `preview-sp-running-predicate` | changed | no | no | Drain box and highway floor each decide whether SP is running | one PreviewScene / PreviewActivation helper, e.g. sp_running_at(ms) | none |  |
| 158 | duplicate | 3 | 6 | `sp-cap-minimum` | open | no | no | Preview SP gauge applies the at-least-one-bar cap floor twice | build_sp_meter_curve in src/app/preview_view.cpp | none |  |
| 159 | duplicate | 3 | 6 | `gauge-phrase-collection` | changed | no | no | Gauge decides 'before or inside the window' in ms here, ticks there | build_sp_meter_curve, comparing ticks throughout | none |  |
| 160 | duplicate | 3 | 6 | `note-value-one-formula` | open | no | no | One note's 1x value is built by two different formulas | core/model ChordNote::basescore plus one per-note dynamic-share helper | none |  |
| 161 | duplicate | 3 | 6 | `note-value-one-formula` | open | no | no | A note's Star Power value is computed twice inside category_scores | category_scores in src/core/scoring.cpp | none |  |
| 162 | duplicate | 3 | 6 | `sp-multiplier-one-owner` | open | no | no | Star Power doubling lives in the scorer and again as a display constant | core/scoring: category_scores reads kStarPowerMultiplier | none |  |
| 163 | duplicate | 3 | 6 | `solo-bonus-helper` | open | no | no | Per-chord solo bonus formula written in engine and replay | core: solo_bonus(chord, flag_solo) beside kSoloBonusPerNote | none |  |
| 164 | duplicate | 3 | 6 | `combo-walk-one-owner` | changed | no | no | Running combo walked separately by the graph, the replay, the scorer and a test | category_scores returns the combo after the chord; graph and replay read it | none | (low) Are hand-written test checks (oracles) exempt from the derive-once rule, or must the combo test call production code? |
| 165 | duplicate | 3 | 6 | `total-score-one-sum` | open | no | no | A path's total score is summed in three places | core/model Path::totalscore | none |  |
| 166 | duplicate | 3 | 6 | `stars-with-solo` | open | no | no | Stars tab and its test add the solo bonus to each cutoff; core/stars only subtracts it | core/stars: StarCutoffs carries a with-solo array; score_without_solo helper beside path_stars | none |  |
| 167 | duplicate | 3 | 6 | `solo-sections-data` | open | no | no | Solo section boundaries decided in replay and in Preview scene | parse/song: solo sections stored as data both read | none |  |
| 168 | duplicate | 3 | 6 | `report-status-one-owner` | open | no | no | Fill and DM report pages decide 'which side is higher' twice | collect_fill_rows / collect_dm_rows status; page script keys on it | none |  |
| 169 | duplicate | 3 | 6 | `report-path-order` | open | no | no | Report sorts paths by score again after the engine already did | engine / HydraRecord::all_paths guarantees the order | none |  |
| 170 | duplicate | 3 | 7 | `dm-report-rules` | open | no | no | Leaderboard page prints 'SP cap 4' as text; its Clone Hero rules gate is split | collect_dm_rows in src/app/dm_report.cpp prints kCloneHeroSpCap and owns all three Clone Hero rules; the toolbar button asks it | none through the GUI; a future CLI caller could not compare at 1.0 fills by mistake. |  |
| 171 | duplicate | 3 | 6 | `stars-range-text` | open | no | no | Stars filter error hard-codes '0 to 7' beside kMaxStars | kMaxStars in src/core/stars.h, formatted into the message | none |  |
| 172 | duplicate | 3 | 6 | `paths-count-one-method` | open | no | no | 'Paths kept' and the path-count sort column count paths two ways | one HydraRecord method for the path count | none |  |
| 173 | duplicate | 3 | 6 | `within-n-band-test` | open | no | no | Search keeps a shortcut copy of the 'within N scores' rule | Engine::reduce_group in src/search/engine.cpp: one band predicate | none |  |
| 174 | duplicate | 3 | 7 | `ch-probe-tools` | open | no | no | Probe scripts and tests hard-code engine field offsets instead of reading constants.py | tools/ch_probe/constants.py (OFF_* constants, PRECISION_MODE_BIT) read through engine.EngineModel | none; developer console tools only. | (low) Should the three old probe scripts (find_engine, find_clock3, poll_windows) be deleted or kept? Task 21 step 10 asked this and it looks unanswered. |
| 175 | duplicate | 3 | 6 | `preview-taken-fill-index` | changed | no | no | Preview finds the taken fill twice, in the scene and again for lane colour | build_preview_scene: store the taken fill index on PreviewActivation | none |  |
| 176 | duplicate | 3 | 6 | `early-fill-window` | open | no | no | Early-fill window test written twice, in is_e0 and is_e_critical | one window test in src/core/model.h | none |  |
| 177 | duplicate | 3 | 6 | `fill-rule-names` | open | no | no | Legacy flag to fill rule mapping written twice in pather.cpp | one mapping function in src/search/pather.cpp | none |  |
| 178 | duplicate | 3 | 6 | `fill-attach-one-check` | open | no | no | Fill-attach check copied between the MIDI and .chart parsers | shared helper beside fill_lands_on_chord in src/parse/song.cpp | none |  |
| 179 | duplicate | 3 | 6 | `tied-count-one-owner` | open | no | no | Tied path count kept by the engine and recounted from the tree | Path::recount_tied_paths in src/core/model.cpp | none |  |
| 180 | duplicate | 3 | 6 | `allzero-one-definition` | open | no | no | The all-0 path's 0 ms limit is written twice, plus a test copy | named constant or predicate in src/search/pather.cpp; one path-difficulty rule | none |  |
| 181 | duplicate | 3 | 6 | `default-depth-constant` | open | no | no | The default search depth of 4 is written in three places | one constant in src/search/pather.h used by app::Settings and tools/replay.cpp | none |  |
| 182 | duplicate | 3 | 6 | `audio-ms-frames` | changed | no | no | Audio ms-to-frames conversion written out by hand four times | frames_of_ms / ms_of_frames helper pair in src/audio | none |  |
| 183 | duplicate | 3 | 6 | `ms-to-tick` | open | no | no | Rounding milliseconds to a chart tick written twice in the app | a rounded ms-to-tick beside MsIndex::tick_at_ms, plus format_measure for the label | none |  |
| 184 | duplicate | 3 | 6 | `ms-to-tick` | changed | no | no | Paths timeline left label is a hard-coded 'm1' | build_activations in src/app/path_view.cpp: timeline_start and timeline_end from format_measure | none |  |
| 185 | duplicate | 3 | 6 | `preview-playhead-clamp` | open | no | no | Drain box restates half of the Preview playhead clamp | shown_ms in src/app/preview_view.cpp, called by every box | none |  |
| 186 | duplicate | 3 | 6 | `notes-file-pick` | open | no | no | 'notes.mid beats notes.chart' is written separately for folders and .sng | one pick-notes-file helper in src/parse/chart_files.cpp | none |  |
| 187 | duplicate | 3 | 6 | `chart-format-one-owner` | changed | no | no | Preview checks .sng/.srb by suffix instead of asking chart_format_of | chart_format_of in src/parse/chart_files.cpp | none |  |
| 188 | duplicate | 3 | 6 | `srb-layout-one-walk` | open | no | no | The .srb stream layout is walked separately by three readers | src/parse/srb.h: one function returning the metadata and stream offsets | none |  |
| 189 | duplicate | 3 | 6 | `song-ini-pick` | changed | no | no | Scan and Preview each walk the folder to find song.ini | is_song_ini plus one folder-entry picker (or the Preview reuses the scan's ini path) | none |  |
| 190 | duplicate | 3 | 6 | `lane-flag-predicate` | changed | no | no | The kick-means-2x, pad-means-cymbal lane rule is respelled with no owner | one exported lane-flag predicate in src/core/model.h beside allows_cymbals | none |  |
| 191 | duplicate | 3 | 6 | `song-length` | changed | no | no | The Preview works out song length itself instead of asking the store | store::song_length_ms, called by build_preview_scene | none |  |
| 192 | duplicate | 3 | 6 | `chart-hash-normalise` | open | no | no | Chart hash lowercasing written in four places | normalize_chart_hash beside stream_md5 in src/app/analysis.cpp, used at the DMBot boundary | none |  |
| 193 | duplicate | 3 | 7 | `typed-error-kinds` | changed | no | no | Error messages become plain text by matching each thrower's exact wording | typed error kinds thrown at the source, mapped to sentences once in app::plain_error | A missing or damaged Preview asset would show a plain sentence instead of raw exception text. | (low) Should failures carry typed error kinds instead of matching message wording, and should a failed Preview show the plain sentence instead of the raw error text? |
| 194 | duplicate | 3 | 6 | `store-blob-header` | open | no | no | The structure blob's 12-byte header layout is respelled by hand in the record store | store/path_codec: export the header offsets and a read-header helper | none |  |
| 195 | duplicate | 3 | 6 | `store-blob-header` | open | no | no | Three separate little-endian byte codecs serve the stored blobs | store/serialize (BinaryWriter/BinaryReader or one exported helper) | none |  |
| 196 | duplicate | 3 | 6 | `stale-reason-one-evaluation` | open | no | no | Why a row is Stale is worked out separately from whether it is Stale | rank_row in src/store/record_store.cpp returns the reasons with the verdict | none; only hydra_replay dump prints the reasons. |  |
| 197 | duplicate | 3 | 6 | `record-key-identity` | open | no | no | reload_row checks row identity without the lens | RecordKey and the lens columns: reload_row compares the full key | none |  |
| 198 | duplicate | 3 | 7 | `replay-defaults-from-settings` | open | no | no | hydra_replay's Args struct holds its own copy of every GUI default setting | app::Settings in src/app/config.h; Args holds optionals and settings_from changes only passed flags | hydra_replay without flags would follow any future change to the app's defaults. |  |
| 199 | duplicate | 3 | 6 | `rules-field-table` | open | no | no | Rules field names typed twice: fingerprint and INI reader | core/rules: one (name, member) table walked by the fingerprint and the reader | none |  |
| 200 | duplicate | 3 | 6 | `settings-key-table` | open | no | no | Settings INI key names typed twice, in load and save | app::Settings: one key table used by load and save | none |  |
| 201 | duplicate | 3 | 6 | `ascii-string-helpers` | open | no | no | ASCII case and prefix helpers copied outside core/strutil after it became their owner | core/strutil (src/core/strutil.cpp), exporting character tests, starts_with, case-blind equals and starts_with_ci | none |  |
| 202 | duplicate | 3 | 7 | `report-file-name` | open | no | no | The report file name is typed twice, once for the GUI and once for the CLI | a file-name constant in src/app/report_files.h | The CLI report would land in the app's reports folder if the user picks that. | (low) When hydra_report runs with no --out, should it write into the current folder as now, or into the same Documents\Hydra folder the app uses? |
| 203 | duplicate | 3 | 6 | `squeeze-kind-name` | open | no | no | Squeeze kind is written by type_name but read back with a typed 'SqOut' | SPSqueeze::type_name in src/core/model.h plus a matching parse function | none |  |
| 204 | duplicate | 3 | 6 | `search-term-applies` | open | no | no | Library search decides twice whether a search term applies to a column | one term_applies_to helper in src/app/library_query.cpp | none |  |
| 205 | duplicate | 3 | 6 | `dynamics-count-one-accessor` | open | no | no | Ghost plus accent count is summed in two places | DynamicsCounts in src/app/dynamics_breakdown.h: a dynamic() accessor | none |  |
| 206 | duplicate | 3 | 6 | `settings-description-helper` | open | no | no | Three settings-to-text formatters describe the same analysis settings | one settings-description helper beside app::Settings, reusing within_label | none; only developer-tool headers read differently. |  |
| 207 | duplicate | 3 | 6 | `file-check-interval` | open | no | no | Two 2-second file re-check constants and their caching code typed twice | src/ui/app_state.h: one cached file-exists helper or one shared constant | none | (low) Should the selected-chart file check and the report file check share one refresh interval, or stay separate? |
| 208 | duplicate | 3 | 6 | `ui-font-size-constant` | open | no | no | UI font size 18 is typed three times in setup_imgui | one kFontSize constant in src/ui/app_shell.cpp | none |  |
| 209 | duplicate | 3 | 6 | `resource-folder-helper` | open | no | no | Resource folder path is built twice; only the font loader takes an override | one resource-folder helper next to app::exe_dir() | none |  |
| 210 | duplicate | 3 | 6 | `dpi-scale-one-owner` | open | no | no | DPI-to-scale is computed by Hydra and by the vendored ImGui backend | ui_scale_for_dpi in src/ui/app_shell.cpp | none |  |
| 211 | duplicate | 3 | 6 | `library-share-valid` | open | no | no | Library split share range is checked twice in app_shell.cpp | one share_is_valid helper in src/ui/app_shell.cpp | none |  |
| 212 | duplicate | 3 | 6 | `job-fail-helper` | open | no | no | AnalyzeJob::start hand-copies the job base class's failure handling | ResultJobBase in src/ui/job_base.h: a protected fail helper | none |  |
| 213 | duplicate | 3 | 6 | `button-width-helper` | open | no | no | Button width formula re-typed in three places beside its helper | button_slot_width in src/ui/widgets.h | none |  |
| 214 | duplicate | 3 | 6 | `fits-on-line-helper` | open | no | no | Does-the-next-item-fit test written five times | one fits-on-line helper in src/ui/widgets.h | none |  |
| 215 | duplicate | 3 | 6 | `preview-jump-constants` | open | no | no | Preview 5-second jump typed as bare 5000 four times | a kJumpMs constant beside kTickStep in src/ui/preview_tab.cpp | none |  |
| 216 | duplicate | 3 | 6 | `preview-sp-curve-predicate` | changed | no | no | Has-an-SP-meter-curve test asked twice, then ANDed | one predicate on PreviewScene in src/app/preview_view.h | none |  |
| 217 | duplicate | 3 | 6 | `shell-execute-ok` | open | no | no | ShellExecute success test typed in two files | one shell_execute_ok helper next to core/winstr | none |  |
| 218 | duplicate | 3 | 3 | `sp-colours` | open | no | no | Dimmed grey and hover teal kept as parallel values | src/ui/theme.h: alias kDisabledInputTextColor to kDimTextColor and add one hover-teal constant | none for the grey; frame and header hover could shift by 4 units of green and blue if matched to the button teal. | (low) Should the frame and header hover teal (0,100,100) match the button hover teal (0,104,104), or stay a separate choice? |
| 219 | duplicate | 3 | 6 | `status-fade-constant` | open | no | no | Status-line fade is a bare 6.0 seconds that the GUI test mirrors as 400 frames | a named constant (kStatusFadeSeconds) beside render_status_line in src/ui/library_toolbar.cpp; the test derives its wait from it | none unless a different time is chosen. | (low) Is 6 seconds the fade time you want for a news line in the status bar? |
| 220 | duplicate | 3 | 6 | `preview-config-one-source` | open | no | no | Preview's Onyx numbers kept in both 3d-config.json and PreviewConfig defaults | assets/preview/3d-config.json (ADR 0008); PreviewConfig does not restate the values | none |  |
| 221 | duplicate | 3 | 6 | `track-rect-helper` | open | no | no | Track rectangle size floor, aspect and bottom edge worked out in several places | track_rect(cfg, w, h) beside render::track_height in src/render/highway_draw | none |  |
| 222 | duplicate | 3 | 7 | `dm-base-speed` | open | no | no | '100 means base speed' typed four times, and the status line ignores it | one base-speed constant or predicate in src/net/dmbot_client.h | Off-speed runs might stop counting as above optimal on the dmleaderboards page. | (high) Does a playback speed other than 100% change a Clone Hero score, and should an off-speed run count as 'above optimal' on the leaderboard page? |
| 223 | duplicate | 3 | 6 | `toggle-on-helper` | open | no | no | Toggle 'on after this instant' rule written twice in the renderer | one helper next to the Toggle enum in src/render/track_state.h | none |  |
| 224 | duplicate | 3 | 6 | `highway-far-time` | open | no | no | Highway far-end time typed three times in highway_draw.cpp | a far_time(cfg, now_s, speed) helper in src/render/highway_draw.cpp | none |  |
| 225 | duplicate | 3 | 6 | `gain-clamp-owner` | open | no | no | Lowest output gain clamp written in both the playhead and the transport | Playhead::set_gain in src/audio/player.h | none |  |
| 226 | duplicate | 3 | 6 | `obj-fan-helper` | open | no | no | Polygon-to-triangle order written twice in obj_loader.cpp | one fan-triangulation helper in src/render/obj_loader.cpp | none |  |
| 227 | duplicate | 3 | 6 | `opus-rate-constant` | partly_fixed | no | no | Opus decode rate 48000 typed twice and its largest packet worked out by hand | one kOpusRate constant in src/audio/opus_reader.cpp, with kMaxFrame and kPreRoll derived from it | none |  |
| 228 | duplicate | 3 | 6 | `ch-probe-tools` | open | no | no | Probe constants.py holds the 85 ms normal back window twice (0.085 s and 85.0 ms) | tools/ch_probe/constants.py: EXPECT_NORMAL_BACK_MS = EXPECT_NORMAL_BACK_S * 1000 | none; developer probe tool only. |  |
| 229 | duplicate | 3 | 6 | `ch-probe-tools` | open | no | no | Old diagnostics rebuild the engine search pattern and module span inline | engine_finder.normal_pattern / all_patterns and engine_finder.MODULE_SPAN | none; developer probe tool only. | (low) Should the three old probe scripts (find_engine, find_clock3, poll_windows) be deleted or kept? Task 21 step 10 asked and it looks unanswered. |
| 230 | duplicate | 3 | 6 | `ch-probe-tools` | open | no | no | Engine bytes, fields and the precision bit are decoded by hand outside process.py and... | process.py decode_* for bytes; EngineModel for fields with a one-read snapshot method; pure is_precision(flags); one s_to_ms helper | none; developer probe tool only. |  |
| 231 | duplicate | 3 | 6 | `ch-probe-tools` | open | no | no | Process memory read/write is bound twice, in process.py and debugger.py | tools/ch_probe/process.py; the debugger keeps only the instruction-cache flush | none; developer probe tool only. |  |
| 232 | duplicate | 3 | 6 | `ch-probe-tools` | open | no | no | The 'did the press hit' check is written three times, settle time twice | one 'pressed input hit?' helper beside EngineModel.score in engine.py, with one settle constant | none; developer probe tool only. |  |
| 233 | duplicate | 3 | 6 | `ch-probe-tools` | open | no | no | probe_chart.py and probe_songs.py each write probe .chart text with their own constants | tools/ch_probe/probe_chart.py builders and constants.PROBE_CHART_NOTE_KICK; one resolution/BPM constant pair | none; developer probe tool only. |  |
| 234 | duplicate | 3 | 6 | `ch-probe-tools` | open | no | no | Probe song folder, sub-folder names, name prefix and folder writer spelled out twice | tools/ch_probe/probe_songs.py: DEFAULT_OUT, sub-folder names, name prefix and write_song | none; developer probe tool only. |  |
| 235 | duplicate | 3 | 6 | `ch-probe-tools` | open | no | no | Normal and precision window predictors repeat the same inner formula | one private helper in tools/ch_probe/experiments/analysis.py returning the scaled parabola term | none; developer probe tool only. |  |
| 236 | duplicate | 3 | 6 | `ch-probe-tools` | open | no | no | Finding and focusing the Clone Hero window is written four times | a small window helper beside constants.PROCESS_NAME or in input_driver, seeded from active_probe.find_game_window | none; developer probe tool only. |  |
| 237 | duplicate | 3 | 6 | `ch-probe-tools` | open | no | no | milestone1 re-lists the probe's constant set instead of asking EngineModel | engine.EngineModel.constants | none; developer probe tool only. |  |
| 238 | duplicate | 3 | 6 | `ch-probe-tools` | open | no | no | Three Python .chart readers pick drum notes differently | one shared Python .chart note reader for the probe tests and play_chart | none; developer probe tool only. |  |
| 239 | duplicate | 3 | 6 | `ch-probe-tools` | open | no | no | Tick-to-ms identity assumed in four places instead of calling ms_to_ticks | tools/ch_probe/probe_chart.py ms_to_ticks plus an inverse ticks_to_ms | none; developer probe tool only. |  |
| 240 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | Tests and the fill report hand-write key parts instead of asking Settings and Lens | Settings::record_key and store::Lens::from | none |  |
| 241 | duplicate | 3 | 6 | `dynamic-names` | open | no | no | A note's dynamic has two name tables in model.cpp | src/core/model.cpp dynamic_str (display sites capitalise) | none |  |
| 242 | duplicate | 3 | 6 | `report-chart-count` | open | no | no | Path report counts charts twice: C++ subtitle and JavaScript tile | src/app/report.cpp generate_report (unfiltered count in payload) | none |  |
| 243 | duplicate | 3 | 6 | `allzero-one-definition` | open | no | no | Two different rules decide whether the all-0 section is redundant | src/core/model.cpp: one redundancy predicate called by build_path_list | none |  |
| 244 | duplicate | 3 | 6 | `bass2x-available` | open | no | no | The 2x Bass checkbox checks 'Expert only' itself | src/app/config.cpp Settings (new bass2x_available()) | none |  |
| 245 | duplicate | 3 | 6 | `allzero-one-definition` | open | no | no | Search code restates part of the all-0 path definition | src/core/model.cpp Path::is_allzero | none |  |
| 246 | duplicate | 3 | 6 | `graph-build-cap` | open | no | no | Targeted search builds its graph at the raw cap, not graph_build_cap | src/search/pather.cpp graph_build_cap | none |  |
| 247 | duplicate | 3 | 6 | `multiplier-squeeze-rule` | open | no | no | Multiplier-squeeze direction guessed from combo, not from note payouts | src/core/scoring.cpp category_scores | none |  |
| 248 | duplicate | 3 | 6 | `multiplier-squeeze-rule` | open | no | no | Copy path builds the multiplier-squeeze 'Nx' label itself | src/core/model.cpp MultSqueeze::notationstr | none |  |
| 249 | duplicate | 3 | 3 | `preview-stale-chart-mode` | open | no | no | Two 'same path' rules; the Preview cache key skips SP facts | src/core/model.cpp: one path-identity helper beside Path | none |  |
| 250 | duplicate | 3 | 2 | `disco-flip-gate` | open | no | no | 'Disco flip only under Pro Drums' is written once in each parser | src/parse/song.cpp emit_chord_timestamp | none |  |
| 251 | duplicate | 3 | 6 | `parent-folder-helper` | changed | no | no | A path's parent folder is worked out by three hand-written helpers | src/core/winstr (one parent-folder helper) or std::filesystem::path | none |  |
| 252 | duplicate | 3 | 6 | `store-sql-owners` | open | no | no | 'Rows at this cap' is written twice in the store's lookup SQL | src/store/record_store.cpp (one cap-match helper and binder) | none |  |
| 253 | duplicate | 3 | 6 | `store-sql-owners` | open | no | no | Summary column list and order are hand-written in several SQL statements | src/store/record_store.cpp kSummaryColumnList plus a column-count constant | none |  |
| 254 | duplicate | 3 | 6 | `batch-running-owner` | open | no | no | 'Is a batch or analysis running?' is re-spelled next to its owner | src/ui/app_state.cpp AppState::batch_running / analyze_running | none |  |
| 255 | duplicate | 3 | 2 | `lower-diff-dynamics` | open | no | no | The chart's note total is counted three separate ways | src/parse/song.cpp: Song-level note count beside sp_phrase_count | none |  |
| 256 | duplicate | 3 | 6 | `scan-item-conversion` | open | no | no | One scanned-chart row is defined twice and converted by hand three times | src/app: one to_library_entry(ScanItem) or ScanItem as an alias | none |  |
| 257 | duplicate | 3 | 6 | `app-identity-constants` | open | no | no | Taskbar ID and app name are typed in both version.h and the installer | CMakeLists.txt next to the version, passed to version.h and Inno with /D | none |  |
| 258 | duplicate | 3 | 2 | `time-signature-zero` | open | no | no | Ticks-per-measure rule restated in Song's constructor and test fixtures | src/parse/song.cpp apply_timesig (constructor calls it) | none |  |
| 259 | duplicate | 4 | 1 | `squeeze-rating-one-rule` | open | partly | no | 'Is this row decided by frontend timing' is answered twice in rate_activation | src/core/squeeze_rating: one is_frontend_decided(row) | none |  |
| 260 | duplicate | 4 | 6 | `graph-build-cap` | open | no | no | Meter height min(cap, phrase count) is computed twice in search | src/search/graph.cpp ScoreGraph::max_sp_bars | none |  |
| 261 | duplicate | 4 | 6 | `allzero-one-definition` | open | no | no | All-0 path's cost against optimal is worked out twice in path_view | src/app/path_view.cpp: one helper used by build_path_buttons; drop allzero_label's delta | none |  |
| 262 | duplicate | 4 | 6 | `dynamics-keys` | open | no | no | Dynamics counts keyed by file path in memory but by md5 in the store | src/app/dynamics_breakdown.cpp dynamics_store_key | none |  |
| 263 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | Fill-deadline tests retype the deadline formula, and the 1.1 case pins nothing | src/search/graph.cpp activation_fill_deadline_ms (tests pin literals) | none |  |
| 264 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | Squeeze-rating tests restate effective-ms and use a different beyond-edge rule | src/core/squeeze_rating.cpp effective_backend_ms and beyond_edge_ms | none |  |
| 265 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | Report test fixtures recompute DM delta, percent and status | src/app/dm_report.cpp collect_dm_rows (and fill_report's row builder) | none |  |
| 266 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | Model and replay tests retype the per-note multiplier rule from scoring.cpp | src/core/scoring.cpp category_scores (tests pin literals) | none |  |
| 267 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | A store test retypes the schema 2 column list and invents its table layout | src/store/record_store.cpp kSchema2ResultsColumns (+ schema 2 DDL exposed to tests) | none |  |
| 268 | duplicate | 4 | 6 | `path-activation-order` | open | no | no | Path activation order is written in ActivationWalk and again in Path helpers | src/core/model.h ActivationWalk | none |  |
| 269 | duplicate | 4 | 6 | `shell-path-owner` | fixed | no | no | report_file_exists repeats winstr's file-exists test | src/core/winstr | none |  |
| 270 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | Frame clear colour copied into the GUI test harness | src/ui/app_shell.h (one constant) | none |  |
| 271 | duplicate | 4 | 6 | `ch-probe-tools` | open | no | no | EngineModel.constants spells key prefixes twice; seconds stored in _ms names | tools/ch_probe/constants.py CONST_KEY_*; rename process.verify_targets variables to back_s/front_s | none |  |
| 272 | duplicate | 4 | 6 | `ch-probe-tools` | open | no | no | Three test fakes re-implement Process.resolve's base-plus-offset math | tools/ch_probe/process.py Process (tests build a real Process with fakes) | none |  |
| 273 | duplicate | 4 | 6 | `ch-probe-tools` | open | no | no | Tests restate probe timing values; 3 ms key hold default written twice | named constants in engine_finder, input_driver (used by interfaces.py) and walk_edges | none |  |
| 274 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | Scan snapshot path rule is written in bench and again in its test | src/core/strutil: one shared relative-path helper | none |  |
| 275 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | Test cache key keeps its own list of record-changing settings | src/search/pather.h: a key function next to SearchSettings | none |  |
| 276 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | Tests type layout and timing constants by hand instead of reading them | production constants (kRowMeasureX, PreviewConfig, kSpanEndTicks, scaled_style) | none |  |
| 277 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | Test helpers are copied between test files | tests/corpus_util.h for the corpus loop; a shared audio test header | none |  |
| 278 | duplicate | 4 | 6 | `tests-recompute-production` | changed | no | no | Mixer test oracle re-writes the stem summation | src/audio/mixer.cpp add_into (oracle may stay on purpose) | none |  |
| 279 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | Three tests restate production layout and SP formulas instead of calling them | render::track_height and render::bottom_left_room | none |  |
| 280 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | Unit tests retype the GUI tests' analysis settings and expected Paths text | one shared test helper for the uitest scratch settings | none |  |
| 281 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | Score-box test recomputes the withheld-solo total instead of reading the replay | src/core/replay.cpp cum_onscreen_total | none |  |
| 282 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | Test helper retypes the Path-to-ReplayScore field pairing in reverse | src/core/replay.cpp score_of plus a reverse helper | none |  |
| 283 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | Replay warning test picks fixtures with its own 500 ms window checks | one core helper meaning 'within kSqueezeWindowMs' | none |  |
| 284 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | run_search test spells out the all-0 search options itself | src/search/pather.cpp search_allzero (expose allzero_options()) | none |  |
| 285 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | Ghost-kick test re-inlines check_invariants' tick and code checks | tests/test_song.cpp check_invariants (option to skip the fill check) | none |  |
| 286 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | test_song.cpp keeps its own MIDI writer beside tests/midi_util.h | tests/midi_util.h (varlen delta and multi-track variant) | none |  |
| 287 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | Per-process temp-path recipe copied across about a dozen test files | tests/temp_util.h built on core/winstr wide_to_utf8 | none |  |
| 288 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | Two WCAG contrast formulas in tests with different thresholds | one shared test helper (WCAG 2.2, 0.04045) | none |  |
| 289 | duplicate | 4 | 6 | `ui-label-functions` | open | no | no | Analyze button label rule retyped in the GUI test helper | src/ui/details_panel.cpp analyze_button_label(RecordStatus) | none |  |
| 290 | duplicate | 4 | 6 | `ui-label-functions` | open | no | no | Analyze search button label rebuilt in tests and twice in the toolbar | src/ui/library_toolbar.cpp one label function | none |  |
| 291 | duplicate | 4 | 6 | `ui-label-functions` | open | no | no | Backend table ID format rebuilt in the GUI test | src/ui/paths_tab.cpp (export the id builder) | none |  |
| 292 | duplicate | 4 | 6 | `ui-label-functions` | open | no | no | Layout tests type their own longest activation badge | src/app/path_view.cpp activation_badge (or a longest-badge sibling) | none |  |
| 293 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | WARP test device and texture read-back written twice | tests/warp_util.h | none |  |
| 294 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | GUI test name matching written twice in hydra_uitest | tests/ui/uitest_harness.cpp: one select function | none |  |
| 295 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | ImGui text size formula retyped three times in GUI tests | tests/ui/uitest_harness: one text-width helper | none |  |
| 296 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | GUI tests type 4 instead of the Clone Hero SP cap constant | src/core/model.h kCloneHeroSpCap | none |  |
| 297 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | Column-order test types the Best path column index as 4 | src/ui/library_table.cpp kColumn* exposed beside LibrarySort | none |  |
| 298 | duplicate | 4 | 6 | `store-schema-number` | open | no | no | Database schema number is written every open but never read | src/store/record_store.cpp has_column probes (or user_version moved to stored_versions.h) | none | (low) Should the database's schema number be dropped (the column checks stay the gate), or made the real gate and moved into stored_versions.h? |
| 299 | duplicate | 4 | 6 | `store-sql-owners` | open | no | no | The rescan-cache column is checked twice; the second check is dead | src/store/record_store.cpp RecordStore constructor has_column + ALTER | none |  |
| 300 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | Hand-built test Songs mark SP phrase ends without sp_phrase_start | tests: one shared test Song builder mirroring mark_sp_phrase_end | none |  |
| 301 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | test_search retypes the record byte-equality check record_bytes.h owns | tests/record_bytes.h record_bytes | none |  |
| 302 | assumption | 1 | 3 | `percent-rounding` | open | no | no | Dynamics tab percentages round down, so shares read up to one point low | src/app/display_format.h (one shared percent formatter) | Dynamics percentages round to nearest, so 12 of 453 reads 3% not 2%, and 999 of 1000 reads 100%. | (low) Should the Dynamics tab round percentages to nearest (like the progress bars) instead of cutting off the decimals? |
| 303 | assumption | 2 | 1 | `squeeze-rating-one-rule` | open | yes | no | Nobody chose the 1 ms impact gate or the 0.005 scale tolerance | src/core/squeeze_rating (rate_note, is_scaled); path_view format_scale | Every row or SqIn with a multiplier other than exactly 1 shows its effective figure; the scale line gains extra decimals when two would read x1.00. |  |
| 304 | assumption | 2 | 7 | `leeway-edge` | open | no | no | A note exactly 3.0 ms after SP end counts as uncounted, with no decision | src/core/backend_value.h counted_without_squeeze | Only a backend row at exactly +3.0 ms could change from 0 points and Hard (uncounted) to full points and Standard. | (high) Should a backend note sitting exactly on the 3 ms leeway edge still count (inclusive), or not (as now)? |
| 305 | assumption | 2 | 3 | `early-fill-badge` | open | no | no | Optional early fills get a badge, reversing the approved plan's test | src/core/model.h Activation (one rule for 'needs a squeeze') | An activation that skips fills would show or lose its 'early fill N ms' badge. | (low) Should an activation whose early fill is optional (it skips fills) still get an 'early fill' badge? |
| 306 | assumption | 2 | 3 | `rating-tiers` | open | no | no | Backend rating bands start at a fixed 10 ms nobody chose | src/core/model.cpp BackendSqueeze::summarystr (named constant or Rules field) | Rating labels near +/-10 ms (and with a leeway of 10 or more) could change; the User Guide would name backend_leeway_ms instead of a fixed 3. | (high) Should the inner rating-band edges stay at a fixed 10 ms, or follow the hit window or the leeway setting? |
| 307 | assumption | 2 | 7 | `cap-clamp-tie` | open | no | unsure | An exact tie with the cap ceiling is not counted as a clamp | src/search/graph.cpp ScoreGraph::extend_deacts | none (an exact tie shows no overfill warning today) | (high) When the cap ceiling and the plain +2 measures land on the same tick, should that count as an overfill clamp (as the squeeze anchor sees it)? Today it does not. |
| 308 | assumption | 2 | 3 | `preview-gauge-cap` | open | no | no | Unanalyzed song's Preview gauge pins at 4 bars, ignoring the settings cap | src/app/config.cpp Settings::sp_cap | With the cap set above 4 and a song not yet analyzed, the Preview gauge pins at the settings cap, not at 4. | (low) For a song with no result yet, should the Preview gauge use the SP cap from Settings (what the next analysis would use) or Clone Hero's fixed 4? |
| 309 | assumption | 2 | 3 | `dm-statuses` | open | no | no | Leaderboard calls every score at or below optimal 'matched' | src/app/dm_report.cpp collect_dm_rows | Status chip names and the speed rule for % of optimal could change in the leaderboard comparison. | (low) Should 'matched' keep meaning 'joined by chart hash', or should the leaderboard use different status names, and should other speeds show a percent? |
| 310 | assumption | 2 | 3 | `avg-multiplier-empty` | open | no | no | Average multiplier shows 0.000x when a path has no scoring notes | src/core/model.cpp Path::avg_mult | A path with no scoring notes could read 'n/a' instead of 'Avg. Multiplier: 0.000x'. | (low) What should the details text show for the average multiplier of a path with no scoring notes: 0.000x or a dash? |
| 311 | assumption | 2 | 7 | `settings-validation` | open | no | no | A negative path depth is fixed only in the GUI; INI and CLIs crash the search | src/app/config.cpp Settings::load_file (one validator for GUI, INI and CLIs) | A hand-edited depth_value=-1 or --depth -1 stops failing with 'search reached a broken state'. | (low) When the path depth is negative in the INI or on the command line, should it be clamped to 0 (as the GUI does) or refused with an error? |
| 312 | assumption | 2 | 3 | `fill-compare-label` | open | no | no | Fill comparison labels a chart with no score anywhere as 'Only in 1.1 db' | src/app/fill_report.cpp collect_fill_rows | A chart whose 1.0 record has no paths would stop being listed under 'Only in 1.1 db'. | (low) Should the fill comparison label a chart by which database holds a record, instead of by whether it has a score? |
| 313 | assumption | 2 | 7 | `early-fill-window` | open | no | unsure | 60 ms early-fill window has no recorded user decision for its value | src/core/model.h kEarlyFillWindowMs (plus a decision record); is_e0 should call is_e_critical | none if 60 is confirmed; a different value changes which activations exist and where E appears. | (high) Is 60 ms the right early-fill window (it was narrowed from 85 using a corpus check, hardest real E0 is 57.7 ms)? |
| 314 | assumption | 2 | 2 | `ticks-per-beat` | open | no | unsure | A beat is always a quarter note in every meter, with no named rule | src/core/timing.h SongTiming: a named ticks-per-beat | none if confirmed; if Clone Hero counts beats differently in 6/8 or 7/8, some fills there would be offered or refused differently and Preview beat lines would change. | (high) Does Clone Hero count fill beats as quarter notes in compound meters like 6/8 and 7/8, as Hydra assumes? |
| 315 | assumption | 2 | 2 | `fill-placement-rules` | open | no | unsure | Fill placement tie-breaks, truncation and silent drops have no user decision | src/parse/song.cpp Song::check_activations and fill_lands_on_chord | none if confirmed; changing a rule could move or add activation points on charts with equidistant chords or odd resolutions. | (high) Confirm the fill rules Hydra applies today: ties go to the later chord, distances are cut to whole ticks, one authored fill stops fill generation, and a fill ending before its start is dropped. |
| 316 | assumption | 2 | 7 | `tie-merge-key` | open | no | unsure | Tied paths merge without comparing when their meter reached 2 bars | src/search/engine.cpp Engine::reduce_iteration_paths | none if the key stays; adding sp_ready_ms could add tied variants to the Paths list. | (high) Should the path-merge key include the moment the meter reached 2 bars (sp_ready_ms), or stay as now with a recorded decision? |
| 317 | assumption | 2 | 7 | `retired-auto-cleanup` | open | no | no | Auto cleanup recognises only default-ladder rows; custom-ladder rows wait for re-analysis | src/core/rules.cpp Rules::retired_auto_fingerprint | Custom-ladder Auto rows would be deleted at first start instead of reading Stale until the chart is re-analyzed. | (low) Should the one-time Auto cleanup also delete rows from a custom (hand-edited) cap ladder, or leave them to disappear on re-analysis? |
| 318 | assumption | 2 | 3 | `time-format` | open | no | no | Preview time box can read 0:60.000 instead of 1:00.000 | src/app/preview_view.cpp clock_str (round to whole ms, then split) | The time box reads 1:00.000 where it now reads 0:60.000 for the last half millisecond of each minute. |  |
| 319 | assumption | 2 | 2 | `time-signature-zero` | open | no | no | Charts with no time signature read as 4/4; default written three ways | src/parse/song.h Song constructor via apply_timesig(0,4,4); build_time_box reads scene only | none (an empty Preview would follow Song's default instead of its own literal). | (low) Confirm that a chart with no time signature is read as 4/4, and that a .chart 'TS n' with no second number means n/4. |
| 320 | assumption | 2 | 2 | `chart-dynamics-gate` | open | no | unsure | Dynamics: .chart files always count ghosts and accents and report dynamics on | src/parse/song.cpp ChartParser::parse | none if confirmed; if Clone Hero gates .chart dynamics, scores and counts for every .chart would change. | (high) Does Clone Hero score .chart ghost and accent notes without any enabling tag, as Hydra assumes? |
| 321 | assumption | 2 | 7 | `scan-metadata-cap` | open | no | no | Scan reads .sng and .srb names from only the first 1 MB | src/app/analysis.cpp discover_charts (kSngHeadCapture) | none for normal files; a file with metadata past 1 MB would show its real title instead of '(unknown)'. | (low) Is a 1 MB cut-off on reading .sng and .srb names acceptable, or should the scan read the whole metadata block like the Preview does? |
| 322 | assumption | 2 | 7 | `settings-validation` | open | no | no | Settings value ranges: some enforced only in the UI, some never | src/app/config.cpp Settings (one range table built from kSqueezeWindowMs) | A hand-edited out-of-range setting is corrected when loaded instead of silently changing on first click. | (low) Which ranges should Settings enforce on load (for example ms limit above 500, backend limit sign, depth_mode other than 0/1) and what should replace a bad value? |
| 323 | assumption | 2 | 7 | `library-query-squeeze` | open | no | no | Library search 'squeeze<N' quietly means 'at most N' | src/app/library_query.cpp parse_library_query | none if kept; if made strict, squeeze<20 would stop listing a song at exactly 20.0 ms. | (low) Should library search 'squeeze<N' be strictly less than N, or keep meaning 'at most N' like squeeze<=N? |
| 324 | assumption | 2 | 6 | `allzero-one-definition` | open | no | no | One duplicate hides the whole 'Best all-0 path' section | src/app/path_view.cpp build_path_list (one redundancy predicate, agreeing with the engine's test) | Non-duplicate all-0 variants would stay visible when another variant duplicates a listed path. | (low) If one all-0 variant duplicates a listed path, should the whole 'Best all-0 path' section hide, or only the duplicate? |
| 325 | assumption | 2 | 7 | `rules-file-ranges` | open | no | no | hydra_rules.ini lower bounds were picked in code, not by the user | src/app/rules_file.cpp load_rules_file | A rules-file value that is refused today (or accepted) might change. | (low) Which values should hydra_rules.ini accept for backend_leeway_ms, max_tied_paths and the fill settings (today 0 is fine for leeway but refused for fill length and tied paths)? |
| 326 | assumption | 2 | 4 | `cli-messages` | open | no | no | hydra_bench corpus mode benchmarks with no timing limit, unlike the GUI default | tools/bench.cpp corpus_bench (keep fixed, relabel 'cap4 d4 no-ms'; use kCloneHeroSpCap) | The hydra_bench corpus output line gains an honest label such as 'cap4 d4 no-ms'. | (low) Should hydra_bench corpus mode stay fixed at no timing limit (just relabelled) or follow the GUI default of 10 ms? |
| 327 | assumption | 2 | 4 | `record-decisions` | changed | no | no | Preview, audio and leaderboard literals with no recorded user decision | each literal's own module (src/render, src/audio, src/net); decisions recorded in an ADR or CONTEXT.md | none | (low) Do you accept the unrecorded tuning numbers in the renderer, audio and leaderboard client (overlay min scale 0.6, 50 ms cancel poll, flat-line epsilon, magenta fallback colour, 64-byte Ogg tag window, WinHTTP timeouts, status 200 only) as they are? |
| 328 | assumption | 2 | 6 | `ch-probe-tools` | open | no | no | Probe decision cut-offs have no user decision behind them | tools/ch_probe/constants.py (one named constant per threshold) | none | (low) Do you accept the probe's current cut-offs (10 ms hit-time gap, 2 ms clock freshness, 1.0 ms and 80% clamp verdict) or want different ones? |
| 329 | assumption | 2 | 3 | `preview-beat-grid-end` | open | no | no | Preview beat grid stops a hard-coded two measures past the last note | src/app/preview_view.cpp build_preview_base (named constant, or the transport length) | Beat lines would continue to the end of the audio instead of stopping two measures after the last note. | (low) Should the Preview's beat lines run to the end of the song's audio, or keep stopping two measures after the last note? |
| 330 | assumption | 2 | 7 | `path-limit-optimal-tie` | open | no | unsure | Path limit keeps an over-limit path when it ties the optimal score | src/search/engine.cpp Engine::reduce_group; User Guide line 51 states the rule | With a 10 ms limit, a second path that ties the optimal score but needs a larger squeeze (78.9 ms on 'I Am... All Of Me') would either stay (guide updated) or disappear. | (high) Should a path that ties the optimal score be kept even when its hardest squeeze is over the Path limit (as now)? If yes, the User Guide just needs to say so. |
| 331 | assumption | 2 | 2 | `chart-modifier-no-note` | open | no | no | A .chart cymbal, ghost or accent marker with no note under it fails the whole chart | src/parse/song.cpp ChartParser (one malformed-modifier rule, same as the duplicate-note rule) | Such a chart would load (marker ignored) or fail with a clear chart error, instead of the raw 'apply_cymbal: note not present' message. | (low) When a .chart cymbal, ghost or accent marker has no note at its tick, should the marker be ignored or the chart rejected with a clear error? |
| 332 | assumption | 3 | 1 | `transfer-scale-unknown` | open | no | unsure | Unknown transfer ratios are silently stored as x1.00 | src/search/engine.cpp copy-out (explicit 'unknown' in the record) | none for normal charts; a record with unknowable ratios would stop looking like a flat-tempo chart. | (high) Should a stored record be able to say 'transfer scales unknown' (as ADR 0011 says) or keep storing x1.00 and ADR 0011 gets corrected? |
| 333 | assumption | 3 | 3 | `rating-tiers` | open | no | no | Timing tier ladder shape has no recorded user decision | src/core/squeeze_rating.cpp timing_tiers | none if confirmed; a different ladder would move paths between the report's tier chips. | (high) Is the timing tier ladder right: a 2 ms floor, then W/2, W, 3W/2 and 2W, then Beyond? |
| 334 | assumption | 3 | 6 | `sp-activation-minimum` | open | no | no | The two-bar activation minimum is a bare literal in four engine lines | src/core/model.h: a named constant beside kCloneHeroSpCap | none | (low) Confirm that Clone Hero needs 2 banked bars (half a meter) to activate, so it can be recorded in CONTEXT.md. |
| 335 | assumption | 3 | 6 | `multiplier-squeeze-rule` | open | no | no | Cymbal, solo and multiplier-step values have no recorded decision | src/core/model.h and to_multiplier in src/core/timing.cpp; MultSqueeze::applies derives from to_multiplier | none | (low) Confirm the scoring values taken from upstream (cymbal +15, solo 100 per note, multiplier steps at 10/20/30) so they can be recorded. |
| 336 | assumption | 3 | 4 | `record-decisions` | open | no | no | Star cutoffs use a 32-bit float multiply not recorded as a decision | src/core/stars.cpp (owner); CONTEXT.md records the float detail | none | (low) Confirm that star cutoffs multiply as 32-bit floats (taken from the decompiled game) so the detail can go into CONTEXT.md. |
| 337 | assumption | 3 | 4 | `record-decisions` | open | no | no | Preview spans end half a tick after their last note, with no recorded choice | src/render/track_state.cpp kSpanEndTicks (single owner) | none | (low) Should Preview SP, solo and fill spans end half a tick after their last note (the 2026-09-24 choice) and be recorded? |
| 338 | assumption | 3 | 4 | `record-decisions` | open | no | no | Preview audio offset rule is a user decision but lives only in a plan | src/app/preview_source.cpp preview_audio_offset_ms (already the owner); ADR or CONTEXT.md entry | none |  |
| 339 | assumption | 3 | 4 | `record-decisions` | open | no | no | Preview's own thresholds and fallbacks have no user decision | src/app/preview_view.cpp (slack, BPM 0.0) and src/parse/song.h (4/4 default) | none | (low) Do you accept the Preview's half-millisecond 'on an activation' slack, half-tick span end and BPM 0.0 fallback as they are? |
| 340 | assumption | 3 | 6 | `preview-overlay-key` | open | no | no | Show in Preview parses an overlay key it was told not to parse | src/ui/preview_controller.cpp: a matches(path_key) method beside overlay_key | none |  |
| 341 | assumption | 3 | 6 | `tests-recompute-production` | open | no | no | Scrubber marks copy ImGui's hidden 2 px slider padding | src/ui/preview_tab.cpp draw_scrub_marks (single owner; add a GUI test against the real grab rect) | none |  |
| 342 | assumption | 3 | 6 | `ch-probe-tools` | open | no | no | Engine scan silently assumes the front window sits 8 bytes after back | tools/ch_probe/engine_finder.py (derive the gap from constants.py) | none |  |
| 343 | assumption | 3 | 7 | `stamp-rules` | open | no | unsure | Stored tempo map is written once and never refreshed or stamped | src/store/record_store.cpp upsert_song, or a StampRule in src/store/stored_versions.h | none today; after a parser change, re-analysis would refresh the measure text and timeline positions too. | (high) When a chart is re-analyzed, should the stored tempo map be rewritten every time, or given its own version stamp? |
| 344 | assumption | 3 | 7 | `stamp-rules` | open | no | no | Results stamp's bump rule leaves out the parser; the dynamics rule includes it | src/store/stored_versions.h kResultsStamp comment and ADR 0018 | none; after a parser-only fix, library chips would go Stale and re-analysis would be needed. | (high) Should a change in src/parse that alters chart output require bumping kResultsStamp (so records re-analyze), like the dynamics stamp already does? |
| 345 | assumption | 3 | 7 | `stamp-rules` | open | no | no | Rescan cache reuses chart names and hashes with no reader stamp | src/store/stored_versions.h: a chart-metadata StampRule checked in RecordStore::chart_library_cache (or an ADR accepting reuse) | none today; after a reader change, unchanged files would refresh their artist and charter on rescan. | (low) Should the rescan cache carry a version stamp so a change to the title, artist, charter or hash readers refreshes unchanged files, or is reuse until the file changes accepted? |
| 346 | assumption | 4 | 6 | `backend-offset` | changed | no | no | A backend row with no stored offset is read as 0 ms in seven places | src/core/model.h: make offset_ms non-optional, or one accessor | none |  |
| 347 | assumption | 4 | 6 | `preview-gauge-cap` | open | no | no | Preview guesses a 4-bar cap for a record missing one, which can't happen | store: every Ready record carries sp_cap; Preview reads it with no fallback | none |  |
| 348 | assumption | 4 | 4 | `record-decisions` | open | no | no | Half-millisecond slack for 'playhead is on an activation' has no recorded choice | src/app/preview_view.cpp kOnActivationMs (single owner) | none | (low) Do you accept half a millisecond as the Preview's 'playhead is on an activation' slack? |
| 349 | assumption | 4 | 4 | `record-decisions` | open | no | no | Rules fingerprint encoding is a code choice, not in ADR 0014 | src/core/rules.cpp (already the only place); ADR 0014 gains a sentence | none | (low) Do you accept the rules fingerprint encoding (FNV-1a 64 over name=value text, 17 digits, 0 mapped to 1) and want it written into ADR 0014? |
| 350 | assumption | 4 | 6 | `tests-recompute-production` | open | no | no | Dynamics bad-blob test assumes the stamp is byte 0 and 2 is rejected | store::kDynamicsBlobStamp (test uses written + 1) | none |  |
| 351 | assumption | 4 | 4 | `record-decisions` | open | no | no | Batch 'time left' waits for three finished charts | src/ui/library_jobs batch_eta_s / kEtaMinFinished (already the owner) | none |  |
| 352 | assumption | 4 | 6 | `tests-recompute-production` | open | no | no | Preview overlay test assumes the button list order equals all_paths order | src/app/path_view.cpp build_path_buttons (test finds the button by its path pointer) | none |  |
| R7.1 | drift | 2 | 3 | `time-format` | open | no | no | Preview load and batch strip show 'time left' by two rules and two wordings | src/ui: one time-left formatter beside format_duration | The Preview load and the batch strip would word 'about ... left' the same way. | (low) Which wording should 'about ... left' use everywhere: m:ss (about 1:30 left), whole seconds then minutes (about 2 min left), or something else? |
| R7.2 | drift | 2 | 7 | `auto-open-report` | open | no | no | 'Open automatically' says it covers finished batches, but also opens every leaderboard co | src/app/config.h beside Settings::auto_open_report (one written meaning) | Either leaderboard comparisons stop opening on their own, or the hint and User Guide read 'whenever a report is built'. | (low) Should 'Open automatically' cover only finished batches (the leaderboard comparison stops auto-opening), or should the hint and User Guide say it opens every report? |
| R7.3 | drift | 2 | 7 | `can-scan-owner` | open | no | no | Two side doors start a library scan in the middle of a batch, and the toolbar button refu | src/ui/app_state.cpp: one AppState::can_scan() enforced by start_scan | 'Scan now' and 'Rescan library' would refuse while a batch runs, like the Scan library button does. | (low) Should a library scan be blocked while a batch is running (as the toolbar button already does), or allowed from the other two buttons? |
| R7.4 | drift | 2 | 3 | `stale-message` | open | no | no | Settings bar says 'this song' is analyzing while the panel shows a different song | src/ui/app_state.cpp AppState::analyze_job_shown | The settings bar names the analyzing song instead of saying 'this song' when you have moved to another one. |  |
| R7.5 | drift | 2 | 2 | `offset-delay-parse` | open | no | no | A .chart Offset with a unit is honoured, and a song.ini delay with a unit is dropped | src/core/strutil: one 'number from chart text' helper used by both parsers | 'Offset = 500ms' and 'nan' would be read the same way by .chart and song.ini instead of shifting Preview audio by 500 s or NaN. | (low) How strict should reading a .chart Offset or a song.ini delay be: reject any text with a unit like '500ms', and reject 'nan'? |
| R7.6 | drift | 2 | 2 | `negative-tempo` | open | no | unsure | Lookups assume ms never fall as ticks rise, and a negative .chart tempo breaks that | src/parse/song.cpp: reject or clamp tempos at or below zero where both parsers write bpm_changes | A .chart with a negative tempo (B -60000) would be refused or clamped instead of analyzed with the Preview and engine disagreeing and 'Score unavailable' showing. | (high) Should a .chart tempo of zero or below be rejected at load (with a clear error) or clamped to a positive value? |
| R7.7 | drift | 2 | 2 | `practice-sections-sorted` | open | no | no | Practice sections sorted from .chart but in track order from .mid; time box keeps the uns | src/parse/song.cpp MidiParser::parse (sort like ChartParser::parse); drop the is_sorted branch in build_time_box | The Preview time box names the right practice section on a .mid with several EVENTS tracks. |  |
| R7.8 | drift | 2 | 7 | `stem-length-contract` | open | no | no | A stem's length: the mix trusts the file header, and the old mixer counted what decoded | src/audio/stem_reader.h: one StemReader rule for 'length unknown or wrong' | A FLAC whose header says length 0 would play instead of going silent in the Preview; audio end would follow the decoded length. | (low) When a stem's file header gives a length of 0 (or a wrong one), should the Preview read it to the end (as the old mixer did) or stay silent? |
| R7.9 | drift | 2 | 7 | `stem-length-contract` | open | no | no | After a decode error, seeking back revives every stem except an Opus one | src/audio/stem_reader.h: one rule for failed() and seek after failure | A damaged Opus stem would play again after you seek back before the damage, like Vorbis, MP3, WAV and FLAC do. | (low) After a damaged Opus stem hits a decode error, should seeking back before the damage bring it back (as every other format does) or stay silent until the chart is reopened? |
| R7.10 | drift | 2 | 1 | `squeeze-window-500ms` | open | no | yes | For SP that outlasts the chart, graph and display apply the 500 ms window from different  | src/search/engine.cpp rebuild: one window helper against final_sp_end (finding 30's owner) | A tail activation could store and show backend rows further from the SP end than today, for newly analyzed records only. | (high) For the last activation of a path whose Star Power outlasts the chart, should backend rows be limited to within 500 ms of the SP end (as the display does) or of the last note (as the graph does)? |
| R7.11 | duplicate | 3 | 6 | `read-file-bytes-owner` | open | no | no | MidiFile::from_file sizes a file with the 32-bit ftell the 2 GB fix removed elsewhere | src/core/winstr.cpp read_file_bytes (from_file becomes MidiFile(read_file_bytes(path))) | none |  |
| R7.12 | duplicate | 2 | 5 | `shell-path-owner` | open | no | no | 'Is this path too long?' asked with two cut-offs; shell edge typed several times | src/core/winstr: named constants side by side plus one predicate such as fits_shell | none | (low) Is the 248-character plain-path limit (MAX_PATH - 12) the right cut-off, or should it be ADR 0020's 260? Record the answer in ADR 0020. |
| R7.13 | duplicate | 3 | 6 | `track-state-spans` | open | no | no | The state of a span at an instant is worked out twice in track_state.cpp | src/render/track_state.cpp: synthesize asks a one-query SpanSweep, or toggle_at moves to tests as the oracle | none |  |
| R7.14 | duplicate | 3 | 6 | `track-state-spans` | open | no | no | The taken-fill lane tie-break is written twice in track_state.cpp and again in the draw c | src/render/track_state.cpp TrackState carries the pad on each span (finding 76's owner) | none |  |
| R7.15 | duplicate | 3 | 6 | `track-state-spans` | open | no | no | The rule that turns moments into timeline instants is written twice | src/render/track_state.cpp build_track_state calls the same overlay merge as rebuild_overlay_fields | none |  |
| R7.16 | duplicate | 3 | 6 | `preview-track-options` | open | no | no | The Preview's highway options are built by hand four times and compared on one field twic | src/ui/preview_controller.cpp: one track_opts() and an operator== on TrackStateOptions | none |  |
| R7.17 | duplicate | 3 | 6 | `chart-format-one-owner` | open | no | no | A load asks 'is this a container, and which one?' in three ways | src/parse/chart_files.cpp chart_format_of (asked once per load) | none |  |
| R7.18 | duplicate | 3 | 6 | `preview-overlay-sizes` | open | no | no | The Preview overlay box sizes are worked out twice, once to fit and once to draw | src/ui/preview_tab.cpp: one box-measuring helper beside overlay_scale | none |  |
| R7.19 | duplicate | 3 | 6 | `midi-pitch-table` | open | no | no | The MIDI parser lists its drum pitches three ways | src/parse/song.cpp: one pitch table with the note-off gate derived from it | none |  |
| R7.20 | duplicate | 3 | 3 | `typed-error-kinds` | open | no | no | Error matching has fallen behind its throwers (extends finding 193) | src/app/user_messages.cpp plain_error_text (typed error kinds, as finding 193 proposes) | A file read error would say what went wrong instead of 'Something went wrong.' |  |
| R7.21 | duplicate | 3 | 6 | `audio-ms-frames` | open | no | no | The front-pad ms-to-frames conversion moved and was not removed (extends finding 182) | src/audio: one ms-to-frames helper callable from src/ui too | none |  |
| R7.22 | duplicate | 3 | 6 | `worker-count` | open | no | no | The 'at least one worker' floor is written three times; worker count only partly decided | src/app/analysis.cpp batch_worker_count (run_work_pool and the test seam trust it) | none | (low) Confirm the parts of the worker count the 2026-09-24 decision does not cover: 'hardware threads minus one', a guess of 1 when Windows reports 0, and the same count for the scan's hashing pool. |
| R7.23 | duplicate | 3 | 6 | `id3-sniff` | open | no | no | An ID3 tag in front of FLAC is MP3 to one rule and FLAC to another (extends finding 74) | src/audio/decode.cpp sniff_format returns Flac for an ID3-wrapped FLAC; open_ma_reader stops sniffing | none |  |
| R7.24 | duplicate | 3 | 6 | `mp3-reservoir` | open | no | no | The MP3 bit-reservoir rule is written twice in one file, with its 511 typed twice | src/audio/ma_reader.cpp: one reservoir_after() helper; kMaxReservoir from the miniaudio macro | none |  |
| R7.25 | duplicate | 3 | 6 | `preview-file-size` | open | no | no | A loose stem's file size is read twice per Preview load | src/audio/mapped_file.cpp MappedFile (job maps first, takes total from bytes.size()) | none |  |
| R7.26 | duplicate | 3 | 6 | `fill-rule-stamp` | open | no | no | The meta key 'engine_mode' is typed three times in record_store.cpp (extends finding 54) | src/store/record_store.cpp: a named constant and the getter | none |  |
| R7.27 | duplicate | 3 | 6 | `format-measure-owner` | open | no | no | The Preview time box types 'm1.1.0' twice instead of asking format_measure (extends findi | src/app/path_view.cpp format_measure | none |  |
| R7.28 | duplicate | 3 | 6 | `stem-converter-config` | open | no | no | The stem converter's settings and passthrough test are written twice (extends finding 278 | src/audio: one converter-config function used by StreamMix and convert_stem | none |  |
| R7.29 | duplicate | 4 | 6 | `keep-everything-band` | open | no | no | The targeted search's 'keep everything' band is a bare billion, retyped in a test | src/search/pather.h: a named constant the test reads | none |  |
| R7.30 | duplicate | 4 | 6 | `tests-recompute-production` | open | no | no | The equivalence tests keep frozen copies of the replaced loops (new instances of 164 and  | tests: pinned literal values instead of copied loops, if oracles are not exempt | none | (low) Are test oracles that keep a frozen copy of a replaced production loop exempt from the derive-once rule, or should they be removed once the switch is proven and replaced with pinned literal values? |
| R7.31 | assumption | 2 | 4 | `record-decisions` | open | no | unsure | Path limit is on at 10 ms and backend display limit defaults to 50 ms, with no recorded d | src/app/config.h Settings (defaults recorded in CONTEXT.md or the User Guide) | none if confirmed; a different Path-limit default changes which alternate paths every new analysis keeps. | (high) Do you confirm the defaults of a 10 ms Path limit (on) and a 50 ms backend display limit (off until ticked), so they can be recorded? |
| R7.32 | assumption | 3 | 4 | `record-decisions` | open | no | no | Preview loading bar's step shares and units have no decision from you | src/ui/preview_load_job.cpp (decision recorded if wanted) | none if confirmed. | (low) Do you accept the Preview loading bar's step shares (8% read, 82% open audio, 4.5% scene), MB rounding, one-second pacing and the switch to minutes at 59.5 s? |
| R7.33 | assumption | 3 | 3 | `ui-timings` | open | no | no | Short UI timings are bare numbers with no decision | src/ui/app_state.h: one named set of UI timings beside kFileCheckSeconds | none if confirmed. | (low) Do you accept the short UI timings ('Done!' 0.5 s, 'Copied' 2.0 s, search re-filter 0.15 s, batch refresh 1.0 s) and a shared rule with the 6.0 s fade from finding 219? |
| R7.34 | assumption | 3 | 4 | `record-decisions` | open | no | no | Numeric choices in the new audio readers have no decision | src/audio readers (opus_reader.cpp, ma_reader.cpp, stream_mix.h); record in ADR 0019 | none | (low) Do you accept the unrecorded audio reader numbers (zero-byte Opus packet = 120 ms, 512-packet queue, MP3 seek points every 0.5 s, 1 s hop, 4096-frame blocks, 4 GiB MP3 and 2 GiB Vorbis limits)? |
| R7.35 | assumption | 3 | 6 | `backend-offset` | open | no | no | The engine turns a missing fill deadline or squeeze timing into 0 ms (extends finding 346 | src/search/engine.cpp: map a missing value to NO_DOUBLE or refuse the graph, with finding 346's owner | none |  |
| R7.36 | assumption | 4 | 4 | `record-decisions` | open | no | no | Three tuning values have no decision and one owner each | src/ui/app_state.h kParkedLookups; src/search/engine.cpp progress step; src/search/pather.cpp kMainProgressShare | none | (low) Do you accept 16 parked record lookups, engine progress every 0.005, and 0.9 of the bar for the main pass? |
| R7.37 | doc | 2 | 4 | `adr-corrections` | open | no | no | 'A stem that hits a decode error goes silent from that point' is not what the code does | docs/adr/0019 and CONTEXT.md 'Mixer' (after the decisions on R7.8 and R7.9) | none |  |
| R7.38 | doc | 2 | 4 | `adr-corrections` | open | no | no | 'A straight read with no seek gives exactly the old samples' holds for one reader, not fo | docs/adr/0019 line 61 (after the decision on R7.8) | none |  |
| R7.39 | doc | 2 | 4 | `adr-corrections` | open | no | no | 'Chained files: each link plays in turn' is not what the Opus reader does | docs/adr/0006 line 31 and docs/adr/0019 lines 71-72, or src/audio/opus_reader.cpp | none if the ADRs are corrected; an Opus file with a later link of another channel count would play that link if the code is changed. | (low) For a chained Opus file whose later link has another channel count or no audio, should the reader keep the first link only (as now, and correct the ADRs) or play every link as ADR 0006 says? |
| R7.40 | doc | 3 | 4 | `adr-corrections` | open | no | no | ADR 0020 gives 260 as the only edge, and lists one fewer fallback than the code has | docs/adr/0020 (after the 248 vs 260 call in R7.12) | none |  |
| R7.41 | doc | 3 | 4 | `adr-corrections` | open | no | no | '1e-3 per sample after the first 20 ms' does not hold for odd rate pairs | docs/adr/0019 lines 66-67 | none |  |
| R7.42 | doc | 3 | 4 | `adr-corrections` | open | no | no | 'The MP3 reader checks every seek' is true only inside the scanned frames | docs/adr/0019 line 51 | none |  |
| R7.43 | doc | 4 | 4 | `adr-corrections` | open | no | no | 'The 3.1-hour overflow is gone' is true; the new limit it hints at is not new | docs/adr/0019 line 43 (add one sentence about the 2 GiB Vorbis limit) | none |  |
| R7.44 | doc | 3 | 4 | `adr-corrections` | open | no | no | ADR 0020's source-scan test is described as stronger than it is | docs/adr/0020 lines 41-42, or tests/test_long_paths.cpp (match recursive_directory_iterator, absolute, status, equivalent, space, current_path) | none |  |
