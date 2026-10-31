# AOA-A2: derivation audit 2026-10-03, 22 assumption findings that carry a decision

Code read at main 1fc55723. Paths are relative to the checkout. QF = `docs/audit/2026-10-04-phase-3-5-questions.md`, Q7 = `docs/audit/2026-10-04-phase-7-questions.md`, DEC = `docs/audit/2026-10-03-fix-decisions.md`.

Summary: no real split in any of the 22. They are numbers nobody chose, plus two code copies (330's tie rule, 335's multiplier list) that are now one. The weak spot is how the numbers were asked. The phase 3-5 question 33 lists 8 sub-bullets under "Renderer, audio and leaderboard tuning values" and cites 9 finding ids. Several of the cited ids have values that no sub-bullet shows the user.

### derivation 327: preview, audio and leaderboard literals

- **Splits:** None: twelve numbers nobody chose (overlay min scale 0.6, 50 ms cancel poll, flat-line epsilon 0.001, magenta fallback, 64-byte Ogg tag window, 4096 chunk, WinHTTP timeouts, status 200 only). Two code copies inside one module (magenta twice, "OpusHead" twice) were folded: `kBadColor` at src/render/preview_config.cpp:17 is now the one magenta.
- **How it left:** "fixed" in the triage; the docs were changed to say "confirmed as they are, D48, Q33". CONTEXT.md:311-312 says of the leaderboard fetch: "checks Cancel every 50 ms; and it accepts only HTTP status 200" and then "(src/net/dmbot_client.cpp; D48, Q33)." docs/adr/0008-preview-is-a-faithful-port-of-onyx.md:33 has a section headed "Hydra's own Preview numbers (D48, Q33)" that records 0.6 and 0.001.
- **User's words:** QF:175 "Renderer, audio and leaderboard tuning values (327, 336, 337, 339, 348, 349, R7.32, R7.34, R7.36):", followed by exactly eight sub-bullets (QF:176-183): star cutoffs as 32-bit floats; spans half a tick past their last note; the half-millisecond slack; the rules fingerprint format; the loading bar's shares and pacing; the audio reader limits; 16 parked lookups; the engine's progress step. QF:185 "Recommended: confirm all." D48 (DEC:137): "Every number in Q33 is kept as it is and recorded."
- **Code today:** the numbers are still there (src/render/overlay_layout.h:157 `kOverlayMinScale = 0.6f`; src/render/preview_config.cpp:17 `kBadColor`; src/audio/decode.cpp:27 `kChunkFrames = 4096`), by reading.
- **Verdict:** REOPEN (low weight, nothing the user sees changes)
- **Why:** None of the 327 literals is in Q33's sub-list. Seven of them (0.6 overlay floor, flat-line 0.001, magenta, 64-byte tag window, 50 ms poll, WinHTTP timeouts, status 200) were never shown to the user, yet CONTEXT.md and ADR 0008 cite "D48, Q33" as their ruling. The citation is by topic ("tuning values"), not by the question's words.
- **Examples:** The overlay boxes never shrink below 0.6 of full size, so on a very small preview image the text overlaps instead of shrinking further. A slow leaderboard host is given 15 s to resolve, 20 s to connect, 30 s to send, 120 s to receive. A leaderboard server that answers 204 or 301 is treated as a failure (status 200 only). The user was never told any of these.

### derivation 328: probe decision cut-offs

- **Splits:** None: dev-tool thresholds nobody chose (tools/ch_probe/constants.py:297 `HIT_TIME_MAX_GAP_MS = 10.0`, :300 `CLOCK_FRESH_S = 0.002`, :306 `CLAMP_TOLERANCE_MS = 1.0`, :309 `CLAMP_DECISIVE_FRACTION = 0.8`). They now live in one file, so the in-file repeat of SETTLE_MS is not a split.
- **How it left:** a question-less ruling in a plan list approved as a batch. `docs/superpowers/plans/2026-10-04-phase-6.md`, line 180 (removed; `git show 444631e6:docs/superpowers/plans/2026-10-04-phase-6.md`): "The developer-tool and pool numbers are recorded as they are, D49-style: the probe cut-offs in 328 (10 ms hit-time gap, 2 ms fresh clock, 50 ms fill cap, 1.0 ms tolerance, 80 % majority, 0.001 window floor, 0.000001 clock step)".
- **User's words:** DEC:211 D54: "Approved with D53: the "Recorded as recommended" list in `docs/superpowers/plans/2026-10-04-phase-6.md` (removed; `git show 444631e6:docs/superpowers/plans/2026-10-04-phase-6.md`), as written." DEC:205 D53: the user answered "use recommended answers to everything". DEC:219 repeats "the probe cut-offs in 328".
- **Code today:** the four named thresholds above are in constants.py with those values, by reading. I did not check the 50 ms fill cap or the 0.001 / 0.000001 pair.
- **Verdict:** PASS
- **Why:** The plan line the user approved names each number with its value, and the code still holds them. It changes only a developer probe's console verdicts.

