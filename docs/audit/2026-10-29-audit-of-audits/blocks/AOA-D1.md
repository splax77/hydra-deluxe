# AOA-D1: derivation audit 2026-10-03, drift and double-reading findings with a decision (scout a95952fa95d1bd27e)

Paths below are relative to the checkout `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\fervent-bohr-838562` (main 1fc55723). "Decisions file" is `docs/audit/2026-10-03-fix-decisions.md`.

### Derivation audit 1: score box keeps doubled disc after SP ends, drain box says idle

- **Splits:** "Is Star Power running at the playhead just after the SP end?": score box held the last chord's doubled multiplier until the next chord (old code, finding text `docs/audit/2026-10-03-derivation-audit.md`:50-63) vs drain box idle at `a.ms <= now < sp_end_ms`. Today both sides ask one helper: score box `src/app/preview_view.cpp`:599-600 and drain box `src/app/preview_view.cpp`:636 both call `running_activation` (`src/app/preview_view.cpp`:273).
- **How it left:** fixed, then a second half recorded as "a known gap". Triage: "the box drops to the plain multiplier when SP ends (D10/D11). Fixed by 4983e1f." (`docs/audit/2026-10-03-fix-triage.json`, row 1 `status_evidence`). Decisions file D53 item 4: "The Preview's xN disc at the Star Power end (finding 1's second half) is left as it is and recorded as a known gap" (`docs/audit/2026-10-03-fix-decisions.md`:209).
- **User's words:** step-1 plan question: "Q8. When SP ends, should the Preview score box drop to the plain multiplier right away, instead of holding the doubled value until the next chord? Recommended: yes, after checking one FC video that the game's disc drops too. It would then agree with the drain box and highway." (`docs/superpowers/plans/2026-10-03-step1-engine-stores-what-displays-guess.md`, line 80; removed, `git show 444631e6:docs/superpowers/plans/2026-10-03-step1-engine-stores-what-displays-guess.md`), answered as D10: "All answered as the plan recommends. Q8 was confirmed first on FC video: in four activations across Divine Inner Tension and Desecration Day, the game's disc drops from x8 to x4 within two frames of Hydra's SP end and stays plain until the next chord." (decisions file:25). D11: "The Preview score box shows the plain multiplier from the SP end on." (decisions file:27).
- **Code today:** `build_score_box` shows `at.multiplier` only while `running_activation(scene, now) != nullptr`, else `at.multiplier_plain` (`src/app/preview_view.cpp`:597-600), by reading. Commit 4983e1f1 "Score box drops the doubled disc when SP ends" is the change.
- **Verdict:** GONE
- **Why:** The two boxes now ask one function, so at D+10 ms both say "not running". The question text (Q8) named both behaviours (drop right away vs hold until next chord) and the drain box.
- **Side picked without a ruling:** none; Q8/D10/D11 name the drop-at-SP-end side. Record conflict to note: D53 item 4 (decisions file:209, from `docs/audit/2026-10-04-phase-6-questions.md`:35-46) says the doubled disc is left as is and the game's disc "has never been checked against video". That contradicts D10 (video checked) and the code. D53 item 4 is stale; the user answered "recommended" to a question whose premise was already false.

### Derivation audit 2: report footer says tiers match squeeze ratings (D50 item 1 pair)

- **Splits:** "How hard is one SqOut, in words?": report tier ladder `timing_tiers` (`src/core/squeeze_rating.cpp`:175-185: Normal up to `kDifficultMs` 2.0, Hard up to w/2, Extreme up to w, Insane up to 3w/2, Insane+ up to the nominal budget, then Beyond) vs Paths-row label ladder `BackendSqueeze::summarystr` (`src/core/model.cpp`:348-352 for a squeezed-out row: Insane SqOut below -w, Hard SqOut below -10, Standard SqOut up to +10, Easy SqOut below w, else Free SqOut). Same words (Hard, Insane), different edges and a different measure. Example: a 5 ms SqOut is "Hard" in the report and "Standard SqOut" on the row.
- **How it left:** question answered "all recommended", then a D50 item. Triage: "the report footer reads D50's sentence instead of claiming the tiers match squeeze ratings".
- **User's words:** Q5 in phase 3-5 file: "**5. The report footer says its tiers match the Paths rows.** (2)" / "Today a 5 ms SqOut reads "Hard" in the report and "Standard SqOut" on its Paths row, under a footer saying the two match." / "Recommended: reword the footer. The row label measures distance from the SP end, both early and late; the report tier measures size. They answer different questions." (`docs/audit/2026-10-04-phase-3-5-questions.md`:35-38), answered as D48: "The report footer is reworded (Q5)." (decisions file:123) and D50 item 1: "The path report's footer (finding 2) reads: "Timing tiers measure how big each squeeze is, in steps of your hit window. The Paths tab's row labels measure how far a hit lands from the Star Power end, so the two can differ. 'Beyond' means past the N ms window."" (decisions file:148; user "answered "all recommended"" at :147).
- **Code today:** `src/app/report.cpp`:339-344 prints exactly that footer text; both ladders still exist as above, by reading.
- **Verdict:** PASS
- **Why:** The question and D50 both name the two sides (report tier, Paths row label), give the two real words for one real value (5 ms: Hard vs Standard SqOut) and say what each measures (size vs distance from the SP end). The user kept two ladders knowingly. Weak spot, not enough to reopen: the numeric edges (2 ms / w/2 for tiers, 10 ms / w for rows) were never shown side by side; the 3 ms vs 10 ms split inside the rows is a different pair (M39) and is not covered by this ruling.

