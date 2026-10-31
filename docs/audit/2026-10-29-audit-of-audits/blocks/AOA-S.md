# AOA-S: smaller audits, closures of the bad kind

Scout AOA-S, agent a9e269070407dd38b. Read-only. I ran out of time before I could check every item in the 09-25 audit's section 5; the ones I did not finish are under "Not reached".

How these files read: the 09-24, 09-25 and phase-6-recheck files are finding lists and plans, not closure lists. Almost nothing in them says "kept" or "by design". Most of what could leak was handed on as a plan decision or a question, so the blocks below check where each ended up.

### 09-24 derivation audit, finding 5: rating edge 2 ms vs engine leeway 3 ms

- **Splits:** "Does a plain backend row 2 to 3 ms past the SP end need a late hit?": the label and the engine said no (under 3 ms, `BackendSqueeze::summarystr` and `create_deactivated_path`) vs the rating said yes (past `kDifficultMs`, 2 ms, in `rate_activation`) (docs/audit/2026-09-24-derivation-audit.md:61).
- **How it left:** fixed. Commit f4eebf9a (2026-09-24): "Rate plain backend rows by the engine's leeway edge, not the 2 ms floor". Its body: "A plain row 2-3 ms past the SP end is counted by the engine, so it no longer trips the late scale line or the SP overfilled flag. The label, the warn and the flag all ask core::counted_without_squeeze with Rules::backend_leeway_ms."
- **User's words:** the plan `docs/superpowers/plans/2026-09-24-derivation-fixes.md`, line 52 (removed; `git show 444631e6:docs/superpowers/plans/2026-09-24-derivation-fixes.md`), lists it under "User decisions (already made)": "7. For 2-3 ms plain backend rows the 3 ms leeway wins everywhere, in the rating and the cap_clamped flag too." That names both sides (2-3 ms rows, the 3 ms leeway). I did not find the user's own typed words behind that line. The session card for 2a88c065 (2026-09-25-2a88c065.md) lists only the run-method rulings, not these.
- **Code today:** `rate_activation` asks `core::counted_without_squeeze(o, backend_leeway_ms)` (src/core/squeeze_rating.cpp:125), owned by src/core/backend_value.h:41-42 (`paid_by_sp_walk(offset_ms) || offset_ms < leeway_ms`), by reading. The 2 ms `kDifficultMs` (src/core/model.h:90) still exists and is a different question ("is this squeeze difficult"). The tier table still uses it as Normal's edge (squeeze_rating.cpp:180). The 10-03 audit owns that edge (docs/audit/2026-10-03-derivation-audit.md:754, "is a timing of exactly 2.0 ms difficult?").
- **Verdict:** GONE
- **Why:** the 2 ms vs 3 ms disagreement in the rating is gone; the rating, the label and the engine now share one edge.
- **Side picked without a ruling:** the 3 ms leeway won. The plan's decision list names it, but I found no quote of the user's own words. Main session: confirm the plan's "already made" list came from the user.

### 09-24 derivation audit, findings 3 and 4: calibration fill sign, average multiplier rounding

- **Splits:** (a) "Which sign does the calibration fill show?": details line showed raw `e_offset` vs header and report showed the flipped value (docs/audit/2026-09-24-derivation-audit.md:47-49). (b) "How is the average multiplier cut?": Song Details truncated at 3 decimals vs report `py_round3` (same file :55-57). Both were written "Which ... is your call".
- **How it left:** fixed. Commit c8100154 (2026-09-24): "The details line now shows the calibration fill as positive = early, like the header and the report. Song Details rounds the average multiplier with py_round3, like the report, instead of truncating."
- **User's words:** plan lines 48-49 under "User decisions (already made)": "3. Calibration fill sign: positive means hit early, everywhere." and "4. The average multiplier is rounded (py_round3) everywhere, so Song Details changes." Both sides are named. User's typed words not found (same card gap as above).
- **Code today:** one formatter `format_avg_mult` (src/app/path_view.cpp:430) and the details line calls `act.e_difficulty(/*verbose=*/true)` (src/app/path_view.cpp:230-232), by reading.
- **Verdict:** GONE
- **Why:** one formatter each now.
- **Side picked without a ruling:** "positive = early" and "round". The plan says the user decided them; I found no quote.

### 09-24 derivation audit, finding 32 and 09-25 audit section 5 item 4: Preview ignores song delay / .sng gets 0