### derivation 329: Preview beat grid end

- **Splits:** None left. The original question was "how far past the last note do beat lines run", with the transport running longer. Today: src/app/preview_view.cpp:361-373 ends the grid at the song's length (`build_beat_events(timing, timing.display_tick_at_ms(scene.song_length_ms))`), and only a song with no length falls back to `last->tick + 2 * mi.length_at(...)` at :372.
- **How it left:** a question, then superseded by a later ruling. QF:120-122 (Q25 "Where the song ends. (9, 329)"): "The beat lines stop two measures after the last note." Recommended: "... and the beat lines run to the end of the audio." Answered by D48 (DEC:134).
- **User's words:** DEC:1030 D266 call 3, made in chat 2026-10-29: "The Preview's beat lines stop at the song's end. That end is `song_length_ms`, the one end D255 call 5 made every part of the Preview use. This extends D255 and replaces D48 Q25's "the beat lines run to the end of the audio"."
- **Code today:** as above, by reading. The literal two measures remains only as the fallback when there is no song length.
- **Verdict:** GONE
- **Why:** The two-measure rule no longer applies to any song with a length, and the user has since ruled on the beat-line end by name (D266 call 3). Code change is in the D266 build (beat grid ends at song length).
- **Side picked without a ruling:** Not a split. Note only: Q25 recommended "audio end" and the code now uses the chart's end; the user's D266 call 3 chose that explicitly.

### derivation 330: over-limit path that ties the optimal score

- **Splits:** None: one rule. An over-limit path survives only if it ties the optimal score (src/search/engine.cpp:1219-1221 `can_outscore`: `!filtered_[idx] || (has_optimal_ && score == optimal_score_)`). The all-0 list ignoring the Path limit is a separate, ruled behaviour (D85).
- **How it left:** a question, Q7:18-20.
- **User's words:** Q7:18 "**2. A path that ties the best score needs a squeeze over your Path limit. Is it shown?** (330)"; Q7:20 "Recommended: keep showing it, and the guide gains one sentence. The best score is what you asked for. The orange figure already warns that it's hard." Answered D51 (DEC:162): "A path that ties the best score stays shown even when it is over the Path limit, and the guide says so (Q2, 330)."
- **Code today:** matches. docs/UserGuide.md:160: "A path that ties the best score is kept even when its timing is over the limit, and its figure shows in orange; below the best score, a path over the limit is dropped." Engine rule at src/search/engine.cpp:1216-1221, by reading.
- **Verdict:** PASS
- **Why:** The question showed the real rule with a worked example, the answer is in D51, and the guide and engine now say the same.

### derivation 331: .chart marker with no note under it

- **Splits:** None. The two homes (marker with no note; duplicate note) now answer the same way: both raise `ChartFileError` and the .chart parser's per-op catch skips the op (src/core/model.cpp:287-304). The .mid path also skips such an op (audit side note in docs/audit/2026-10-11-concept-audit/findings/V7.md:34).
- **How it left:** a ruling in chat, recorded as D27. DEC:65: "a `.chart` modifier with no note is skipped, as Clone Hero does at 0x215DDB0 (331)".
- **User's words:** DEC:65 D27: "Approved as recommended: ..." (the user's exact chat words are not quoted in the log). Searched: DEC, the step-2 plan header (`docs/superpowers/plans/2026-10-03-step2-parser-rules.md`, line 39; removed, `git show 444631e6:docs/superpowers/plans/2026-10-03-step2-parser-rules.md`), ADR 0023 line 40, the phase question files; not the session cards in full.
- **Code today:** marker with no note is dropped with the rest of the chart intact, src/core/model.cpp:291-304, by reading.
- **Verdict:** PASS
- **Why:** The ruling names the behaviour (skip, not reject) and its Clone Hero source. Nothing sits on the other side to disagree.