### Derivation audit 3: timeline outlines any badged activation orange, row badge grey

- **Splits:** "When is an activation shown in warning orange?": timeline outline drew orange for any badge (old, finding text) vs row badge orange only when `a.difficult`. Today both read `a.difficult`: outline `src/ui/paths_tab.cpp`:231-238 (`warn_or_dim(a.difficult)`, grey otherwise).
- **How it left:** fixed. Triage: "the timeline outline is orange only when the row is difficult (M_D 57dcf23)."
- **User's words:** "**8. Orange outline on the Paths timeline.** (3)" / "Today the timeline outlines any badged activation in orange, even a 1 ms squeeze-in or an early fill whose row badge is grey." / "Recommended: orange only when the row itself is orange (difficult)." (`docs/audit/2026-10-04-phase-3-5-questions.md`:48-51), answered as D48: "The timeline is orange only for difficult rows (Q8)." (decisions file:126); D50 item 7: "On the Paths timeline (finding 3), a difficult activation keeps its orange outline; any other badged activation gets a grey outline matching its grey badge." (decisions file:154).
- **Code today:** `src/ui/paths_tab.cpp`:231-238, by reading.
- **Verdict:** GONE
- **Why:** The code now has one colour rule for both. The question named both sides with the 1 ms SqIn and early-fill examples.
- **Side picked without a ruling:** none; Q8 and D50 item 7 name the side picked.

### Derivation audit 4: one squeeze reads 13 ms on the badge but 12 ms when copied

- **Splits:** "How is a squeeze written in whole ms?": badge `%.0f` (round half even) vs copied text `static_cast<long long>` (cut off). Today one formatter `format_ms_whole` (`src/core/model.cpp`:919-921, `std::lround`) feeds the badge (`src/app/path_view.cpp`:74, 92), the details (`src/core/model.cpp`:556-557) and the backend-limit text (`src/app/path_view.cpp`:396).
- **How it left:** fixed. Triage: "whole-ms text rounds to nearest in the badge and the copied path (M_D 57dcf23)."
- **User's words:** "**2. How does a squeeze timing print?** (4, 18, 36, 154)" / "Today one squeeze can read "13 ms" on the activation badge, "12 ms" when you copy the path, and "12.6 ms" on the path button. ..." / "After: whole-ms text rounds to nearest everywhere, so the badge and the copied text agree. One-decimal text is unchanged. ..." / "Recommended: yes. ..." (`docs/audit/2026-10-04-phase-3-5-questions.md`:20-24), answered as D48: "Whole-ms text rounds to nearest everywhere (Q2)." (decisions file:120).
- **Code today:** by reading, as above. `std::lround` rounds an exact half away from zero (162.5 gives 163), which matches "rounds to nearest".
- **Verdict:** GONE
- **Why:** Cut-off and round are now one function. The question showed all three values and said which wins.
- **Side picked without a ruling:** none.

### Derivation audit 7: "Optimal" means one path in the report but every tied path in Paths and Preview

- **Splits:** "Which paths are optimal?": report rank 1 only vs Paths/Preview every path tied at the top score. Today both read `HydraRecord::is_optimal` (`src/core/model.cpp`:886-889: `path.totalscore() == *optimal`); the report's "Best path only" filter is `if (best_only && !r.optimal) return false;` (`src/app/path_report_view.cpp`:158), rows get `row.optimal = p.optimal` (`src/app/report.cpp`:49; set at `src/store/record_store.cpp`:607); Paths uses `record.is_optimal(*p)` (`src/app/path_view.cpp`:538). `CONTEXT.md`:445-452 now says a tied variant is optimal too.
- **How it left:** fixed. Triage: "HydraRecord::is_optimal makes every tied top-score path optimal in the report, Paths tab and Preview (M_D 57dcf23)."
- **User's words:** "**1. Two paths tie for the top score: are both "optimal"?** (finding 7, weighty)" / "Today the Paths tab and the Preview call both of them Optimal (gold). The report's "Best path only" view keeps only the first one and bolds only it." / "After: one answer everywhere." / "Recommended: both are optimal. A drummer gets the same score playing either, so the report lists the tied path too." (`docs/audit/2026-10-04-phase-3-5-questions.md`:15-19), answered as D48: "Tied top-score paths are all optimal, in the report too (Q1)." (decisions file:119).
- **Code today:** by reading, as above.
- **Verdict:** GONE
- **Why:** One rule on the record; the question named both sides.
- **Side picked without a ruling:** none.

### Derivation audit 9: Paths timeline and Preview scrubber use different song lengths