- **Splits:** "Does the Preview shift the music by the chart's delay or Offset?": folder charts yes, `.sng` charts 0 (docs/audit/2026-09-25-codebase-audit.md:182), and the 09-24 audit said nothing read delay at all.
- **How it left:** fixed. Commit b868de9c "Mix Preview stems one at a time and apply the delay to .sng and .srb". Plan decision 9: "Preview audio delay and Offset: first a live check at the game, then the Preview honors them."
- **User's words:** card 2026-09-25-2a88c065.md ruling: "yes, remove the worktrees and branches. preview is good." (13:30 UTC, "confirms the Preview by ear for Task 17"). Memory file "CH audio delay/Offset rule" also records the nonzero-delay-replaces-Offset rule.
- **Code today:** `sng_delay_ms` and `parse_delay_ms` in src/app/preview_source.cpp:110-179, by reading.
- **Verdict:** GONE
- **Why:** both the folder and the .sng paths read the delay now.

### 09-24 derivation audit, findings 33 to 37: the squeeze-out first-note share, 3 ms leeway, fill constants, 4 tied paths, tie key

- **Splits:** None. These were "rules with no written reason" (docs/audit/2026-09-24-derivation-audit.md:217-237), asked as "I'd ask you for each one rather than guess" (:297).
- **How it left:** plan decision 13: "The 3 ms leeway, the squeeze-out rule, the fill constants (including the 1/32-beat landing slop), 4 tied paths, the Auto cap ladder and its 120 s budget are your own choices and become configurable in hydra_rules.ini." and decision 16: "The squeeze-out rule offers `first_note` (today) and `whole_chord` (every note in the chord loses its SP doubling)." (plan lines 58 and 61). Finding 37 (tie key without SP-ready time) went to D35: "Adopt the cheaper ready-time key" (docs/audit/2026-10-03-fix-decisions.md:81).
- **Code today:** not re-read in the time I had. src/core/rules.h exists; I did not open it.
- **Verdict:** PASS (for the "made configurable, defaults unchanged" exit; both values of the squeeze-out rule are named in decision 16). I did not verify the code or find the user's typed words.
- **Why:** the rule became a setting with both choices written down, so no hidden split remains.

### 09-24 derivation audit, "Checked and dropped" and the late-SqOut warning

- **Splits:** two claims closed with "on purpose": (1) "The two path-identity keys (all-0 check versus Preview) answer different questions on purpose." (docs/audit/2026-09-24-derivation-audit.md:277). (2) The late-SqOut warning on a free backend row: "The comment at `src/core/squeeze_rating.cpp` lines 90 to 95 explains that warning on purpose" (:31).
- **How it left:** "Checked and dropped" and an earlier-draft correction. No ruling quoted for either.
- **User's words:** None found; I did not search for them.
- **Code today:** (1) the phase-6 recheck says C4a made `path_overlay_key` return `path_identity` (docs/audit/2026-10-04-phase6-recheck/b.md:36), so the two keys now share an owner. I read that in the recheck, not in the code. (2) The rating code was rewritten by f4eebf9a, so the comment lines moved; not re-read.
- **Verdict:** not decided; a fresh scout should check both. Listed under "Not reached".

### 09-25 codebase audit, section 5 item 1: song names frozen at first analysis