### derivation 332: unknown transfer ratios stored as x1.00

- **Splits:** None left. Stored "unknown" (`std::optional<TransferScale> transfer_post`, src/core/model.h:644) vs a silent x1.00: the engine now leaves the field unset (src/search/engine.cpp:2051-2062), and the details view shows "Transfer scale unknown." (src/app/path_view.cpp:253-256).
- **How it left:** a ruling in chat, D4.
- **User's words:** DEC:13 D4: 'The record may hold an explicit "unknown" transfer scale instead of a silent x1.00, ... But your condition is that this must never happen: "there should never be a situation when the scale cant be computed".'
- **Code today:** as above, by reading. Proof that no fresh record stores unknown is a test the log says exists; I did not run it.
- **Verdict:** PASS
- **Why:** The user's words name both sides (unknown vs x1.00) and give the condition. The code matches.

### derivation 333: timing tier ladder

- **Splits:** None: one table, `timing_tiers` (src/core/squeeze_rating.cpp:175-186): Normal up to kDifficultMs 2.0, Hard W/2, Extreme W, Insane 3W/2, Insane+ `nominal_budget_ms(w)` = W x (1+1) = 2W, then Beyond.
- **How it left:** a question batch. QF:171.
- **User's words:** QF:168 "**33. Confirm today's numbers so they get recorded.** Nothing changes on screen if you say yes." QF:171 "The report's tier ladder: 2 ms, half a hit window, one window, one and a half, two, then Beyond (333)." QF:185 "Recommended: confirm all." D48 (DEC:137): "Every number in Q33 is kept as it is and recorded."
- **Code today:** matches the question's words. The edge rule changed by a separate ruling: 2.0 ms is Normal (D48 Q3, src/core/model.h:92-95 `past_difficult_floor`).
- **Verdict:** PASS
- **Why:** The bullet lists every step of the ladder with its value, and the code has exactly those.

### derivation 334: two-bar activation minimum

- **Splits:** None: four bare 2s became `kSpActivationBars = 2` (src/core/timing.h:51); engine and graph read it (src/search/engine.cpp:818, 946-947, 965; src/search/graph.cpp:386). I found no bare 2 left for this question.
- **How it left:** a plan list approved as a batch, D54.
- **User's words:** `docs/superpowers/plans/2026-10-04-phase-6.md`, line 181 (removed; `git show 444631e6:docs/superpowers/plans/2026-10-04-phase-6.md`): "`kSpActivationBars = 2` (half a meter to activate, a Clone Hero rule no doc states; finding 334)". DEC:211 D54: "Approved with D53: the "Recorded as recommended" list ... as written." DEC:220 repeats it. CONTEXT.md:472 now records it.
- **Code today:** as above, by reading.
- **Verdict:** PASS
- **Why:** The approved list names the number and what it means, and the code has one owner. I did not check Clone Hero itself for the rule.

### derivation 335: cymbal, solo and multiplier-step values

- **Splits:** None left. The second copy (a hand-typed combo list in `MultSqueeze::applies`) now asks `crosses_multiplier_step(combo, chord.count())` (src/core/model.cpp:373-374). Values: src/core/model.h:148-151 (50, 15, 100), src/core/timing.cpp:11-14 (steps at 10, 20, 30).
- **How it left:** a question batch. Q7:72-80.
- **User's words:** Q7:74 "The scoring values Hydra took from Clone Hero: a cymbal is worth +15, a solo pays 100 per note, and the multiplier steps up at combos 10, 20 and 30." Q7:80 "Recommended: record all five as they are." (the list has six bullets). DEC:179 D51: "The scoring values: cymbal +15, solo 100 per note, multiplier steps at combos 10, 20 and 30 (335)."
- **Code today:** as above, by reading.
- **Verdict:** PASS
- **Why:** The question named all three values and the code matches. The "five" for six bullets is a slip in the question; D51 lists all six.

### derivation 336: star cutoffs as 32-bit float