- **Splits:** "Where does the song end?": Paths timeline used the last note, Preview used later of last note and audio end. Today there is one length: `app::song_length_ms` feeds the Paths fraction (`src/app/path_view.cpp`:205-226, 409-410) and the Preview (`src/app/preview_view.h`:310-314, 540-542: "The audio's end is never an input").
- **How it left:** fixed by a one-helper change in phase 3, then the rule was changed by D255. Triage: "one song-fraction helper and one song length, so activations sit at the same fraction on the scrubber and the Paths timeline".
- **User's words:** phase 3-5 Q25: "Recommended: both bars measure position against the last note, so an activation lines up in both. The scrubber still lets you play the audio tail, and the beat lines run to the end of the audio." (`docs/audit/2026-10-04-phase-3-5-questions.md`:120-123), D50 item 4: "The Preview scrubber's right edge is the last note (finding 9), so activations sit at the same fraction as on the Paths timeline." (decisions file:151). Later replaced by the user's own words in D255 item 5: "the problem is that we have two different rules. there should one rule that every function uses. we already know how clone hero decides the end of the song, we don't need different places deriving different rules." and the choice "Ignore it (Recommended)" for the audio length (decisions file:909); D266 call 3 extends it to the beat lines (decisions file:1030).
- **Code today:** by reading, as above.
- **Verdict:** GONE
- **Why:** The two lengths are one. The user ruled it twice, the second time in his own words naming "two different rules".
- **Side picked without a ruling:** none.

### Derivation audit 11: disco flip at Hard and below follows Expert's markers

- **Splits:** "Is a disco-flip section active at the difficulty being parsed?": regex hard-coded `mix 3` (Expert) vs the parser's own difficulty. Today `is_disco_on_marker(s, mix_digit)` and `is_disco_off_marker` take the difficulty's digit (`src/parse/song.cpp`:306-318), used at :977-978 (.mid) and :1256-1258 (.chart).
- **How it left:** a question asked in chat (not in the question files), then fixed. Triage: "D19: is_disco_on_marker/off_marker take the difficulty's mix digit ... Fixed by 1285c45."
- **User's words:** session card: "D19: chose "Per difficulty (Recommended)" to "Disco flip: should each difficulty read only its own [mix N] markers ..." (01:50 UTC, each difficulty reads only its own marker)" (`docs/sessions/2026-10-04-bbf41dc4.md`:18). Record: "Disco flip is per difficulty: each difficulty reads only its own `[mix N drums...]` markers, in both formats, as Clone Hero does ... Effect: Expert never changes; Hard, Medium and Easy Pro Drums scores change on about 640 library charts each, mostly up" (decisions file:49).
- **Code today:** by reading, as above.
- **Verdict:** GONE
- **Why:** The parser has one difficulty choice. The user chose the per-difficulty side after seeing the score effect.
- **Side picked without a ruling:** none.

### Derivation audit 12: Dynamics tab counts 2x kicks in one kick total and drops them in another

- **Splits:** "Which kick rows count in a kick total when 2x Bass is off?": "All kicks" row (old: always added 2x) vs Totals box (old: added 2x only with 2x Bass on). Today both read `DynamicsBreakdown::kicks_total(bass2x)` (`src/app/dynamics_breakdown.cpp`:25-42, `src/ui/dynamics_tab.cpp`:191, 248). Remaining visible difference, ruled: the "2x kicks: X of Y kick notes" line counts every charted kick (`src/app/dynamics_breakdown.cpp`:112-119, `kicks_total(/*bass2x=*/true)`), so with 2x Bass off it can read "12 of 453" beside an All kicks row of 441.
- **How it left:** fixed through D20 (asked in chat) and D31 (plan Q2). Triage: "DynamicsBreakdown::kicks_total(bass2x) feeds both kick totals on the Dynamics tab. Fixed by a3a3157."
- **User's words:** D20: "...The Dynamics tab stops reading Expert's 2x kicks below Expert, so the phantom rows and swallowed kicks go away, and one kick total feeds both "All kicks" and Totals." (decisions file:51); card line: "D20: chose "A: own pitch, box everywhere" to "2x kicks below Expert: which shape?"" (`docs/sessions/2026-10-04-bbf41dc4.md`:19). Plan Q2, approved as D31: "D20 says "one kick total feeds both All kicks and Totals" but doesn't name this line. *Approved (D31):* it keeps counting every charted kick, as today, because it describes the chart. If it followed the 2x Bass box, with the box off it would read "12 of 441 kick notes", where the 12 are not part of the 441." (`docs/superpowers/plans/2026-10-03-step2-parser-rules.md`, line 65; removed, `git show 444631e6:docs/superpowers/plans/2026-10-03-step2-parser-rules.md`); card: "D31: chose "Keep charted count (Recommended)" to "Should it keep counting every charted kick, as today, even when the 2x Bass box is off?"" (`docs/sessions/2026-10-04-bbf41dc4.md`:32). UserGuide now says "both All kicks and the totals leave the 2x kicks out" (`docs/UserGuide.md`:368).
- **Code today:** by reading, as above.
- **Verdict:** GONE
- **Why:** The two totals are one function. The one difference left on the tab (the "X of Y" line) was put to the user with both numbers and kept by his choice.
- **Side picked without a ruling:** none for the line; for "All kicks drops 2x with the box off" the picking text is D20's "one kick total" plus plan Q1/D31 wording of UserGuide line 168. It is named only in the plan text the user approved as D31 ("Approved as the plan proposes", decisions file:75).