- **Splits:** "Do reports follow a fixed song.ini?": the report name stays from the first analysis vs the library table (docs/audit/2026-09-25-codebase-audit.md:179).
- **How it left:** D63 (docs/audit/2026-10-03-fix-decisions.md:271): "Every other chart keeps the names from its newest analysis, so a fixed song.ini reaches the reports on the next analysis, as decided on 2026-09-26." The user's words from 2026-09-26 I did not find; D63 itself says the user "took the recommended answer".
- **Verdict:** GONE (by D63's own text; code not re-read).
- **Side picked without a ruling:** none visible; D63 cites an earlier decision I could not locate.

### 09-25 codebase audit, section 5 item 2: path report mixes chart modes

- **Splits:** "Which modes does the path report list?": every mode in one table vs only the current one (docs/audit/2026-09-25-codebase-audit.md:180).
- **How it left:** D235 (docs/audit/2026-10-03-fix-decisions.md:829): chose "Drop it (Recommended)" to "The path report now lists only your current settings, so every row's Mode cell would read the same." Also :586 "The report's Mode column and the row click's mode switch go."
- **Verdict:** GONE
- **Why:** the report lists one mode; the user chose to drop the column that would have shown the difference.

### Phase 6 recheck a, Q3 and b/c/d "Recorded as recommended": the xN disc gap, overlay-key cap, 2 s checks, WCAG 0.04045, dev-tool numbers

- **Splits:** the xN disc: the Preview disc still reads doubled after SP ends vs the drain box already says idle (docs/audit/2026-10-04-phase6-recheck/a.md:110, docs/audit/2026-10-04-phase-6-questions.md:35-41). Nothing else in this group is a split: the 2 s file check vs the 2 s report check agree (b.md:103), the overlay key ignoring the cap is today's behaviour kept (b.md:104), and 0.04045 vs 0.03928 give the same answer for every 8-bit colour (d.md:120).
- **How it left:** the disc was asked and recorded: D53 item 4 (docs/audit/2026-10-03-fix-decisions.md:205-210): "left as it is and recorded as a known gap: the disc shows the multiplier the last hit chord earned until the next chord, so it reads doubled for a moment after SP ends while the drain box says idle." The question text (phase-6-questions.md:37) names both sides: "the disc still reads doubled for a moment while the drain box already says idle". The rest left through D54: "Approved with D53: the 'Recorded as recommended' list in `docs/superpowers/plans/2026-10-04-phase-6.md` (removed; `git show 444631e6:docs/superpowers/plans/2026-10-04-phase-6.md`), as written."
- **User's words:** D53: "use recommended answers to everything" to the four questions. For D54 the user's words are the same sentence; I could not tell from the decision text whether the long "Recorded as recommended" list was in front of the user when they sent it. The list holds no split, so I count it as a number-nobody-chose exit.
- **Code today:** not re-read for the disc.
- **Verdict:** PASS for the disc (question names both sides, recommended line is the decision). PASS with a note for the D54 list: no split found in it, but its approval wording is the weakest link.
- **Why:** the disc gap is one a user ruled on with both behaviours stated.

### Phase 6 recheck c, finding 298: the `user_version` slot

- **Splits:** None: a stored number nothing reads.
- **How it left:** asked. D53 item 1 (fix-decisions.md:207): "Hydra stops writing it. A new database carries 0 there, an existing one keeps its 3". The question (phase-6-questions.md:11-13) describes the real code ("Nothing in Hydra ever reads that slot back").
- **Verdict:** PASS

### Phase 6 recheck a: Q1 (all-0 section) and Q2 (4- and 5-note multiplier squeezes)

- **Splits:** both were real display questions in the recheck. The phase 6 questions file says they "now belong to phase 7's E3 and E2" (phase-6-questions.md:7), so phase 7 owns them. I did not follow them further.
- **Verdict:** owned by phase 7 (E3, E2); not checked here.

### Phase 6 recheck d, finding 341: scrubber 2 px pad

- **Splits:** None: one Hydra copy of an ImGui internal (d.md:110-112).
- **How it left:** D54: "341: closed as 'single owner, ImGui internal'."
- **Verdict:** PASS. Each side: there is only one copy, in src/ui/preview_tab.cpp:161-164 (as read in the recheck, not re-read).

### Test audits (2026-10-10 and 2026-10-25 report)

- **Splits:** None in what the user sees. Both files are about test quality. Closures found: the 10-25 report leaves "the golden image's second frame and its limits (finding 14)" and "whether the tooltip text in test_report_view should stay pinned word for word" with the user, and says "your earlier choice was yes". Those are labelled as the user's call, not closed. Finding 5 (no frozen current-release library file) is a gap for the user's stored library, not a split.
- **Verdict:** no block needed. The 10-10 audit's "test_single_owner grows and never shrinks" is covered by ADR 0029 ("the single-owner rules are data", docs/adr/0029-...); I did not read it.

### Hook audit 2026-10-10: media guard (D189)

- **Splits:** "Who may write ffmpeg or yt-dlp output outside scratch?": the ruled rule was main session free under C:\ytdl, subagents denied, other non-scratch outputs ask (main) or deny (subagent) (memory hook-overhaul-2026-10.md) vs today's hooks, which have no such check.
- **How it left:** ruled, then built (T4b, branch ho-mediaout for the every-output version, never merged: card 2026-10-10-c3a986cf.md "Nothing was merged") [correction, 2026-10-30: it was merged into the hooks repo as 1380a37, "Merge mediaout: hook overhaul R6 wave", on 2026-10-10, and that commit is on the hooks repo's main]. Then the 2026-10-25 rebuild replaced every hook. The rebuild plan's dropped-hooks list (docs/superpowers/plans/2026-10-25-hooks-from-scratch.md:208) does not name the media guard, and the plan, the build handoff and the gates README contain no mention of ffmpeg, yt-dlp or media (grep found none).
- **User's words:** D189 (docs/audit/2026-10-03-fix-decisions.md:737): chose "Main session free (Recommended)" to "The media guard has been off since the main session moved from Fable to Opus. ..." (17:26 UTC, C:\ytdl). And D187: chose "Drop it (Recommended)" to "What should happen to the watchdog's ffmpeg kill? ..." (the kill, not the guard).
- **Code today:** grep for `ffmpeg|yt-dlp|ytdl` in C:\Users\Patrick\.claude\gates (*.py, *.ps1, settings.fragment.json, README.md), settings.json, CLAUDE.md and agents finds nothing outside state\agents\*.handback files. Rebuild cutover commit: 11b80b1 "Switch to the new hook set". By reading. I saw only the first 12 hook commands in settings.json (python gates and two powershell lines at 122 and 133 whose arguments I did not read).
- **Verdict:** REOPEN
- **Why:** the user ruled a rule (D189) and the hook that kept it is gone. The rebuild's list of what was dropped never mentions it, and its approval was "approve as reommended" over a plan that did not name the guard.
- **Examples:** a subagent runs `ffmpeg -i source.mp4 ... source.mp4` over a file outside scratch: D189 said deny; today the delete rule (literal-path deletes only) does not look at it, so nothing refuses. The main session writing under `C:\ytdl`: D189 said allowed; today also allowed (nothing checks), so this end agrees. A main session writing to `D:\` : D189 said ask; today nothing asks (the gap).

### Hook audit 2026-10-10: other broken items and decisions

- **Splits:** None; each is a gate behaviour, not two parts disagreeing.
- **Dispatchers fail open:** GONE by the rebuild. `common.py:32-47` runs every hook inside a try and, in "closed" mode, turns a crash into a deny ("Gate hook crashed, so the call is refused"), by reading. Cutover commit 11b80b1.
- **Stop gate never blocked:** retired. The six audit decisions were taken as "accept all" (D185, docs/audit/2026-10-03-fix-decisions.md:729, "approves all 21 plan recommendations"); the memory file lists decision 3 as "Retire stop_orchestrator_gate". The rebuild keeps no Stop-time block (plan :208 lists "bullet check at Stop" as dropped).
- **Watchdog kills every ffmpeg:** D187: chose "Drop it (Recommended)" to "What should happen to the watchdog's ffmpeg kill? ...". GONE.
- **Reviewers got implementer wording (role read from parent transcript):** GONE. The rebuild recognises a reviewer only by agent type (plan Q2, "Recommendation: yes"; CLAUDE.md "Derive-once review" says only `subagent_type: reviewer` counts).
- **Status line: minutes only:** matches the code today (src/progress.py `STATUS_NAG_MINUTES, STATUS_DENY_MINUTES = 5, 10`, line 16 in C:\Users\Patrick\.claude\gates\progress.py). D194 covers chained status lines.
- **Verdict:** PASS for all five (rulings D185, D187, D194 plus the rebuild cutover). The "accept all" in D185 is a batch answer, but each item is a removal of a bug, not a choice between two behaviours.

### Hook design audit 2026-10-25: sleeping computer counts against an agent's 30 minutes

- **Splits:** "Does an agent's time budget count machine sleep?": wall clock (counts) vs awake time only.
- **How it left:** plan out-of-scope paragraph (docs/superpowers/plans/2026-10-25-hooks-from-scratch.md:208): "The sleeping-laptop wall-clock issue from the audit (an agent's 30 minutes includes machine sleep) is not addressed; it is rare and the fix (counting only awake time) has no cheap source of truth."
- **User's words:** card 2026-10-25-dee9c61e.md: "otherwise, approve as reommended and create a hjandoff for the next session to build" (04:51 UTC). The plan text names the sleep issue, so the approval covered it.
- **Code today:** progress.py:43 and :60 use `time.time()` and elapsed minutes, so sleep still counts, by reading.
- **Verdict:** PASS
- **Why:** the plan states both sides and the user approved it as recommended. Note: the card's Roadblocks line says "the later plan covers the sleep gap", which is wrong; the plan says it does not.

### Hook design audit 2026-10-25: items 2 to 6 of "What would stop the stream"

- **Splits:** None; they are process proposals (shadow mode, replay harness, a merge helper, one owner for the numbers plus a docs-agree check, a gate freeze).
- **How it left:** the audit's six items were replaced by the from-scratch plan after the user said "make a new plan to get rid of every hook we have now and start over from scratch." (card 2026-10-25-dee9c61e.md). I did not find a statement that shadow mode, the replay harness or the merge helper were dropped on purpose; the card's open threads say "the shadow-mode length ... never answered". No user-visible behaviour is at stake.
- **Verdict:** no block; listed in side notes.

## Counts per file

- docs/audit/2026-09-24-derivation-audit.md: 4 closures of the bad kind found (the two "on purpose" lines in "Checked and dropped"/finding 1, plus findings 3/4/5/32 left as "your call" and then closed by plan decisions); 5 blocks written.
- docs/audit/2026-09-25-codebase-audit.md: 7 section-5 items; 2 blocks written (items 1 and 2) plus the .sng item folded into the delay block; 4 items not reached.
- docs/audit/2026-10-04-phase6-recheck/a.md: 3 questions (Q1, Q2 moved to phase 7, Q3 asked as D53 item 4); 1 block.
- docs/audit/2026-10-04-phase6-recheck/b.md: 5 "decisions with no display change" (340, 207, 210, 220, R7.13) closed by D54; 1 shared block.
- docs/audit/2026-10-04-phase6-recheck/c.md: 1 question (298, D53 item 1) and the "numbers with no decision" list (D54); 2 blocks.
- docs/audit/2026-10-04-phase6-recheck/d.md: 2 closures (341 and the 288 threshold); 1 block (341) plus the shared D54 block.
- docs/audit/2026-10-10-test-suite-audit.md: 0 closures of the bad kind; no block.
- C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\handoffs\2026-10-25-test-audit\report.md: 0 closures of the bad kind (two items left "for you to decide"); no block.
- docs/audit/2026-10-10-hook-audit.md: 6 decisions + 4 broken items; 3 blocks (media guard REOPEN, the batch of five broken-item exits).
- docs/audit/2026-10-25-hook-design-audit.md: 2 closures (sleep gap, items 2 to 6); 2 blocks.

## Not reached

- 09-25 audit section 5 items 3 (number boxes commit on leave), 5 (dropping pre-1.7 migrations), 6 (retire ADR 0002; ADR 0002 and ADR 0016 both still exist) and 7 (time budget out of the rules fingerprint; `grep budget` in src/core/rules.cpp and rules.h found nothing, so it may have moved or been renamed). I found no closing record for any of them. A fresh scout should grep docs/audit/2026-10-04-phase-3-5-questions.md and the fix decisions for each.
- 09-24 "Checked and dropped" two "on purpose" lines (the path-identity keys and the late-SqOut warning).
- 09-24 findings 38 (dynamics counts carry no parser version) and 39 (deleted-Python pointers on scoring rules); the plan's decision 12 stamps dynamics rows. Not read further.
- The phase 6 recheck's Q1 and Q2 in phase 7 (E3, E2).
- The user's own typed words behind plan decisions 3, 4, 7, 9 and 13 of `docs/superpowers/plans/2026-09-24-derivation-fixes.md` (removed; `git show 444631e6:docs/superpowers/plans/2026-09-24-derivation-fixes.md`). The 2a88c065 card does not list them. The earlier 09-24 session card(s) were not searched.

## Side notes

- Card `2026-10-25-dee9c61e.md` says "the later plan covers the sleep gap"; docs/superpowers/plans/2026-10-25-hooks-from-scratch.md:208 says it is not addressed.
- The 2026-10-25 hook rebuild dropped many hooks without a dated user quote per hook; the user's approval was "otherwise, approve as reommended" over a plan with an "Out of scope (for the user to approve as dropped)" list. The media guard is the one I found missing from that list. Other hooks that had user rulings in D185 to D194 and are not named in the dropped list: the scratch-root rule (D192, D193) is in the new delete rule; the 20-line/2,000-character doc-review limit (D191) is now owned by publish_gate.py per CLAUDE.md. I did not check D190 ("Let submit through" after a hard stop) against the new code.
- docs/audit/2026-10-04-phase6-recheck/c.md:11 says "ADR 0020 ... covers neither 269" etc.; not checked.
- Line 17 of docs/audit/2026-09-24-derivation-audit.md mentions "Your two open questions from the handoff still stand" (on the backend-row value function: should the engine's hot loop use the function, and what should the 0-point row say). Plan decisions 1 and 2 answer them ("shows 0 in Points with the tag 'squeezed out (uncounted)'", engine loop calls the shared function), but D-records for them are not in fix-decisions as far as my grep went.