- **Splits:** None: one formula (src/core/stars.cpp:10-15: product as `float`, then `std::ceil`).
- **How it left:** a question batch. QF:176.
- **User's words:** QF:175-176 "Renderer, audio and leaderboard tuning values (327, 336, ...): - star cutoffs multiplied as 32-bit floats, as the game does;" QF:185 "Recommended: confirm all."
- **Code today:** matches, by reading.
- **Verdict:** PASS
- **Why:** The bullet states the rule the code uses.

### derivation 337: Preview spans end half a tick after the last note

- **Splits:** None: `kSpanEndTicks = 0.5` (src/render/track_state.h:40), used at src/render/track_state.cpp:132. I did not check whether the scoring side of "is this note inside a phrase" differs from the drawn edge for a note exactly on the end.
- **How it left:** a question batch. QF:177.
- **User's words:** QF:177 "Preview spans ending half a tick past their last note;" under QF:175, Recommended QF:185 "confirm all."
- **Code today:** matches, by reading.
- **Verdict:** PASS
- **Why:** The bullet states the value and the code has it.

### derivation 339: Preview's own thresholds and fallbacks

- **Splits:** None. Half-ms slack and half-tick span end are 348 and 337. The 4/4 fallback was a code copy of the parser default; D27 gave it one owner (DEC:65 "one owner for the default 4/4 (258, 319)").
- **How it left:** a question batch, in part. QF:177-178 name the span end and the slack. The BPM 0.0 fallback (src/app/preview_view.cpp:556 `double bpm = 0.0;`) is not named in any question.
- **User's words:** QF:177 "Preview spans ending half a tick past their last note;" QF:178 "the half-millisecond "on an activation" slack;"; DEC:65 for 4/4.
- **Code today:** the fallback still exists at src/app/preview_view.cpp:556-563 and cannot run for a loaded song (the song always has a tick-0 tempo), per the audit; I only saw the fallback line, I did not prove the unreachability.
- **Verdict:** PASS (caveat)
- **Why:** The user's words name the visible parts. The BPM 0.0 fallback was never shown, but it is an unreachable default with no on-screen effect. The main session may prefer to fold it into the 327 question.

### derivation 343: stored tempo map written once

- **Splits:** None left: the item is gone.
- **How it left:** a question. Q7:67-70, answered D51 call 12.
- **User's words:** Q7:67 "**12. The stored tempo map and the scan cache follow reader changes.** (343, 345)"; Q7:69 "After: each analysis rewrites the tempo map."; Q7:70 "Recommended: yes to both." DEC:172 "Each analysis rewrites the stored tempo map, and the scan cache carries its own version stamp (Q12, 343 and 345)."
- **Code today:** no tempo map is stored. D87 (DEC:389) dropped "tempo maps, song names and lengths" from hydra.db; a click re-runs the engine. Commit 5a5c8faa ("WIP storage-T5: the store keeps summary rows only"), reached main via the D87 merges. `grep tempo_map|upsert_song` over src/store and src/app/analysis.cpp finds nothing, by reading.
- **Verdict:** GONE
- **Why:** There is nothing stored to go stale. D87 removed the stored map after the user's D51 answer.
- **Side picked without a ruling:** Not a split. The user's D87 ruling removed the storage.

### derivation 344: results stamp rule leaves out the parser

- **Splits:** None left. The two stamps listed different code; now both name the readers. src/store/stored_versions.h:43-47: "the chart readers (src/parse) when a chart reads differently".
- **How it left:** a ruling in chat, D23.
- **User's words:** DEC:57 D23: "The written `kResultsStamp` rule names `src/parse`: a parser change that alters chart output forces re-analysis." The user's own chat words are not in the log; searched DEC, the plans, the phase question files.
- **Code today:** as above, by reading. The triage says it was already fixed by D23.
- **Verdict:** PASS
- **Why:** D23 says what the rule now includes, and the comment says it. Both sides (engine/core only vs including parse) are named by the ruling's phrase "names src/parse".

### derivation 345: rescan cache has no reader stamp