### Derivation audit 15: note exactly at the playhead, score box says hit, highway says not yet

- **Splits:** "Is a note exactly at the playhead hit?": score box `<=` (hit) vs highway strict `<` (not hit). Today one predicate `struck_at(now, note) { return note_ms <= now_ms; }` (`src/app/preview_view.h`:387), used by the score box (`src/app/preview_view.cpp`:592) and the highway (`src/render/highway_draw.cpp`:247).
- **How it left:** question, answered "all recommended". Triage: "struck_at counts a chord exactly on the playhead as hit in the score box and the highway (M_D 57dcf23)."
- **User's words:** "**27. A note exactly on the playhead.** (15)" / "Today, after "< Act" or "Act >", the score box already counts the chord under the playhead while the highway shows it unlit." / "Recommended: it counts as hit in both, so a jump lands on a struck gem." (`docs/audit/2026-10-04-phase-3-5-questions.md`:128-131), answered as D48: "A chord on the playhead counts as hit" (decisions file:134, in "The Preview items Q22 to Q27 take theirs. Position is measured against the last note, the beat lines run to the audio end, teal means SP running, and a chord on the playhead counts as hit.").
- **Code today:** by reading, as above.
- **Verdict:** GONE
- **Why:** One predicate, and the question named both sides with the "< Act" jump case.
- **Side picked without a ruling:** none.

### Derivation audit 16: running Star Power is teal on the highway and gold in the drain box

- **Splits:** "Which colour means SP is running?": highway floor teal `sp_active_color` (`assets/preview/3d-config.json`:48) vs drain box text gold. Today the drain box text uses the same `cfg.hydra.sp_active_color` when active (`src/ui/preview_tab.cpp`:366-375). The gold `kStarPowerColor` (`src/ui/theme.h`:26) remains only on the gauge fill and the "SP" label (`src/ui/preview_tab.cpp`:797, 821) and the stars (:356).
- **How it left:** question, answered "all recommended". Triage: "the drain box text turns teal like the highway floor while Star Power runs (M_D 57dcf23)."
- **User's words:** "**26. Preview colours.** (16, 136, 218)" / "Today the highway floor is teal while Star Power runs, but the drain box text is gold at the same moment. The scrubber's activation marks use one gold (250,210,0) and the next-activation header another (255,204,51)." / "Recommended: teal means "SP running" everywhere, so the drain box text turns teal like the floor (the Onyx look, ADR 0008). Activation marks and the header both use the best-path gold." (`docs/audit/2026-10-04-phase-3-5-questions.md`:124-127), answered as D48 (decisions file:134: "teal means SP running").
- **Code today:** by reading, as above. I did not check the next-activation header's gold (the second half of Q26, finding 136 or 218), only the finding-16 half.
- **Verdict:** GONE
- **Why:** Running-SP text and floor share one teal; the question named both colours.
- **Side picked without a ruling:** none.

### Derivation audit 17: same drum note has two names (Paths/Preview vs Dynamics tab)

- **Splits:** "What is a drum note called?": `ChordNote::str` ("GreenTom") vs `dynamics_row_label` ("Green tom"). Today `note_label(note, pro)` (`src/core/model.cpp`:60) feeds `Chord::rowstr` (:75-80) and the Dynamics row label (`src/app/dynamics_breakdown.cpp`:105).
- **How it left:** question, answered "all recommended". Triage: "one note_label gives the Dynamics words and the Pro Drums flag everywhere (M_D 57dcf23)."
- **User's words:** "**11. Drum note names.** (17)" / "Today a green tom is "GreenTom" on the Paths tab and in the Preview box, but "Green tom" on the Dynamics tab. With Pro Drums off, the Paths tab says "YellowTom" where Dynamics says "Yellow"." / "Recommended: use the Dynamics wording everywhere ("Green tom", "Yellow cymbal", "2x kick", plain "Yellow" with Pro Drums off)." (`docs/audit/2026-10-04-phase-3-5-questions.md`:60-63), answered as D48: "Note names follow the Dynamics wording (Q11)." (decisions file:129).
- **Code today:** by reading, as above.
- **Verdict:** GONE
- **Why:** One naming function; the question named both vocabularies.
- **Side picked without a ruling:** none.

### Derivation audit 18: hardest squeeze shown as whole ms on the badge, one decimal elsewhere

- **Splits:** "How many decimals does a squeeze timing get?": badge `format_ms_whole` (`src/core/model.cpp`:919-921, e.g. "163 ms") vs path button `format_ms` one decimal (`src/app/path_view.cpp`:523, "163.0 ms"). This split still exists.
- **How it left:** a question that was really about rounding; the precision gap rode on it. Triage: "the badge, copied path and report print whole ms through one owner (M_D 57dcf23)."
- **User's words:** Q2 shows all three values: "Today one squeeze can read "13 ms" on the activation badge, "12 ms" when you copy the path, and "12.6 ms" on the path button." / "After: whole-ms text rounds to nearest everywhere, so the badge and the copied text agree. One-decimal text is unchanged." (`docs/audit/2026-10-04-phase-3-5-questions.md`:20-24), D48: "Whole-ms text rounds to nearest everywhere (Q2)." (decisions file:120). Also the finding itself says the 2026-09-27 UI spec approved "squeeze out 163 ms" and "163.0 ms" (`docs/audit/2026-10-03-derivation-audit.md` finding 18 text).
- **Code today:** as above, by reading.
- **Verdict:** PASS
- **Why:** The user saw both formats side by side with values and approved whole ms for the badge and one decimal for the button ("One-decimal text is unchanged"). It is a precision difference, not a different rule, and the question described what the code does.

### Derivation audit 21: .chart and .mid parsers disagree on when a Star Power phrase ends

- **Splits:** "Has this SP phrase ended, which chord is last?": .mid closed the phrase itself at the 116 note-off vs .chart closed at the first later tick at or past the end (finding table: length 500 flags 1152 on .mid, nothing on .chart; length 0 flags nothing on .mid, 768 on .chart). Today both call `close_sp_phrase` (`src/parse/song.cpp`:401, called at :805 for .mid and :1414 for .chart).
- **How it left:** question asked in chat (not in the question files). Triage: "close_sp_phrase in song.cpp ends a phrase for both parsers with Clone Hero's rule (D21). Fixed by 9b14098."
- **User's words:** the question: "SP phrase edges: should .chart use Clone Hero's rule (start ≤ tick < end; zero-length pays nothing; past-the-end pays on the last note) through one helper both parsers share? 21 library charts lose 2,160–19,180 points." (the full question text was not kept; card `docs/sessions/2026-10-04-bbf41dc4.md`:21 keeps its start); answer: "D21: chose "Yes, one helper (Recommended)"" (card `docs/sessions/2026-10-04-bbf41dc4.md`:21); record D21: "One phrase-end helper for both parsers, with Clone Hero's rule (0x20D2440): start <= tick < end; a zero-length phrase pays nothing; a phrase running past the last note pays on that note. Effect: 20 Expert charts and 1 Hard chart lose 2,160 to 19,180 points that Clone Hero never pays" (decisions file:53).
- **Code today:** by reading, as above. Not run.
- **Verdict:** GONE
- **Why:** One helper for both formats. The question named the rule, both parsers and the score loss.
- **Side picked without a ruling:** none.

### Derivation audit 22: free squeezes still show a timing figure

- **Splits:** (a) "Does a path with only free squeezes show a figure?": button, badge, report showed "-163.0 ms" vs UserGuide "shows nothing there". Today the path button, filter, stored hardest ms and report use `Activation::difficulty()` which is empty when nothing needs timing (`src/core/model.cpp`:495-534), `SPSqueeze::needs_no_timing` (`src/core/model.h`:383-386). (b) "Is a squeeze at exactly 0.0 ms free?": `needs_no_timing` says a SqOut at 0.0 needs timing (SqIn at 0 is free), but the all-0 list passes it (`allzero_options` `ms_filter = 0.0`, `src/search/pather.cpp`:69-77; `timing_within_limit`, `src/core/model.h`:283). (c) Row badge still shows a free squeeze's figure ("squeeze in -316 ms") while the path button shows none (`Activation::badge_timing`, D80). All three still exist and each was ruled.
- **How it left:** question (phase 7 Q4), D51 call 4, then D80 and D85 corrected it. Triage: "phase 7 wave 2, merged to main 199fec4".
- **User's words:** Q4: "A free squeeze is a squeeze-in at exactly the SP end; it needs no timing. Today such a path still shows a figure like "-163.0 ms" on its button, its badge and the report's Hardest column. The guide says nothing is shown." / "After: no figure for a path with nothing to time. A squeeze-out at exactly 0.0 ms still counts as needing timing (D13 already decided that), and the all-0 list must follow D13 too." / "Recommended: yes. Before merging, I'll show you which charts' all-0 lists change." (`docs/audit/2026-10-04-phase-7-questions.md`:27-31), D51 call 4 (decisions file:164). D80: "D51's question had described a free squeeze as "a squeeze-in at exactly the SP end", but the code's rule (`SPSqueeze::is_free`) is "at or before the SP end, however early" ... The user chose "back to 2.0 exactly" for the row badge ... Everything else D51 call 4 decided stays: the path button, the Path limit, the library filter, the stored hardest ms, the report and the all-0 list still treat a free squeeze as nothing to time" (decisions file:359). D85, the user's words: "best all-0 (limit 0ms) is the intended behavior, and that's how it existed in 2.0 until you decided to remove it for no reason. hiding a path when it clearly exists was the mistake." ... "A squeeze-out exactly on the SP end and an early fill with exactly zero slack pass again." (decisions file:372).
- **Code today:** by reading, as above.
- **Verdict:** PASS
- **Why:** Q4 was narrower than the code's rule (D80 says so), but the user later ruled the badge-vs-button split (D80) and the 0 ms all-0 edge (D85) in words that name both sides. Two cautions: Q4 said "D13 already decided" the SqOut-at-0.0 case, yet D13 (decisions file:31) only covers a SqIn at offset 0; and D85 reversed Q4's all-0 instruction. Main session may want to confirm the user is content with SqOut at exactly 0.0 ms reading "needs timing" on the button but "passes" in the all-0 list.