- **Splits:** None left. The rescan cache now has its own stamp: `kChartMetaStamp{6, {6}}` (src/store/stored_versions.h:98), stored as the meta row `chart_meta_version` (src/store/record_store.cpp:1045, :1502, :1526), and the stored comment says a stale stamp "drops the whole rescan cache, so the next scan reads every chart once (D51 call 12)".
- **How it left:** a question. Q7:67-70, same as 343.
- **User's words:** Q7:68 "The rescan cache also reuses chart names and hashes forever."; Q7:69 "The scan cache gets its own version stamp, so a reader change refreshes every chart on the next scan."; Q7:70 "Recommended: yes to both."
- **Code today:** as above, by reading.
- **Verdict:** PASS
- **Why:** The question described the real rule (reuse until the file changes) and its fix, and the code does the fix.

### derivation 348: half-ms slack for "on an activation"

- **Splits:** None: one constant, `kOnActivationMs = 0.5` (src/app/preview_view.cpp:744), read at :751, :755, :767.
- **How it left:** a question batch. QF:178.
- **User's words:** QF:178 "the half-millisecond "on an activation" slack;" QF:185 "Recommended: confirm all." ADR 0008:42 records it.
- **Code today:** matches, by reading.
- **Verdict:** PASS
- **Why:** The bullet states the value and the code has it.

### derivation 349: rules fingerprint encoding

- **Splits:** None: one owner, src/core/rules.cpp (`%.17g` at :12, `fnv1a64` at :16, 0 mapped to 1 near :50).
- **How it left:** a question batch. QF:179.
- **User's words:** QF:179 "the rules fingerprint format;" QF:185 "Recommended: confirm all." The detail (FNV-1a 64, 17 digits, 0 mapped to 1) was in the triage `decision` text but not in the question. ADR 0014 now spells it out (docs/adr/0014*.md:37-40).
- **Code today:** matches the ADR, by reading.
- **Verdict:** PASS (caveat)
- **Why:** The user is never shown this and changing it would only mark every row Stale; the bullet names the topic without the encoding. If the main session wants every value shown, move it into the 327 question.

### derivation R7.31: Path limit on at 10 ms, backend display limit 50 ms

- **Splits:** None: a default nobody chose. src/app/config.h:153-154 `mslimit_enabled = true`, `mslimit_value = 10`; :160-161 `backendlimit_enabled = false`, `backendlimit_value = 50`. hydra_replay takes the same Settings (tools/replay.cpp:179-180). (The 3 ms vs 10 ms backend-band split is M39, not this item.)
- **How it left:** a question batch. QF:169.
- **User's words:** QF:169 "The path limit starts on at 10 ms, and "Hide backend rows beyond" starts off with 50 ms in the box (R7.31). Changing these would change which paths new analyses keep, so they're listed first." QF:185 "Recommended: confirm all." DEC:137: "That includes the 10 ms Path limit on, 50 ms backend hide off".
- **Code today:** matches, by reading. docs/UserGuide.md:136 shows the default via a docs marker.
- **Verdict:** PASS
- **Why:** The question named both defaults with their on/off state and warned that it changes stored paths. The code has exactly that.

### derivation R7.32: Preview loading bar shares and pacing

- **Splits:** None: numbers nobody chose. src/ui/preview_load_job.cpp:31-33 `kReadShare = 0.08f`, `kOpenShare = 0.82f`, `kSceneShare = 0.045f`.
- **How it left:** a question batch. QF:180.
- **User's words:** QF:180 "the loading bar's shares and pacing;" QF:185 "Recommended: confirm all." ADR 0008:59 records "8%, ... 82%".
- **Code today:** the shares match ADR 0008, by reading. I did not check the one-second pacing or the 59.5 s switch.
- **Verdict:** PASS (caveat)
- **Why:** The bullet names the topic, not the numbers. The user could not know the 8 / 82 / 4.5 split or the 59.5 s minutes switch. Low weight, and no split.

### derivation R7.33: short UI timings

- **Splits:** None. "Done!" 0.5 s is gone: DEC:389 D87 items 6 and 10 removed the "Done!" box (src/ui/details_panel.cpp:182 "there's no "Done!" (D87 items 6 and 10)"). "Copied!" 2.0 s (src/ui/app_state.h:420 `kCopiedSeconds`), search refilter 0.15 s (:423 `kSearchThrottleSeconds`) and the batch refresh once a second remain; CONTEXT.md:301-305 records them "confirmed as they are, D48, Q33".
- **How it left:** a question batch. QF:172.
- **User's words:** QF:172 "UI timings: 0.5 s "Done!", 2.0 s "Copied!", 0.15 s search refilter, 1.0 s batch refresh (R7.33)." QF:185 "Recommended: confirm all."
- **Code today:** as above, by reading. I did not look at the 1.0 s batch refresh line.
- **Verdict:** PASS
- **Why:** The question named every value, and the one that changed was removed by a later user ruling (D87).