### Derivation audit 23: squeeze filter and Path limit also count required early fills

- **Splits:** "Does 'hardest timing' include a required early fill?": code `Activation::difficulty` includes the E0 fill (`src/core/model.cpp`:529-532 via `hardest()`:503-521) vs words that said "hardest squeeze". Today the words say it: `src/ui/settings_bar.cpp`:177 ("every squeeze and required early fill"), `docs/UserGuide.md`:160, 210-213.
- **How it left:** question, answered "all recommended" (D51 call 5). Triage: "phase 7 wave 2, merged to main 199fec4".
- **User's words:** "**5. Does a required early fill count against the Path limit like a squeeze?** (23)" / "Today it does. A path that needs a fill hit 30 ms early is treated like a 30 ms squeeze by the Path limit and the squeeze<= filter. But the guide and the Path limit tooltip say "hardest squeeze"." / "Recommended: keep the rule, since an early fill is still a timing you must hit. Fix the words to "hardest squeeze or required early fill"." (`docs/audit/2026-10-04-phase-7-questions.md`:36-39); D51 call 5: "A required early fill counts against the Path limit like a squeeze, and the words say "hardest squeeze or required early fill" (Q5, 23)." (decisions file:165).
- **Code today:** by reading, as above.
- **Verdict:** PASS
- **Why:** The finding was code vs words. The question described what the code does and the words now match. No second behaviour remains.

### Derivation audit 28: cap-clamped windows measure the ratio from the activation, not the collecting note

- **Splits:** "Which note anchors the transfer scale on a cap-clamped window?": activation tick (old `frontend_transfer_scales`) vs the collecting note (engine's `clamp_tick`). Today `frontend_transfer_scales` uses `act.deact_anchor_tick()` and `squeeze_anchor_tick(k)` (`src/core/squeeze_rating.cpp`:25-40) which come from `Activation::end_anchor_tick` = latest clamp tick else the activation (`src/core/model.cpp`:753-780).
- **How it left:** question asked in chat, answered D1. Triage: "Fixed by 428a70f."
- **User's words:** card: "D1: chose "Yes, collecting note (Recommended)" to "Call 1 (finding 28): when the SP cap clamps a window, should the transfer scale ..." (20:58 UTC, D1, measure the scale from the collecting note)" (`docs/sessions/2026-10-03-cebf3006.md`:18). D1: "When the SP cap clamps a window, the transfer scale is measured from the collecting note (the stored `clamp_tick`), not the activation." (decisions file:7).
- **Code today:** by reading, as above. Commit 428a70fc.
- **Verdict:** GONE
- **Why:** The scale reads the collecting note; the ruling names both anchors.
- **Side picked without a ruling:** none.

### Derivation audit 30: the 500 ms squeeze window tested in four places, tail rows measured from the last note

- **Splits:** "Is a tail note inside the 500 ms window?": graph kept tail notes within 500 ms of the last chart note vs `display_backends` measures from the final SP end. Today the graph still collects a wider tail list from the head note (`src/search/graph.cpp`:316-326, `src/search/graph.h`:210-218) but the engine narrows it to the window around each activation's own SP end (`src/search/engine.cpp`:1990 `if (!within_squeeze_window(off)) continue;`) and the display asks the same `within_squeeze_window` (`src/core/model.h`:123-127, `src/core/model.cpp`:827). The Backend-limit box reads `kSqueezeWindowMs` (`src/app/config.cpp`:156).
- **How it left:** triage "phase 7 recheck (2026-10-04, D51): already fixed by D5; the 500 ms window has one owner." The triage `decision` field (tail row anchor) was answered in chat as D5.
- **User's words:** card: "D5: chose "From the SP end (Recommended)" to "Call 5 (R7.10): for SP that outlasts the song, measure the 500 ms backend window from where?"" (`docs/sessions/2026-10-03-cebf3006.md`:23). D5: "For an activation whose SP outlasts the song, the 500 ms backend window is measured from the SP end, the same as every other activation." (decisions file:15); D41 for the box: "The box reads `kSqueezeWindowMs`" (decisions file:95).
- **Code today:** by reading, as above. The Backend limit box and help text (the "typed 500" half) are covered by D41.
- **Verdict:** GONE
- **Why:** The anchor is the SP end for every activation, including tail ones. The ruling names both anchors.
- **Side picked without a ruling:** none.

### Derivation audit 32: replay re-decides which chords SP pays

- **Splits:** "Does SP pay this chord (drives the xN disc)?": replay's own gate (`in_sp = sp_claims > 0`, doubled a 0-SP squeezed-out chord) vs the shared pricing rule. Today the replay asks `core::paid_by_sp(...)` (`src/core/replay.cpp`:184-190, `src/core/backend_value.h`:65) and sets `row.in_sp = sp_paid > 0` (:197).
- **How it left:** two questions asked in chat, answered D2. Triage: "Fixed by 129f199."
- **User's words:** card: "D2: chose "Keep walk, ask shared rule (Recommended)" to "Call 2a (finding 32): what should happen to the replay's own ..."" and "D2: chose "No: x2 only if SP paid (Recommended)" to "Call 2b (finding 32): should the Preview disc show x2 on a squeezed-out chord ..."" (`docs/sessions/2026-10-03-cebf3006.md`:19-20). D2: "The Preview's xN disc doubles only when SP actually paid something for the chord. So a squeezed-out chord that SP pays nothing shows the plain multiplier, and a partly paid one still shows doubled." (decisions file:9).
- **Code today:** by reading, as above.
- **Verdict:** GONE
- **Why:** The replay no longer decides on its own. Both sides, the replay walk and the shared rule, are named by D2.
- **Side picked without a ruling:** none.

### Derivation audit 33: uncounted Star Power phrase rows show 0 points but no "(uncounted)" tag

- **Splits:** (a) "Is an uncounted backend row tagged?": plain row tagged vs squeezed-out row untagged. Fixed: `summarystr` adds the tag to both (`src/core/model.cpp`:347, 358-360). (b) Hidden in the finding's own table: "What does a note 5 ms after the SP end read?": plain row "Hard (uncounted)" (`src/core/model.cpp`:358-359, `counted_without_squeeze(5, 3)` false, `off < w`) vs squeezed-out row "Standard SqOut (uncounted)" (`src/core/model.cpp`:347-350, band `kBackendInnerBandMs` 10). The finding's table row "Phrase row at +5 ms | 0 | Standard SqOut" shows it. This is the M39 split. Q6's fix made it more visible: the squeezed-out row now says "Standard" and "(uncounted)" together.
- **How it left:** question for the tag only; fixed. Triage: "summarystr tags an uncounted squeezed-out row "(uncounted)" like the plain rows (M_D 57dcf23)."
- **User's words:** "**6. Uncounted Star Power notes in the backend rows.** (33)" / "Today a phrase note the path doesn't count shows 0 points and a label like "Free SqOut", with no "(uncounted)" tag." / "After: "Free SqOut (uncounted)", like the plain rows." / "Recommended: yes." (`docs/audit/2026-10-04-phase-3-5-questions.md`:39-43), D48: "Uncounted SqOut rows get the "(uncounted)" tag (Q6)." (decisions file:124). The question's only example is "Free SqOut" (a note far past the SP end); it never showed the 5 ms case.
- **Code today:** both halves by reading, as above.
- **Verdict:** REOPEN
- **Why:** The tag half was asked and fixed. The +5 ms label split sat in this finding's own table and no question named it (it is M39, which the main session already knows). Same answer as M39: the user has not ruled how the plain "Standard" band (-10 to under 3) and the squeezed-out "Standard SqOut" band (-10 to under 10) relate.
- **Examples:** note 0 ms after the SP end, squeezed out: "Standard SqOut" (counted, `off < band` and counted); plain row: "Standard". Note 150 ms after: plain "Insane (uncounted)" (`off >= w`), squeezed-out "Free SqOut (uncounted)". Gap, note 5 ms after the SP end: plain row "Hard (uncounted)" vs squeezed-out row "Standard SqOut (uncounted)".

### Derivation audit 34: exactly 2.0 ms is Hard in the report but not warned in the GUI

- **Splits:** "Is a timing of exactly 2.0 ms difficult?": GUI `>` (not difficult) vs report tier `<` (Hard). Today one rule: `past_difficult_floor(ms) { return ms > kDifficultMs; }` (`src/core/model.h`:95), and the report's Normal cutoff is inclusive (`src/core/squeeze_rating.cpp`:177-180 `{"Normal", "t0", kDifficultMs}`, `src/app/report.cpp`:87-92 `*ms <= *t.cutoff`).
- **How it left:** question, answered "all recommended". Triage: "one past_difficult_floor, so exactly 2.0 ms reads Normal in the GUI and the report (M_D 57dcf23)."
- **User's words:** "**3. A timing exactly on a tier edge.** (34, 94)" / "Today a path at exactly 2.0 ms has no orange warning on the Paths tab but reads Hard in the report. A gap of exactly 170.0 ms reads Beyond in the report but has no "over budget" line in the details." / "After: both places agree." / "Recommended: a timing on the edge counts as inside, so 2.0 ms is not difficult and 170.0 ms is Insane+. That's how the Paths tab has always read it." (`docs/audit/2026-10-04-phase-3-5-questions.md`:25-29), D48: "A timing on a tier edge counts as inside (Q3)." (decisions file:121).
- **Code today:** by reading, as above.
- **Verdict:** GONE
- **Why:** One edge, inclusive, in both places. The question named both sides with the two example values.
- **Side picked without a ruling:** none.

### Derivation audit 35: report tile "Tightest squeeze" can show an early fill

- **Splits:** None as a rules split. It is a naming mismatch: the tile said "squeeze" over a value (`r.ms`) that includes a required early fill. Today the tile is named "Hardest ms" (`src/app/report.cpp`:257, column title `src/app/path_report_view.cpp`:89).
- **How it left:** question, answered "all recommended". Triage: "the report tile reads "Hardest ms" instead of "Tightest squeeze" (M_D 57dcf23)."
- **User's words:** "**7. The report's "Tightest squeeze" tile.** (35)" / "Today the tile can show an early fill's timing under the word "squeeze"." / "Recommended: rename it "Hardest ms" to match the column." (`docs/audit/2026-10-04-phase-3-5-questions.md`:44-47), D48: "The "Tightest squeeze" tile becomes "Hardest ms" (Q7)." (decisions file:125).
- **Code today:** by reading, as above.
- **Verdict:** PASS
- **Why:** The question described what the tile shows and the code matches the answer. It follows the item 23 ruling (a required early fill counts as a timing).

### Derivation audit 37: deact edge rebuilds the SP extension without the cap

- **Splits:** "Where does a pending SP end move when a phrase is collected during SP?": `extend_deacts` stopped at the cap ceiling vs `add_deact_edge` added a plain +1 bar. Today the edge's late-SqIn end is `extend_deacts({end}, c->timecode)` (`src/search/graph.cpp`:432), cap included (:296-316).
- **How it left:** question asked in chat, answered D29 (after step 1 merges), landed as D44 (T10). Triage: "phase 7 recheck (2026-10-04, D51): already fixed by step 2 T10/T11 (merged 7266643)."
- **User's words:** card: "D29: chose "Yes (Recommended)" to "Finding 37 (after step 1 merges): use the one SP-end extension that stops at a full meter ..."" (`docs/sessions/2026-10-04-bbf41dc4.md`:29). D29: "37: the SP-end extension during Star Power comes only from `extend_deacts`, which stops at a full meter, as Clone Hero does (0x20F6800); counted after the merge." (decisions file:69). D44: "The user said land it" (decisions file:111, D44 addendum) after measured score effects on generated test charts.
- **Code today:** by reading, as above.
- **Verdict:** GONE
- **Why:** One extension function feeds the node, the edge and the late SqIn. The user chose it after the effect was described.
- **Side picked without a ruling:** none.

### Derivation audit 38: record reader and JSON reader treat a SqOut with no chord tick differently

- **Splits:** "A SqOut with no chord tick: skip or resolve?": `windows_for_path` (record) vs `windows_from_json` + `cmd_score` (resolves the first chord). Today a stored record always carries the SqOut row's tick with its offset (`src/core/replay.cpp`:270-274, `act.sqout_row()`), so the record side has no missing-tick case; the dev tool still resolves an old dump by offset with a stderr note (`tools/replay.cpp`:343-361, `resolve_window_sqout`). Old records read Stale under plan decision 5, so the Preview never reads one.
- **How it left:** "code-only". Phase 7 file: "It keeps resolving an old dump's squeeze-out by its offset (38)." (`docs/audit/2026-10-04-phase-7-questions.md`:152), under "Code-only calls I'll make unless you say otherwise"; D51: "The code-only calls at the end of the sheet are approved as written. They cover findings 38, 59, ..." (decisions file:194). Triage: "phase 7 wave 1, merged to main 6c65208 (M7-1, 2026-10-04)."
- **User's words:** the two lines above. They name the JSON side ("keeps resolving") but never say the record side refuses.
- **Code today:** by reading, as above.
- **Verdict:** GONE
- **Why:** The record side can no longer lack the tick, so the two readers cannot differ on a current record. What remains (the CLI resolves an old hand-edited dump) is a dev-tool-only path the user approved.
- **Side picked without a ruling:** the dev tool keeps resolving; the record side keeps skipping a window with no deact node (`src/core/replay.cpp`:265). The approved line names only the first. Dev tool only; no user-visible effect.

## Not reached

None. All 23 ids were checked.

## Side notes

- `docs/audit/2026-10-03-fix-decisions.md`:209 (D53 item 4) says the Preview xN disc is "left as it is" and the game's disc was never video-checked. D10 (:25) says it was checked on FC video and D11 (:27) and the code (`src/app/preview_view.cpp`:597-600) drop the disc at the SP end. D53 item 4 is stale and contradicts the code.
- `docs/audit/2026-10-04-phase-7-questions.md`:27-31 (Q4) says "(D13 already decided that)" about a squeeze-out at exactly 0.0 ms. D13 (decisions file:31) only covers a SqIn at offset 0. `src/core/model.h`:381-382 repeats the claim ("a SqOut there still has to be hit late (D13)").
- `src/ui/theme.h`:25 (a comment) says kStarPowerColor is "A different gold from kBestPathColor"; Q26 (phase 3-5) asked for the header and the scrubber marks to share the best-path gold. I did not check the header.
- Item 33 shows the M39 3 ms vs 10 ms split again inside a finding the main session may not have listed under M39; it sits in the finding's own table at +5 ms.
- The packet lists phase-3-5 Q17 and phase-7 Q23/Q24 as "citing" findings 4, 2 and 3 only because of heading ids like "(13, R7.4)"; those questions are about other things and were not used.