### derivation R7.34: audio reader numbers

- **Splits:** None: numbers nobody chose (120 ms zero-byte Opus packet, 512 packet queue, 0.5 s MP3 seek points, 1 s hop, 4096-frame blocks, 4 GiB MP3 and 2 GiB Vorbis limits). Still in code: src/audio/opus_reader.cpp:55 `frames_of_ms(120.0, kRate)`, :403 `std::array<ogg_packet, 512>`; src/audio/stream_mix.h:90 `kBlockFrames = 4096`. The 4096 is spelled again as `kChunkFrames` in decode.cpp:27 and vorbis_reader.cpp:28 and `kSkipChunk` in ma_reader.cpp:166, which is a code copy of one number, not a second rule.
- **How it left:** a question batch. QF:181.
- **User's words:** QF:181 "the audio reader limits;" QF:185 "Recommended: confirm all." ADR 0019 line 97: "The reader numbers, confirmed as they are (D48, Q33)."
- **Code today:** matches ADR 0019 (docs/adr/0019*.md:98-104), by reading.
- **Verdict:** REOPEN (low weight)
- **Why:** "the audio reader limits" does not tell the user a single value, yet ADR 0019 cites it as their confirmation of each. Nothing on screen changes either way.
- **Examples:** A zero-byte Opus packet counts as 120 ms of audio (the concealment length). An Ogg page with more than 512 packets stops reading. An MP3 over 4 GiB gets no seek points, so every seek decodes from the start. The user was shown none of these.

### derivation R7.36: three tuning values

- **Splits:** None. `kParkedLookups = 16` no longer exists: commit a82f74ee ("WIP storage-T2: a click analyzes the song on a worker and saves its summary (D87, D90)") removed it with the stored details. The other two remain: engine progress step 0.005 (src/search/engine.cpp:1822 `progress_reported_ + 0.005f`) and `kMainProgressShare = 0.9f` (src/search/pather.cpp:36).
- **How it left:** a question batch. QF:182-183.
- **User's words:** QF:182 "16 parked lookups;" QF:183 "the engine's progress step." QF:185 "Recommended: confirm all."
- **Code today:** as above, by reading and `git log -S kParkedLookups`.
- **Verdict:** REOPEN (low weight)
- **Why:** The all-0 pass's 0.9 share of the bar was never named in any question, and the progress step was named without its value. The parked-lookups bullet is moot (GONE for that part).
- **Examples:** The bar reaches 90 percent when the main search ends and the all-0 pass fills the last tenth. A chart with a long all-0 pass would stall near 90 percent. Only the bar's pacing changes; no score or path.

<!-- next -->

## Not reached

None. All 22 ids have a block.

## Side notes

- QF:168 says Q33 numbers get "one sentence in CONTEXT.md or its ADR", and later docs cite "D48, Q33" as the ruling for items Q33's text never lists. The sources are CONTEXT.md:311-312 (leaderboard timeouts, status 200, 50 ms cancel), docs/adr/0008-preview-is-a-faithful-port-of-onyx.md:33 (0.6 overlay floor, 0.001 flat-line epsilon, magenta) and docs/adr/0019-preview-audio-streams-from-the-compressed-file.md:97 (reader numbers). A scan for "D48, Q33" in docs would list every citation that rides on the heading rather than on a named value.
- Q7:80 says "record all five as they are" over a six-item list (Q7:74-79). D51 (DEC:173-179) lists all six, so no effect, but a count mismatch in a question is the kind that lets one item slide.
- 329's original Q25 (QF:120-122) bundled two questions (position against last note; beat lines to audio end). Both were later replaced by D255 and D266 call 3, so a closure check of Q25's other half (finding 9) belongs to another scout.
- In 327 the triage `status_evidence` says "Literals recorded (CONTEXT.md, ADR 0008, 0019)". The recording happened (phase 4 merge ae60cc5); the ask did not.
