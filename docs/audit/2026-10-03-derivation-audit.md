# Derivation audit, 2026-10-03

This audit looked for places where Hydra works out the same fact more than once. It also looked for copies that disagree, and for rules nobody decided. It was read-only. No source, test or build file was changed, and nothing below has been fixed. Every fix waits for your yes.

The rule being checked is yours: every fact, rule or calculation is derived in exactly one place, and everything else calls that place. The first audit (`2026-09-24-derivation-audit.md`) had the same rule and still missed the double rating inside `rate_activation`. This run was built to close the four gaps that let that happen. It searched by question instead of by function name. It fanned out by question family instead of by folder. It did not accept code comments as decisions. And it proved coverage with a mechanical ledger instead of stopping when nothing more turned up.

The companion file `2026-10-03-derivation-ledger.md` lists every function in scope and who read it.

## How it was done

The audit ran in six steps.

1. **Calibration.** One agent got `src/core/squeeze_rating.cpp` as it stood at commit 707c285, with no hint of what was wrong there. It had to find the two known problems before anything else ran. It found both. The section near the end gives the details.
2. **Finders.** Nine agents each took one family of questions across the whole repo: SP windows and ends, backend scoring, squeeze timing and ratings, fills and activation, star and score totals, tempo and timing, parsing, store and versions, and display formatting. Two more traced every number, word and colour on the Paths, Stars, Dynamics and Preview tabs, the reports and the CLI output back to the code that decides it. They confirmed what the screen shows with `hydra_uitest`, and with a small probe built in scratch for the Preview overlay, which `hydra_uitest` cannot read. Together they raised 302 candidates.
3. **Verification.** Twenty-three different agents checked those 302 against the code. A candidate survived only if its verifier found every copy it relied on. For a drift claim, the verifier also had to produce the input where the copies disagree. 205 were confirmed in full, 92 in part, and 5 were refuted.
4. **Sweep.** The finders had read 841 of the 2,688 functions. Ten sweep agents read every one of the rest and gave each a ledger row. They raised 174 more candidates. Fresh verifiers confirmed 108 in full and 63 in part, and refuted 3.
5. **Write-up.** Writers turned the 468 surviving candidates into findings, merging candidates that described the same problem. One more agent then found findings that two groups had reported separately, and those were merged too.
6. **Completeness passes.** Fresh agents, each with one job, looked for anything the audit had missed. Two passes ran one agent each, then four rounds ran eight agents each. Every round still found new items, so the stop rule was not met. You stopped it after round 6. Their 63 findings are included below, and section 3 gives the details.

Some claims were proven by running tools, and the findings say which. The runs used `hydra_replay`, `hydra_batch`, `hydra_uitest` and `hydra_tests` from `build-cpp/Release`, built 2026-09-29 at 22:07. The only source changes since then are the version header and one deleted comment in `src/ui/main.cpp`, so the binaries match the game logic at HEAD. Other claims were checked by porting the arithmetic to Python in scratch. Everything else was reasoned from the source.

One verifier accidentally started `hydra_batch` with no arguments and killed it at once. That run wrote to `build-cpp/Release/hydra.db` at 10:01. That file is the build folder's test database and is not tracked by git. No tracked file changed; `git status` is the same as when the audit started.

"Not found where I looked" means exactly that. A grep that finds nothing does not prove a thing is absent.

## The short version

The rule is broken in many more places than the first audit found. After verification and merging there are 352 findings: 122 drifts, 13 double readings, 166 duplicates that agree today, and 51 undecided assumptions. 26 of them are wrong or contradictory on screen today.

The double rating you found on 2026-10-03 is confirmed and still in the code. Inside `rate_activation`, one SqOut is judged at two different SP ends. Its backend row reads the scale at the final end, and its SqOut entry reads the scale two measures earlier. A verifier built the input where the two disagree. It is finding "One SqOut chord is judged at two different SP ends" in the drift section.

The biggest pattern is the one ADR 0011 fixed once. A display or tool rebuilds a fact only the engine knows, and gets it wrong in a corner case. The Preview SP gauge shows a full bar after a late SqIn on Epidermis where the engine has 0.875. The transfer scale steps back two measures to find the pre-SqIn end instead of reading it. The replay decides on its own which chords Star Power pays.

The second pattern is parsing at lower difficulties. The Dynamics tab at Hard, Medium and Easy reads Expert's 2x kicks. A verifier reproduced this with `hydra_uitest` on Car Bomb, The Sentinel. Disco flip at Hard and below follows Expert's markers, shown with `hydra_replay` on Band Like That.

The third pattern is formatting with no owner. The same squeeze reads 13 ms on the badge and 12 ms when copied. Percentages round down on one tab and to nearest elsewhere. "SP cap: 1 bars" appears because the count-with-noun rule is retyped. Star Power is gold in one Preview box and teal on the highway.

Most of the rest are duplicates that agree today, and thresholds nobody chose. Each duplicate is a place where the next change can split two copies. Each threshold needs a yes from you, or a different value.

The audit is not finished by your own stop rule. Completeness passes kept finding new items until you stopped them after round 6. Section 3 at the end gives the counts per round.

After delivery, round 7 added 44 findings and seven notes on what today's commits did to existing findings, numbered R7.1 to R7.51 in the section "Round 7 and the 2026-10-03 code".

## Drift: copies that disagree today (122)

These copies give different answers for some input today. Each one names that input. They are ranked by how visible the disagreement is to you.

### Visibility 1: wrong or contradictory on screen today

#### 1. After SP ends, the score box still shows doubled while the drain box says idle

The Preview has two readouts that each imply whether Star Power is on. They disagree right after the SP end (called D here).

The drain box, `build_drain_box` in `src/app/preview_view.cpp` (line 535), asks "is the meter draining at the playhead?". It says yes only while `a.ms <= now < a.sp_end_ms`, so it goes idle at D. The score box, `build_score_box` in the same file (lines 499-509), shows the multiplier the last hit chord earned. It holds that value until the next chord. That chord's `in_sp` comes from `replay_path` in `src/core/replay.cpp` (lines 116-134), which counts chords up to D and inside the 3 ms leeway after it.

These are different questions, and the verifier agrees they are not five copies of one rule. The engine (`ScoreGraph::build` in `src/search/graph.cpp`), `paid_by_sp_walk` and replay all agree on which chords are paid. Replay's tick clamp restates `paid_by_sp_walk`, but no disagreeing input was found. `build_sp_meter_curve` mixes ms and tick bounds in one function, also without a found conflict.

The visible conflict was shown by reading, not by a GUI run. Take a last in-SP chord at D-50 ms and the next chord at D+200 ms. At playhead D+10 ms the score box shows the doubled multiplier while the drain box reads "SP drain (if activated)". This happens on every activation, between D and the next chord.

Whether the disc should drop at the SP end is a user call. If yes, one "is time t inside the window" helper beside `paid_by_sp_walk` in `src/core/backend_value.h` should serve both boxes. The origin was not traced. No user decision was found in `docs/adr` or `CONTEXT.md`.

*Owner proposed: User call first; if they should agree, a shared time-in-window predicate in core/backend_value.h next to paid_by_sp_walk. Candidates: backend-16, spwin-9.*

#### 2. Report footer says its tiers match the squeeze ratings; they use different ladders

The question is: how hard is one SqOut (squeeze out, collecting the next phrase as Star Power ends)? Two ladders answer it in different words.

The report uses `timing_tiers` in `src/core/squeeze_rating.cpp` (around line 197), read through `report::tier_for` in `src/app/report.cpp`. The Paths-tab backend row uses `BackendSqueeze::summarystr` in `src/core/model.cpp` (around line 311). Both read the same offset number. W is the 85 ms hit window.

| SqOut offset | Report tier | Backend row label |
|---|---|---|
| 0 | Normal | Standard SqOut |
| -5 | Hard | Standard SqOut |
| -85 | Insane | Hard SqOut |

The report footer, around line 406 of `report.cpp`, says "Timing tiers match Hydra's squeeze ratings". That is false today. The verifier showed it by reading, backed by passing tests in `tests/test_path_view.cpp` (lines 461-470) and `tests/test_report.cpp`.

The tiers were never a copy of `summarystr`. In b257836 they were copied from a third ladder, `BackendSqueeze.ratingstr`. Commit 2c2eee4 deleted that as dead code on 2026-08-18 and left the footer pointing at it. So the user sees a SqOut at 5 ms called Hard in the report and Standard SqOut in the row, under a footer saying they match.

No ADR or CONTEXT.md entry says whether the two ladders should agree. Plan 2026-09-24 decision 19 only keeps label text. Rewording the footer, or letting `core/squeeze_rating` own one ladder, is the user's call.

*Owner proposed: report.cpp generate_report (reword the footer) or core/squeeze_rating (one ladder) -- user decision. Candidates: squeeze-1.*

#### 3. Timeline outlines any badged activation in warning orange; the row badge stays grey

The question is: when should an activation be shown in warning orange? The Paths tab answers it twice, with different facts.

`render_timeline` in `src/ui/paths_tab.cpp` (around line 187) draws an orange (`kWarningColor`) outline whenever the activation has any badge. `render_activation_row` in the same file (around line 273) colours the badge text orange only when `a.difficult` is set, which comes from `Activation::is_difficult`. Meanwhile `activation_badge` in `src/app/path_view.cpp` hands out badges to squeezes at or under 2 ms and to optional early fills, which are never difficult.

| Activation | Badge | Row badge text | Timeline outline |
|---|---|---|---|
| SqOut at 163 ms | yes | orange | orange |
| SqIn at 1.0 ms | yes | grey | orange |
| E1, early fill -20 ms | yes | grey | orange |

They disagree today; the verifier showed this by reading. So orange means "has a badge" on the timeline and "is difficult" on the row, on one screen. The verifier notes this is not a copied rule: the timeline reads a different fact.

It came in 7afb806 (2026-09-27) from the UI-redesign plan. The approved mockup only shows the difficult 163 ms case, so it does not settle this. No ADR or CONTEXT.md decision was found. Either `ActivationRowView::difficult` should drive both colours, or the timeline should drop the warning colour. That is a user call.

*Owner proposed: user decision: ActivationRowView::difficult drives both colours, or the timeline uses a non-warning colour. Candidates: display-5.*

#### 4. One squeeze reads 13 ms on the badge but 12 ms when copied

The question is simple: how do we write a squeeze timing as a whole number of milliseconds?

Two places answer it, and they use different rules. `activation_badge` in `src/app/path_view.cpp` (around line 53) rounds with `%.0f`. `Activation::notationstr_verbose` in `src/core/model.cpp` (lines 450 and 453) cuts off the fraction with `static_cast<long long>`. That second one feeds the text that Copy path and Ctrl+C put on the clipboard, through `Path::pathstring_verbose`. The one-decimal path button in `build_path_buttons` uses `format_ms_spaced` from `src/app/display_format.cpp`. That is a different precision, not a different rule, and it agrees with the owner.

| difficulty | badge (rounds) | copied text (cuts off) | path button |
|---|---|---|---|
| 12.6 | 13 ms | 12 ms | 12.6 ms |
| 162.6 | 163 ms | 162 ms | 162.6 ms |
| -0.6 (free SqOut) | -1 ms | 0 ms | -0.6 ms |
| 162.5 | 162 ms (ties go to even) | 162 ms | 162.5 ms |
| 12.3 | 12 ms | 12 ms | 12.3 ms |

They disagree today on any fractional difficulty. The verifier showed this by reading the code. A `ucrtbase` probe confirmed the `%.0f` result.

On screen, the activation row badge says "squeeze out 13 ms". The copied path says "(12 ms)" for the same squeeze.

The owner should be `app/display_format`. It already owns the one-decimal formatters. It should get one whole-ms formatter that both the badge and `notationstr_verbose` call. The badge came in 577ddc2 (2026-09-27). The cut-off came from the Python port, 3b69958 (2026-08-15). No user decision on rounding versus cutting off was found in docs/adr, CONTEXT.md or the User Guide.

*Owner proposed: app/display_format (one whole-ms formatter). Candidates: display-2, screenA-5.*

#### 5. SP gauge shows a full bar after a late squeeze-in; the engine gives less

The question is how much Star Power is left right after a phrase is collected while SP is running.

The engine answers it in two places in `src/search/graph.cpp`. `ScoreGraph::extend_deacts` (around line 256) handles an on-time collection: the new end is the earlier of "old end + 2 measures" and "collection + 2 x cap measures". `ScoreGraph::store_new_backend` (around line 241) handles a late squeeze-in (SqIn: a phrase hit just after SP should have ended). It extends SP by one bar counted from the OLD deact node (the note where SP ends). The Preview gauge re-simulates this itself in `build_sp_meter_curve` in `src/app/preview_view.cpp` (lines 153-178). It floors the meter at 0, adds a full bar at the collection note, caps it, then forces zero at the stored deact node.

| Case | Engine, bars right after collection | Gauge |
|---|---|---|
| On time | min(left + 1, cap) | same |
| Exactly at the end | 1.0 | 1.0 |
| Late SqIn | 1 - (overshoot)/2 | always 1.0 |

They disagree today, shown by a run. On Venetian Snares - Epidermis (cap 4, ms 10, depth 4), best path, activation 2, the phrase at tick 209760 lands a quarter measure late. The gauge reads 1.000 bar; the engine's window implies 0.875.

On screen, the gauge and its bar readout show 1.0/4 instead of 0.875. The gauge then drains faster than the drain box's "1 bar / X s" rate. It also hits empty at the collection instead of at the old deact node. On-time collections match.

The engine should own this. If the record stored the SP end after each collection, the gauge could draw straight lines between engine values. CONTEXT.md ("SP meter gauge") already promises it is "never re-derived". Introduced by aab5ff1 (2026-08-31) and 906881d (2026-09-24).

*Owner proposed: Engine: ScoreGraph::store_new_backend / extend_deacts should store the SP end after each collection. Candidates: screenB-1, spwin-8.*

#### 6. Report tooltip calls the average multiplier 'points per note'

The question is what the "Avg multiplier" number means.

`Path::avg_mult` in `src/core/model.cpp` (lines 610-615) divides the score without solo bonuses by `chart_base_score`, the score with every note at 1x. The result is a ratio, not points. Two descriptions open by calling it points. The comment above it in `src/core/model.h` (lines 450-452) says "Points per scored note, on average." The report's Avg multiplier column tooltip in `src/app/report.cpp` (line 108) says "Points per note on average". It then gives the right formula in the same sentence.

| Chart of plain 50-point notes, all at x4, no SP | Value |
|---|---|
| What the code shows | 4.0 |
| What "points per note" would mean | 200 |

The descriptions disagree with the code today, shown by reading. The number on screen is right. The tooltip's first words mislabel its unit, so a user hovering the column reads it as points.

The comment came in c89596b (2026-09-25). The tooltip text came in f1c6a3d / 5457d7d (2026-09-27). No user decision on the wording was found in CONTEXT.md, the user guide or docs/adr. `Path::avg_mult` owns the definition. Both texts should say "average multiplier: score without solos divided by the 1x base score".

*Owner proposed: Path::avg_mult (src/core/model.cpp). Candidates: score-20.*

#### 7. 'Optimal' means one path in the report but every tied path in Paths and Preview

The question is simple: which paths count as optimal?

Two places answer it differently. The HTML report decides in `collect_rows` in `src/app/report.cpp` (lines 266-282). It sorts the paths and gives each a rank. Only rank 1 is the best row. The page script's 'Best path only' filter keeps `rank === 1` (line 118), only rank 1 is bold (line 124), and `generate_report` counts records by rank 1 (line 389). The Paths tab decides in `build_path_buttons` in `src/app/path_view.cpp` (line 470). Every path tied at the top score goes in the Optimal group. `paths_tab.cpp` colours those gold, and `preview_view.cpp` (line 615) adds '(optimal)' to each.

| Top of the record | Report | Paths tab and Preview |
|---|---|---|
| p0 alone at the top | p0 best | p0 Optimal |
| p0 plus a tied variant p1 | p0 best and bold; p1 is rank 2, hidden by 'Best path only' | p0 and p1 both Optimal |

They disagree today on any record whose best path has a tied variant. A variant is an equal-score path the engine folds under its parent (`engine.cpp` lines 760-772). The verifier showed this by reading the code, not by running a chart.

On screen, a path marked Optimal in the Paths and Preview tabs is missing from the report's 'Best path only' view, and is not bold there.

Neither side is called wrong here. `CONTEXT.md` (line 68) says only that the first path is optimal, which matches the report. No user decision on ties was found in `docs/adr` (0002, 0016), `CONTEXT.md`, or the 2026-09-27 ui-redesign plan. The report rule dates from b257836 (2026-08-09, ported in 3b69958); the Optimal group came in 577ddc2 (2026-09-27). The rule should live once on `HydraRecord` in `core/model`, because it is a fact about the record, not about either view. Which meaning wins is a question for the user.

*Owner proposed: core/model (HydraRecord): one 'optimal set' rule both views read. Candidates: score-15, screenB-12.*

#### 8. Reports and the app strip Clone Hero text tags differently

The question is how Clone Hero rich-text markup gets removed from a chart's title, artist or charter before Hydra shows it. Rich text means styling tags like `<b>` or `<color=#e02222>` that the game renders as formatting.

Two functions answer it, and they give different answers.

The first is `strip_rich_tags` in `src/app/library_query.cpp` (around line 302). Its tag list, `kRichTags` at line 187, names eight exact tags: color, size, b, i, u, s, sub and sup. It removes only those, and it does not trim spaces. The library table, the details header and library search all use it.

The second is `report::plain` in `src/app/report.cpp` (around line 181). It removes any tag whose name starts with "color", matched as a prefix, and then trims spaces. The path report, the dmleaderboards page (`collect_dm_rows` in `src/app/dm_report.cpp`) and the fill comparison page (`src/app/fill_report.cpp`) all use it.

A third behavior sits in `src/cli/batch.cpp` (around line 248). The hydra_batch console line prints artist and title raw, with nothing stripped.

| Input | App (`strip_rich_tags`) | Reports (`report::plain`) |
|---|---|---|
| `<color=#e02222>Blood</color>line` | Bloodline | Bloodline |
| `<b>Bold</b> Song` | Bold Song | `<b>Bold</b> Song` |
| `<size=20>big</size>` | big | `<size=20>big</size>` |
| `<color>X</color>` | `<color>X` | X |
| `<colorful>X` | `<colorful>X` | X |
| `<colorful band>` | `<colorful band>` | (empty) |
| `  Title  ` | spaces kept | Title |

The two source findings looked like they disagreed on the "colorful" rows, but they don't. I checked `report::plain`. Once it sees `<color` (or `</color`), it drops everything up to the next `>`. So `<colorful>X` becomes "X", and `<colorful band>` with no text after it becomes empty. Both rows are right.

They disagree today. Two verifiers showed it independently by running Python ports of both functions. One port was checked against `tests/test_library_query.cpp`.

On screen, a chart whose metadata uses `<b>`, `<i>`, `<u>`, `<s>`, `<size>`, `<sub>` or `<sup>` looks clean in the Library. The same chart shows the raw tags in all three HTML report pages. Text with a bare `<color>` tag goes the other way: clean in the reports, raw in the app.

`strip_rich_tags` should own this rule. It is the fuller rule, it matches the Clone Hero tag set, and it has a test. `report::plain` should call it and keep its trim on top if the trim is still wanted.

No user decision allows two strippers. The verifiers checked `docs/adr/`, `CONTEXT.md` (line 62 covers library search only) and the 2026-09-27 UI redesign plan. `report::plain` arrived in 3b69958 (2026-08-15, the C++ port checkpoint), as a port of the Python regex. `strip_rich_tags` arrived later, in b89beed (2026-09-27, the library search work). That later commit added the fuller rule without moving the reports onto it.

*Owner proposed: app::strip_rich_tags in src/app/library_query.cpp; report::plain should call it (with a trim on top if still wanted). Candidates: display-6, sweep-tests2-c1.*

#### 9. Paths timeline and Preview scrubber use different song lengths

The question is where the song ends. The answer places each activation along a timeline and names the song's last measure. Two screens answer it with two different lengths.

The Paths tab uses the last note's start time. `build_activations` in `src/app/path_view.cpp` divides each activation's time by `store::song_length_ms` (around lines 206-208). That value comes from `src/store/record_store.cpp` (around line 462). The same function turns it into the timeline's end label (around lines 360-363).

The Preview uses the transport length. `PreviewTransport::load` in `src/ui/preview_transport.cpp` (lines 11-17) sets it to the later of the last note and the audio end minus the audio offset. That length drives the scrubber range, the time box's length measure and the activation marks (`preview_controller.cpp` around lines 233, 259 and 272). `build_scrub_marks` in `src/app/preview_view.cpp` (around lines 565-570) divides by it. `build_time_box` (around line 459) labels the song length from it.

| Last note 200 s, activation at 100 s | Paths | Preview |
|---|---|---|
| No audio, or audio ends at or before 200 s | 0.500, measure at 200 s | 0.500, measure at 200 s |
| Audio ends at 220 s | 0.500, measure at 200 s | 0.4545, measure at 220 s |
| Audio ends at 230 s | 0.500, measure at 200 s | 0.435, "of" measure at 230 s |

They disagree today for any chart whose audio outlasts its notes. This was shown by reading, not a run. On the Burnout GUI fixture they agree, because it has no audio: Paths reads m96 and the Preview reads m96.3.240. Both GUI tests pass. A worked split case: last note at 120,000 ms and audio ending at 125,000 ms. The Paths label still reads m96, while the Preview length and its scrubber marks run later.

On screen, the same gold activation mark sits further right on the Paths timeline than on the Preview scrubber. The Paths end label names an earlier measure than the Preview's "of" label. The UserGuide (lines 110 and 158) calls both of them the song's last measure.

Smaller copies sit inside this. The fraction formula (activation time over length, clamped 0 to 1) is written twice, once in `build_activations` and once in `build_scrub_marks`. "Last note start" is derived twice: `build_preview_scene` (preview_view.cpp line 267) reads `scene.notes.back().ms` instead of calling `store::song_length_ms`; it agrees today. The comment in `src/app/preview_view.h` (line 221) calls that value the scrubber's right edge, which is stale. `path_view.cpp` also inlines the ms-to-tick rounding that `tick_at` owns, and the "m" + measure label that `format_measure` owns.

The user must first pick chart end or audio end. That is a question for you, not a mechanical merge. Then one song-extent fact should feed both views. `store::song_length_ms` should own the last-note time, the audio tail should be one named rule in `PreviewTransport`, and one helper should own the fraction.

No decision was found. The verifiers checked ADRs 0004, 0005 and 0008, the rest of docs/adr/, CONTEXT.md ("Transport" says nothing about length) and the relevant commit messages. The 2026-09-27 UI redesign plan (around line 7956) says the timeline uses "the same value the Preview uses for its scrubber". That is only true when there is no audio tail.

One finding dated the max rule to 1da4638. The history shows it first came in ba0885a (2026-08-22). 1da4638 moved it into the transport module the same day, and 1b7eeb3 (2026-09-25) added the audio offset. The Paths fraction and the Preview scrubber marks came in 577ddc2 and 59a3dd9 (2026-09-27), with the tab wiring in c5e9758.

*Owner proposed: One song-extent fact, chosen by the user (chart end or audio end), fed to both views; store::song_length_ms owns the last-note time; one fraction helper shared by build_activations and build_scrub_marks. Candidates: display-7, sweep-tests2-c9, sweep-tests4-c17, tempo-26, tempo-5, tempo-6.*

#### 10. Dynamics tab reads Expert's 2x kicks at Hard, Medium and Easy

The question is which kick notes a chart has at the chosen difficulty and 2x Bass setting. A 2x kick is MIDI pitch 95 (or `N 32` in a .chart); it only exists on Expert.

Four places answer it. `Settings::effective_bass2x` in `src/app/config.cpp` (line 133) says 2x only when the box is on and the difficulty is Expert, and the analysis parses with it. `is_handled_note` in `src/parse/song.cpp` (around line 318) accepts pitch 95 at every difficulty, even though its comment says only Expert carries it. `MidiParser::optype` (around line 428) then adds it whenever the bass2x flag is true. `DynamicsLoadJob::run` in `src/ui/dynamics_load_job.cpp` (line 22) always parses with `kDynamicsParseBass2x` = true. Finally `DynamicsBreakdown::played_total` in `src/app/dynamics_breakdown.cpp` (around line 37) decides a second time whether the 2x row counts.

| Car Bomb - The Sentinel, Hard | Analysis parse | Dynamics parse |
|---|---|---|
| Pitch 95 read? | no | yes |
| Kicks | 1,147 | 828 kick + 319 "2x kick" |
| Total notes | 2,038 | 1,719 |

They disagree today, shown by a `hydra_uitest` run. Where a 95 and a Hard kick share a tick, whichever comes first in the file wins (`MidiParser::run_ops` swallows the duplicate). So 319 real Hard kicks become 2x kicks, and `played_total` then drops them from Totals.

On screen, the Dynamics tab at Hard/Medium/Easy shows a 2x kick row and percentage that cannot exist, and Totals undercount.

Pitch 95 came from Python Hydra (7521456, 2024-11-25), ported in 3b69958; the always-on Dynamics parse came in 4bc76ac (2026-09-24). No user decision covers 2x below Expert; the verifier checked CONTEXT.md, docs/adr/, plan 2026-09-24 and commit 133b52e. `docs/UserGuide.md` line 41 says 2x Bass only exists on Expert. The parser should own the Expert-only gate, so no caller can bypass it.

*Owner proposed: MidiParser in src/parse/song.cpp (gate pitch 95 on the Expert base), with Settings::effective_bass2x as the only setting rule. Candidates: screenA-1, parse-7, parse-21, parse-8.*

#### 11. Disco flip at Hard and below follows Expert's disco markers

The question is whether a disco-flip section is active at the difficulty being parsed. Disco flip swaps red and yellow inside a marked section; charts mark it per difficulty with `[mix N drums0d]`, where N is the difficulty number and 3 is Expert.

The disco patterns `re_disco_on` and `re_disco_off` in `src/parse/song.cpp` (lines 85-92) hard-code `mix 3`. `MidiParser::optype` (around line 504) and the `ChartDataEntry` constructor (around line 715) apply them at every difficulty. Meanwhile `difficulty_base_pitch` (around line 302) and `ChartParser::parse` (line 1028) do pick the notes by difficulty. So the regex holds a second, fixed answer to "which difficulty are we parsing".

| Event in file | Flip at Expert | Flip at Hard (Hydra) |
|---|---|---|
| `[mix 3 drums0d]` | yes | yes |
| `[mix 2 drums0d]` | no | no |

They disagree today, shown by a run. Band Like That (`testdata/input/common/IB24/T2/fanclubwallet - Band Like That/notes.mid`) at Hard Pro gives 151 red and 40 yellow cymbals inside Expert's disco spans. As authored, Hard has 40 red and 151 yellow. No Clone Hero ground truth in the repo settles whether CH filters disco by difficulty. The authored data points that way, though: Hard's hi-hat is already written on yellow there.

On screen, at Hard/Medium/Easy with Pro Drums on, the Paths scores, Preview gems and Dynamics rows show those notes flipped.

The regex predates the port (7521456, 2024-11-25, Python Hydra) and came into C++ with 3b69958 (2026-08-15). No user decision was found in CONTEXT.md, docs/adr/, the commit messages, or plans 2026-09-24 and 2026-09-26. The parser's single difficulty choice should feed the disco match too.

*Owner proposed: src/parse/song.cpp (build the disco match from the same difficulty choice as difficulty_base_pitch / difficulty_name). Candidates: parse-1.*

#### 12. Dynamics tab counts 2x kicks in one kick total and drops them in another

The question is which kick rows count toward a kick total on the Dynamics tab.

Two functions in `src/app/dynamics_breakdown.cpp` answer it, each with its own addition. `DynamicsBreakdown::kicks_total` (lines 27-35) always adds the Kick row and the 2x kick row, field by field. `DynamicsBreakdown::played_total` (lines 37-50) starts from `pads_total`, adds the Kick row, and adds the 2x kick row only when 2x Bass is on, field by field again. `pads_total` (lines 17-25) writes the same field-by-field addition a third time. There is no shared addition for `DynamicsCounts`.

Both totals reach the screen through `render_dynamics_panel` in `src/ui/dynamics_tab.cpp`. The "2x kicks: X of Y kick notes" line (around line 152) and the "All kicks" row (around line 183) use `kicks_total`. The Totals box uses `played_total` (computed at line 103, drawn around lines 200-208).

| 2x Bass | All kicks row | Kicks inside Totals |
|---|---|---|
| off | kicks + 2x kicks | kicks only |
| on | kicks + 2x kicks | kicks + 2x kicks |

They disagree today whenever 2x Bass is off. One finder reported them agreeing, but only checked 2x Bass on, where `played_total` does equal `pads_total + kicks_total`. The code confirms the disagreement with 2x Bass off. The verifier showed it by reading plus a scratch MIDI count on `testdata` IB24/T2 "Mayday Parade - Jamie All Over": 441 kicks and 12 2x kicks. With 2x Bass off, the tab says "12 of 453 kick notes" and All kicks = 453, while Totals counts 441 kicks. `tests/test_dynamics_breakdown.cpp` pins pads 11, kicks 4 and played 15, which only covers the 2x Bass on case.

On screen, the Kicks table and the Totals box on the same tab give different kick counts. `docs/UserGuide.md` (line 168) says that with 2x Bass off the 2x kick row "is left out of the totals". That holds for the Totals box but not for the All kicks row, so the guide may only mean the Totals box.

`DynamicsBreakdown` should own one kick total that takes the 2x Bass setting, ideally on top of one shared `DynamicsCounts` addition used by all three totals. Then `played_total` and the tab would call it instead of adding kicks themselves. Both totals came in with ec3ade9 (2026-09-21) and moved to the tab file in 2facc04 (2026-09-27). No user decision, plan, ADR or CONTEXT.md entry covers which total should drop 2x kicks.

*Owner proposed: DynamicsBreakdown in src/app/dynamics_breakdown.cpp: one kick total that takes the 2x Bass setting. Candidates: display-24, sweep-core1-c3.*

#### 13. The Stale explanation is typed four times in three wordings

The question is simple: what does the screen say when a stored result is Stale (out of date) or has no paths?

Four places answer it. `render_headline` in `src/ui/details_panel.cpp` (text around line 110) and `detail::render_record_state` in the same file (around line 229) carry the same sentence word for word. `render_headline` does not call `render_record_state`, even though that helper's comment says it exists "so the states read the same on each" tab. `render_chips` in `src/ui/library_table.cpp` (around line 180) and the tooltip in `render_table` (around line 440) carry a second and third wording.

| Where | Stale text |
|---|---|
| details panel, headline and tab state | "Out of date: this result came from another Hydra version or from different rules in hydra_rules.ini. Re-analyze to refresh it." |
| library chip hint | "Analyzed by another Hydra version, or under different rules in hydra_rules.ini. Re-analyze to refresh." |
| library row tooltip | same as the chip, but ends "refresh it." |

"No paths found." is also typed in both details_panel functions. A third copy in `build_record_status` in `src/app/path_view.cpp` (line 128) is only reached by tests.

They disagree today, shown by reading the literals. The meaning matches; the words do not. None of them reads the store's `stale_build`/`stale_rules` flags, so the screen always names both causes and never says which one applies. The verifier did not uphold the claim that any text is false.

The user sees one state explained three ways. The texts should live in one function in `app/user_messages`, fed by `store::stale_reasons`, because the store already knows the real cause.

Introduced by the 2026-09-27 redesign (99cbda2 for details_panel, a50b3c0 for library_table, from plan f1c6a3d). No user decision on one shared text was found.

*Owner proposed: One message function per record state in app/user_messages, fed by store::stale_reasons through RecordLookup.stale_build/stale_rules. Candidates: display-15, screenA-22, store-9.*

#### 14. The count-with-noun rule is retyped many times; some copies skip the comma, and SP cap 1 shows '1 bars'

The question is how to write a number next to a noun. The house rule is thousands grouped with commas (1,000), and the singular noun only when the count is exactly 1 ("1 bar", "3 bars").

`count_label` in `src/ui/library_dialogs.cpp` (around line 62) is the house rule. `counted` in `src/app/report.cpp` (around line 216) is the same body, byte for byte. `charts_text` in `src/ui/library_table.cpp` (around line 57) is the same rule with the noun fixed.

Several copies keep the plural rule but skip the grouping. They are `bars_text` (lines 16-18), `backends_label` (around line 347) and the score branch of `within_label` (around line 436), all in `src/app/path_view.cpp`, plus the star count in `details_panel.cpp` (around line 139).

Some places write counts with no rule at all. `render_dm_picker_modal` in `library_dialogs.cpp` (around line 649) prints each player's score count with a raw `%d` and always says "scores". `render_scan_modal` prints "(%d found)" and "%d unchanged since last scan" ungrouped (lines 217 and 223), while its own Done line uses `count_label`. `render_batch_confirm` (around lines 288 and 292) picks has/have with its own `n == 1` test; that choice agrees with the rule.

Three SP-cap labels skip the rule and always say "bars". They are `build_record_status` in `path_view.cpp` (line 141), the `cap_label` subtitle in `generate_report` in `report.cpp` (around line 398), and `src/cli/batch.cpp` (line 196).

| Input | House rule (`count_label`, `counted`, `charts_text`) | What the copy shows |
|---|---|---|
| SP cap 1, batch confirm / activation row | 1 bar | 1 bar |
| SP cap 1, status panel (`build_record_status`) | 1 bar | SP cap:  1 bars |
| SP cap 1, report subtitle | 1 bar | SP cap 1 bars |
| SP cap 1, hydra_batch | 1 bar | SP cap     : 1 bars |
| SP cap 1000, cap labels | 1,000 bars | 1000 bars |
| depth 1000 scores, Paths tab (`within_label`) | 1,000 scores | Within 1000 scores |
| dm picker, 1 score | 1 score | 1 scores |
| dm picker, 1234 scores | 1,234 scores | 1234 scores |
| scan modal, 1234 folders | 1,234 | (1234 found) |

They disagree today on reachable inputs, shown by reading. The settings bar (`src/ui/settings_bar.cpp` line 86) floors the cap at 1, so cap 1 is legal. The depth box has only a lower bound of 0, so depth 1000 is legal. The tests pin only caps 4 and 32, where every copy agrees. Bar counts in activation rows stay small, so `bars_text`'s missing grouping shows no difference in practice.

On screen, at cap 1 the song panel, the report and hydra_batch say "1 bars" while the batch dialog and activation rows say "1 bar". In score mode with depth 1000, the Paths buttons say "Within 1000 scores" while the batch summary says "1,000 scores". A big library's scan modal mixes "1234" and "1,234" in one dialog. The dmleaderboards picker can say "1 scores".

One `counted()` helper beside `group_thousands` should own the rule, and every label should call it, the SP-cap lines included. `report::counted` is the natural home, with `count_label` folded into it. The has/have case needs a small verb variant, because no noun follows the number. The copies came in on 2026-09-27 (5457d7d, a591bba, a50b3c0, 577ddc2). The hard-coded "bars" lines date from ba0885a (2026-08-22). The only stated convention is the agent-written UI redesign plan (`docs/superpowers/plans/2026-09-27-ui-redesign.md`). No user decision on separate helpers or cap-1 wording was found in CONTEXT.md, docs/adr/ or those commits.

*Owner proposed: One counted() helper beside group_thousands (report::counted, with count_label folded into it); every label calls it, the SP cap lines included. Candidates: display-16, sweep-tests1-c2, sweep-tests2-c5, sweep-ui1-c11, sweep-ui1-c12.*

#### 15. Note exactly at the playhead: score box says hit, highway says not yet

The question: has the note sitting exactly at the playhead been hit yet?

Two places answer it. `build_score_box` in `src/app/preview_view.cpp` (around line 502) uses an upper-bound search, so a chord whose time equals the playhead counts as hit. `build_highway_draws` in `src/render/highway_draw.cpp` says the opposite. It flashes a gem only when its time is strictly before now (around line 336). It also skips the lane glow when the time is at or after now (around line 316).

| Playhead vs note | Score box | Highway |
|---|---|---|
| before the note | not hit | not hit |
| exactly on the note | hit | not hit (no flash, no glow) |
| after the note | hit | hit |

They disagree today, and the equal case is normal use. '< Act / Act >' and `seek_activation` park the paused playhead exactly on the activation chord's time. Both sides read the same millisecond value, so they compare equal bit for bit. The verifier ran the two tests that pin each side (`tests/test_preview_view.cpp` near line 1044, `tests/test_highway_draw.cpp` near line 238). Both pass.

On screen: after jumping to an activation, the score box already includes that chord's points and combo. The highway still shows its gems waiting, unlit, at the strike line.

The score-box side came in 545d420 (2026-09-25) and has a user decision: the 2026-09-25 score-and-stepping plan says the box reads 'the last chord at or before the playhead', agreed in chat. The highway side came in ba0885a (2026-08-22) as part of the Onyx port (ADR 0008). Nothing decides that the two should differ. One shared 'struck at now' predicate in the app layer should own this. Which side wins is the user's call.

*Owner proposed: one app-level 'struck at now' predicate (src/app/preview_view) used by build_score_box and build_highway_draws. Candidates: sweep-media1-c2.*

#### 16. Running Star Power is teal on the highway and gold in the drain box

The question: which colour means 'Star Power is running' in the Preview?

Two places answer it, and both read the same activation window. `build_highway_draws` in `src/render/highway_draw.cpp` (around line 229) tints the highway floor with `hydra.sp_active_color` from `assets/preview/3d-config.json`: teal `#6cf7c6`, darkened by 0.2. The drain box in `src/ui/preview_tab.cpp` (around line 571) turns its accent gold, `IM_COL32(255,204,51)`, while SP runs and grey otherwise.

The gauge fill, the 'SP' label and the next-activation header in the same file are gold all the time. So gold means 'Star Power' in general there, not 'running'. The gold value is typed four times in `preview_tab.cpp` (around lines 503, 526, 548 and 571) with no shared constant. The teal also has a duplicate default in `src/render/preview_config.h`; that copy is covered by the look-numbers finding.

This is a difference by construction, found by reading. On screen today, while SP runs, the floor is teal and the drain box text is gold at the same moment.

The teal came in ba0885a (2026-08-22, the Onyx port). The gold came in aab5ff1 (2026-08-31) and was reused in 073dfa0 and c5e9758. ADR 0008 covers only the floor tint. The drain-box plan calls the active text 'SP gold', but that line is agent-written; its 'Spec approved 2026-09-27' note may cover it. CONTEXT.md says nothing on SP colours. The user should pick one colour. Either way, the gold wants one named constant.

*Owner proposed: needs the user's choice of one SP colour; teal lives in assets/preview/3d-config.json, gold needs one named constant in src/ui/preview_tab.cpp. Candidates: sweep-media1-c13.*

#### 17. Same drum note has two names: Paths and Preview versus Dynamics tab

The question is what a drum note is called on screen.

Two functions answer it. `ChordNote::str` in `src/core/model.cpp` (around line 57) names notes for path text. It reaches the Paths tab and the Preview's next-activation box through `Chord::rowstr`. `dynamics_row_label` in `src/app/dynamics_breakdown.cpp` (around line 54) names the rows of the Dynamics tab. Both read the same note fields to decide lane and type, so only the words differ.

| Note | Paths tab / Preview | Dynamics tab |
|---|---|---|
| Green tom, Pro Drums off | GreenTom | Green |
| Green tom, Pro Drums on | GreenTom | Green tom |
| Yellow cymbal | YellowCym | Yellow cymbal |
| 2x kick | Kick (2x) | 2x kick |
| Red, Pro Drums on | Red | Red snare |

They disagree today on every chart. The verifier showed this by reading. The gap is widest with Pro Drums off: the parser only marks cymbals in pro mode, so path text calls every yellow, blue and green note a "Tom", while Dynamics drops the type word.

The user sees one note named two ways when switching between the Paths tab and the Dynamics tab.

One naming function in core should own the words, with the Dynamics labels built from it. Or the user can decide the two tabs keep different words. No decision was found in docs/adr/, CONTEXT.md or the plans. `ChordNote::str`'s modifier text came in 396082a (2026-09-21); `dynamics_row_label` came in ec3ade9 (2026-09-21).

*Owner proposed: One naming function in core beside color_str / ChordNote::str, with dynamics_row_label built from it (or a user decision to keep two vocabularies). Candidates: sweep-tests2-c4.*

#### 18. Hardest squeeze shown as whole ms on the badge, one decimal elsewhere

The question is how a timing in milliseconds is written on screen.

The same hardest-squeeze number goes through two formatters. `activation_badge` in `src/app/path_view.cpp` (lines 37-55) has its own `snprintf` with `%.0f`, so it rounds to whole ms. The path button (path_view.cpp line 457) and `squeeze_sentences` use `format_ms_spaced` in `src/app/display_format.cpp` (lines 29-33), which prints one decimal. There are more hand-written ms formats too: `format_ms` (one decimal, no space) for the Early fill line, inline `%.1f` and `%.0fms` strings in path_view.cpp around lines 293-325, and `SPSqueeze::description` in `src/core/model.cpp`.

| Hardest squeeze | Activation badge | Path button and sentence |
|---|---|---|
| 163.0 ms | squeeze out 163 ms | 163.0 ms |
| 12.4 ms | squeeze in 12 ms | 12.4 ms |
| 12.5 ms | 12 ms (round-half-even) | 12.5 ms |

They disagree today on any non-integer timing. The verifier showed this by reading. Burnout's tests pin both 163 and 163.0.

The user sees 12 ms on the badge and 12.4 ms on the button for the same squeeze.

The two looks are approved. The 2026-09-27 UI redesign spec (`docs/superpowers/specs/2026-09-27-ui-redesign-design.md`, lines 35 and 47) and its mockup show exactly "squeeze out 163 ms" and "163.0 ms". That approves the look, not a precision rule. What nobody designed is the badge keeping its own formatter. `display_format` should own both styles. Whether the badge should change precision is the user's call. Introduced in 577ddc2 (2026-09-27).

*Owner proposed: display_format (one ms formatter per approved style, e.g. a whole-ms variant beside format_ms_spaced). Candidates: sweep-tests2-c6.*

#### 19. Analyze button says 'search' when the typed search filters nothing

The question is simple: is a library search on right now? Two places answer it with different tests.

The toolbar's batch button, in `render_actions_row` in `src/ui/library_toolbar.cpp` (line 74), checks whether the raw text in the search box is non-empty. The table footer, in `render_library` in `src/ui/library_table.cpp` (line 486), checks the parsed query through `LibraryQuery::empty()` in `src/app/library_query.cpp` (line 319). `LibraryModel::matches` in `src/ui/library_model.cpp` (line 155) uses the same parsed test, so it sets both the batch scope and the N on the button. The parsed test ignores parse errors, so text that parses to nothing counts as "no search".

| Typed text | Button (raw text) | Table and batch scope (parsed) |
|---|---|---|
| (empty) | library | not searching |
| `abc` | search | searching |
| `stars:7` | search | searching |
| `stars:9` | search | not searching (error only) |
| spaces only, `title:`, `""` | search | not searching |

They disagree today. The verifier showed this by reading `parse_library_query`, not by a run. Type `stars:9` and the button reads "Analyze search (12,345)...", which is the whole library count. Meanwhile the search box shows "stars: needs a number from 0 to 7", and the table shows every row with no Clear search footer. Clicking opens a confirm titled "Analyze library", so only the button's wording is wrong.

`LibraryQuery::empty()` should own this. It is the one that knows what actually filters rows, and the toolbar should ask it instead of the raw string. No user decision covers which test means "search active".

*Owner proposed: LibraryQuery::empty() in src/app/library_query.cpp, reached through app.library.query(). Candidates: P2-1.*

#### 20. The Preview keeps the old chart mode's notes after a difficulty, Pro Drums or 2x Bass change

The question is: which difficulty, Pro Drums and 2x Bass setting was the Preview's song parsed under?

The song really depends on all four inputs. `resolve_preview_source` in `src/ui/preview_load_job.cpp` (around line 35) parses with the notes path, Pro Drums, 2x Bass and difficulty. But `PreviewController::open` in `src/ui/preview_controller.cpp` remembers it by chart md5 alone (`open_key_`, around line 53). When the md5 matches (around line 38), it returns early or rebuilds only the overlay. It never reads the new settings that `render_preview_panel` in `src/ui/preview_tab.cpp` hands it every frame. `AppState::apply_settings` in `src/ui/app_state.cpp` (around line 614) switches the Paths tab and the overlay path to the new mode's record, but never reloads the Preview. The Dynamics tab answers the same question differently: `dynamics_cache_key` in `src/app/dynamics_breakdown.cpp` keys by notes path, Pro Drums and difficulty, so it re-parses.

They disagree today, shown by a `hydra_uitest` run. Open 'Beg' (Evans Blue, Expert only) on the Preview tab, then pick Hard. The Preview still shows the Expert chart with no error. Re-select the chart and the same settings give 'Preview failed: No Hard Pro Drums notes in this chart.' The Pro Drums and 2x Bass toggles take the same branch (by reading, not run).

On screen, the new mode's path, SP gauge and score box are drawn over the old mode's notes. The settings bar help text promises difficulty applies to the preview.

`PreviewController::open` should own this by comparing against one parsed-song key, built once from the same inputs `Settings::to_analysis_settings` uses. The bug dates from ba0885a (2026-08-22).

*Owner proposed: PreviewController::open in src/ui/preview_controller.cpp, comparing against one parsed-song key built from the inputs Settings::to_analysis_settings uses. Candidates: R5-A5-1.*

#### 21. The .chart and .mid parsers disagree on when a Star Power phrase ends

The question is: has this Star Power phrase ended, and which chord is its last one (the chord that awards the phrase)?

Each parser decides this on its own. In `src/parse/song.cpp`, the .mid side closes the phrase at the 116 note-off itself (`MidiParser::op_sp_end`, around line 357). Whether that runs before or after the chord at the same tick is fixed when `MidiParser::optype` sorts the ops (around lines 437 and 467). The .chart side turns 'S 2 len' into an end tick (`ChartParser::optype`, around line 909). It closes the phrase at the first later tick with entries at or past that end (`ChartParser::push_timestamp`, around line 948). `ChartParser::parse` never closes a phrase still open after the last tick. Only the marking step is shared, in `mark_sp_phrase_end`.

The verifier ran `hydra_replay score` on scratch charts with chords at 192 to 1152 and a phrase starting at 768:

| Phrase length | .mid flags | .chart flags |
|---|---|---|
| 192 (ends on a chord) | 768 | 768 |
| 500 (ends past the last chord) | 1152 | nothing |
| 0 (on a chord) | nothing | 768 |

Real charts hit this today. The user's library has 26 zero-length phrases on a note across 4,340 .chart files, for example Muse - Feeling Good. Modern Value's last phrase runs past the last note and is never awarded.

The flag drives Star Power collection, so paths, scores, the Preview's phrase notes and the SP gauge all change. One helper in `src/parse/song.cpp` should own the rule. Both checks came from the port, 3b69958. Which edge Clone Hero uses is not settled: no user decision was found in docs/adr, CONTEXT.md or commit messages. It needs a game check or a user decision.

*Owner proposed: One phrase-end helper in src/parse/song.cpp that both parsers feed a start and end tick. Candidates: R4-A5-1.*

#### 22. Free squeezes still show a timing figure, though docs promise nothing

The question is when a path or activation counts as needing squeeze timing. Squeeze timing means hitting a note early or late enough to move it into or out of Star Power. A negative hardest timing means the squeeze happens on its own. Exactly 0 ms is the edge case.

`attach_allzero` in `src/search/pather.cpp` (around line 48) treats a hardest timing of 0 or less as free, and its comments say "needs no squeeze timing". The all-0 search gets the same answer by passing 0 as the limit to `Engine::passes_ms_filter` in `src/search/engine.cpp`. `squeeze_sentences` in `src/app/path_view.cpp` (lines 72 and 97) treats exactly 0 as still to earn. The screens show a figure for any squeeze at all: `build_path_buttons` (line 456), `activation_badge` (line 37), and the report's Hardest ms column in `src/app/report.cpp` (line 105). The docs say otherwise. `docs/UserGuide.md` line 108 says a path needing no timing "shows nothing there". The report header says "A dash means it needs none".

| hardest | all-0 logic | sentence | path list / badge / report |
|---|---|---|---|
| none | free | none | nothing, dash |
| -163.0 | free | free wording | "-163.0 ms", "squeeze in -163 ms", -163.0 in Normal tier |
| 0.0 | free | "more than 0.0 ms late" | "0.0 ms" |
| 1.0 | needs timing | earn wording | figure shown |

They disagree today. A run of `hydra_uitest` on Burnout at depth 3 shows "377,895 · 3+ 1 1 -163.0 ms" for a path whose only squeeze is free. The badge and the report value were worked out by reading. How You Remind Me's "8- 0" path has a SqOut at exactly 0.0 ms. That path is not the best path, so the 0 ms split was shown by reading only. `rate_activation` in `src/core/squeeze_rating.cpp` also says 0 means earn, but its output is the same either way. The timeline outline part is already known as display-5.

The user sees a timing, a warning badge and a report figure on a path the docs call timing-free.

One predicate in `src/core/model.h`, next to `Path::difficulty`, should own this. Every place listed above would call it. Nobody decided where the 0 edge sits or whether free figures are shown. The verifier checked CONTEXT.md, docs/adr and the commits that added each piece: 21ddf53 (`attach_allzero`), 577ddc2 (the sentences), 5549a4c (per-path timing and the User Guide line), 5457d7d (the report text). Commit 5549a4c is Claude's wording, not a user quote. Whether a note exactly at the SP end counts as inside is still open (backend-9).

*Owner proposed: src/core/model.h, a needs_timing/is_free predicate beside Path::difficulty. Candidates: R5-A7-1.*

#### 23. Squeeze filter and Path limit also count required early fills, but the docs and help text say squeezes only

The question is: does a path's "hardest timing" include a required early fill (an E0 activation, where the fill must be hit early)?

The code says yes. `Activation::difficulty` in `src/core/model.cpp` (around line 424) takes the largest of the squeezes and the E0 fill. `summarize_path` in `src/store/record_store.cpp` builds the stored `hardest_ms` from it, and `query_matches` in `src/app/library_query.cpp` (around line 388) tests that for `squeeze<=N`. `Engine::act_difficulty` in `src/search/engine.cpp` (around line 681) feeds the Path limit the same way.

The words say no. The User Guide (lines 51, 82, 84), `CONTEXT.md` (lines 59 and 154), the Path limit help in `src/ui/settings_bar.cpp` (around line 137) and the comments in `library_query.h` and `library_query.cpp` all say "hardest squeeze". Line 84 adds that a path with no squeeze passes any limit. User Guide line 108 says "squeeze or early fill", so the docs contradict each other.

| Best path | Code `hardest_ms` | Docs say | `squeeze<=20` |
|---|---|---|---|
| no squeeze, no E0 | none | none | passes (agree) |
| no squeeze, E0 offset +10 | -10 | none | passes (agree by luck) |
| no squeeze, E0 offset -30 | 30 | none | code drops it |
| 15 ms SqOut + E0 offset -30 | 30 | 15 | code drops it |

This was shown by reading, not a run. On screen, `squeeze<=20` hides a chart whose best path has no squeeze. A Path limit of 20 drops alternates over a fill the help text never mentions.

`Activation::difficulty` should stay the one rule, and every doc should say "squeeze or required early fill". The alternative is a user decision to test squeezes only. No decision was found in `CONTEXT.md` or `docs/adr/`. The guide wording came in 4e230b8/f1c6a3d (2026-09-27).

A run showed the Path limit half. On testdata "black midi - Sugar／Tzu" with `max_tied_paths = 1`, the path '0 E3+ E5 E0' is over the limit at `--ms 0` and inside at `--ms 10`. Its only squeeze is the same 0 ms SqIn as '0 E3+ E5 E1', which is inside at 0, so the early fill alone decided it. The engine side is `Engine::passes_ms_filter` comparing `search_difficulty(p)` with the limit, and `Engine::act_difficulty` adding `early_fill_difficulty(e_offset)` whenever `is_e0` holds. Commit 7dded44 records your choice of raw squeeze ms at the SP end; no decision covers early fills. The help-text wording came from 99cbda2 and 4e230b8.

*Owner proposed: Activation::difficulty in src/core/model.cpp (the rule); the docs and help text should restate it. Candidates: R3-A2-1, R3-A3-2.*

### Visibility 2: visible in an edge case, in docs or in CLI text

#### 24. Orange scale line and eff. figure use two different "does the ratio move a figure" tests, and the 1 ms half is written three times

The question is whether the transfer scale changes a number the user sees. (The transfer scale, or ratio r, is the factor that converts frontend timing into movement of the SP end.) Two things on screen are meant to answer it. One is the orange scale line. The other is the "(eff. N ms)" figure, which restates a row's gap on the normal budget.

All the answers live in `src/core/squeeze_rating.cpp`. `transfer_is_material` (lines 71-77) drives the warn flag. It says yes if either of two tests passes. The first test asks whether the ratio moves the gap by more than `kTransferImpactMs` (1 ms). The second asks whether the gap is past the budget W(1+r). `rate_activation` then gates the eff. figure separately, with the first test only. It does this twice: once in the backend-row loop (lines 133-137) and once in the SqIn/SqOut note loop (lines 164-167). So the 1 ms test is written three times. All three copies compare `abs(effective_backend_ms(gap, r) - gap)` against `kTransferImpactMs` with a strict `>`.

The orange paint happens in `build_activations` in `src/app/path_view.cpp` (lines 256-259). It turns the line orange from the warn flags, but only for a side whose scale is shown (at least 0.005 away from x1.00, line 227). `docs/UserGuide.md` line 124 says the line is orange when the scale changes a figure, and that those rows show eff.

Here is how the two tests split, with W = 85 ms:

| Input | Moves figure > 1 ms? | Over budget? | Orange? | eff. shown? |
|---|---|---|---|---|
| r = 1.01, plain row at +171 ms | no (0.85) | yes (170.85) | yes | no |
| r = 0.994, plain row at +200 ms | no (0.60) | yes (169.49) | yes | no |
| r = 1.02, plain row at +171 ms | yes | yes | yes | yes (169.3) |

The three copies of the 1 ms test agree with each other today. The verifier checked this by reading, including the edge at exactly 1.0 ms, where all three say no. The first test already implies the warn flag. So in practice the eff. figure appears exactly when the first test passes.

The warn flag and the eff. figure do not agree today. Any row that is over budget but moved by 1 ms or less turns the line orange with no eff. figure. The first two table rows show it. The finder's probe, linked against `hydra_core`, showed the r = 1.01 case. The verifier reproduced both cases with a Python port. I re-checked the arithmetic and the `shows` gate against the code, and both cases paint orange.

The two source findings framed this differently. One said the screen "would" contradict itself if one copy's threshold changed. The code shows the contradiction already happens, through the budget test that only the warn flag uses. The other finding is right on that point.

On screen, the user sees an orange scale line, but no row shows an eff. timing. The row's rating label is also unchanged. That contradicts the guide, which promises eff. on the rows that turn the line orange. If someone later edits one of the three 1 ms copies, a second kind of mismatch appears: an eff. figure with no warning.

`core/squeeze_rating` should own one rule for "the ratio moves a figure". One helper should feed both the warn flag and the two eff. gates. The guide should describe that same rule. The rating module is the right owner because it already computes both outputs from the same gap and scale.

The pieces came in over three commits. The 1 ms clause in `transfer_is_material` came from dea4228 (2026-08-19). The backend eff. gate came from a4bbad6 (2026-08-28). The note eff. gate came from 4f5fcc1 (2026-09-28). The a4bbad6 message describes the intended behavior: show eff. only when it moves the number. It says an over-budget row at x1.00 "still warns", and the code comment at lines 98-99 repeats that. That commit was co-written with Claude and quotes no user. The verifier found no user decision for the split in docs/adr, CONTEXT.md or the 2026-09-24 plan's decision list. One more wrinkle: at exactly x1.00 the `shows` gate keeps the line gray anyway. So "still warns at x1.00" only reaches the screen at ratios like x1.01 or x0.99.

*Owner proposed: core/squeeze_rating: one "scale moves the figure" rule that drives both the warn flag and the eff. figure. Candidates: display-19, screenA-3, screenA-4, squeeze-6, squeeze-8.*

#### 25. One SqOut chord is judged at two different SP ends

The question is which SP end's transfer scale governs a SqOut. A SqOut is a phrase note hit late on purpose so it lands after Star Power ends.

`rate_activation` in `src/core/squeeze_rating.cpp` answers it twice. The squeezed-out backend row reads the `post` scale, measured at the deact node D (around lines 108-117). The same SqOut's entry in `act.sqinouts` reads the `pre` scale, measured two measures before D (around lines 152-158). `frontend_transfer_scales` makes `pre` differ from `post` only when the activation has a SqIn. The comment on `Activation::transfer_pre` in `src/core/model.h` says `pre` governs "the SqIn/SqOut lines". The header `squeeze_rating.h` says it governs the SqIn only. The engine measures the SqOut's offset against D (`ScoreGraph::add_deact_edge` in `src/search/graph.cpp`), so `post` is the end its number belongs to.

| Same SqOut, offset -50 ms | scale read | material? |
|---|---|---|
| backend row | post.early = 1.0 | no |
| sqinouts note | pre.early = 0.8 | yes (eff 55.6 ms) |

They disagree today. The verifier showed it by reading plus a Python port of the arithmetic. The input is an activation with a SqIn then a SqOut, and a tempo change between D-2 measures and D.

On screen, `build_activations` in `src/app/path_view.cpp` turns the scale line orange under "at the SqIn's SP end" because of the SqOut. The SqOut's own row shows no eff. figure.

`rate_activation` should own one rule so the row and the note read the same end. The split comes from dea4228 (2026-08-19), whose message puts SqIn/SqOut under `pre` and backend rows under `post`. That commit puts one chord under both ends and does not say which wins. Nothing was found in ADRs 0001 and 0011 or in CONTEXT.md.

*Owner proposed: core/squeeze_rating rate_activation: one rule for which SP end each squeeze is judged at. Candidates: squeeze-4.*

#### 26. The view hides ratios within 0.005 of 1 even when the rating warns

The question is whether the scale line should mention a ratio. A ratio (or scale) says how much a frontend timing error stretches or shrinks at the SP end.

Two layers answer it. `rate_activation` in `src/core/squeeze_rating.cpp` (around lines 107-132) sets a warn flag whenever `transfer_is_material` says the ratio matters. That includes any row whose gap is over the budget. Then `build_activations` in `src/app/path_view.cpp` (lines 227-228) applies its own `shows` test, which hides any ratio within 0.005 of 1. A matching `same` test treats the two SP ends as equal when their ratios are within 0.005. The line turns orange only when both layers agree (lines 256-259).

| Input (W = 85 ms) | Rating warns? | View prints the ratio? |
|---|---|---|
| r = 1.004, plain row at +171 ms | yes (budget 170.34) | no |
| r = 0.996, row at +169.8 ms | yes (fits at x1.00, not at x0.996) | no |
| r = 0.9952, plain row at +450 ms | yes, and the row prints eff. | no |
| r = 1.02, row at +171 ms | yes | yes, orange |

They disagree today. The verifier showed this with a Python port of the code, not a run of the real build. On screen, a ratio that decides whether a row is possible is never mentioned. In the +450 ms case the row shows "(eff. N ms)" with no scale line, and its tooltip (lines 300-305 skip the `shows` gate) says "x1.00". Hiding at exactly x1.00 is deliberate, and `tests/test_path_view.cpp` (lines 279-283) pins it.

The 0.005 is linked to the `%.2f` print format only by a comment. It came in with ba0885a (2026-08-22) and took its current form in 8d4d9e0 (2026-09-29). The verifier found no user decision in docs/adr, CONTEXT.md, the 2026-09-24 plan decision list, or those commit messages. UserGuide line 124 describes the behaviour, but it is a user doc, not a decision.

`core/squeeze_rating` should own this. CONTEXT.md already gives it "whether the scale is material enough to warn about". It should return which ratios to print, and the view should only format them.

*Owner proposed: core/squeeze_rating (rate_activation). Candidates: screenA-2, display-18, screenA-14, squeeze-9.*

#### 27. The pre-SqIn SP end is rebuilt by stepping back two measures

The question is where SP ended before a SqIn phrase extended it. A SqIn (squeeze-in) collects a phrase just as SP ends, which pushes the end out.

The search knows the answer. It is the destination X of the SqIn deactivation edge. `ScoreGraph::add_deact_edge` in `src/search/graph.cpp` (lines 363-364, and line 246 for a late SqIn) builds the extended end as `plusmeasure(X, +2)`. `Engine::branch_deactivate` in `src/search/engine.cpp` (lines 648-677) takes that branch but stores neither X nor the edge. Later, `frontend_transfer_scales` in `src/core/squeeze_rating.cpp` (lines 46-48) rebuilds the old end by stepping two measures back from the stored end D. Its own comment says this is exact only for the last extension.

| Case | Search's old end | Rebuilt old end |
|---|---|---|
| One SqIn, nothing after | X | X, or one tick before X |
| SqIn, then a plain phrase collected mid-SP | X | about X + 2 measures |
| Two SqIns | X for each | right only for the last |
| Cap clamp after the SqIn | X | wrong, because D is the cap ceiling |

They disagree today. The later-collection case was shown by reading. The one-tick case was shown with a Python port of `SongTiming::plusmeasure`, which truncates (`src/core/timing.cpp` lines 165-195). At resolution 192 in flat 4/4, X = 1537 goes to D = 3073 and back to 1536. 3200 of 6144 ticks over 8 measures fail that round trip. The ratio only changes when a tempo or meter change sits between the two ticks.

In that case, Song Details prints the "... at the SqIn's SP end" clause and the SqIn eff. figure from the wrong measure. The rebuild came in with 4fa0245 (2026-08-20). ADR 0011 says nothing outside the search reconstructs D.

The search should own this. It should store the old end (or its ratio) at copy-out, the same way it stores `deact_tick`.

*Owner proposed: the search (engine copy-out, ADR 0011 pattern). Candidates: squeeze-15, spwin-16, tempo-3.*

#### 28. Cap-clamped windows measure the ratio from the activation, not the collecting note

The question is which note's timing moves the SP end when the SP cap clamps a window. A clamp happens when a phrase collected during SP fills the meter to the cap.

The engine answers it. `Engine::advance` in `src/search/engine.cpp` (lines 504-505) stores the collecting note as `clamp_tick`, following ADR 0013. `extend_deacts` in `src/search/graph.cpp` shows the end is pinned to that note. But `frontend_transfer_scales` in `src/core/squeeze_rating.cpp` (line 30) measures both ratios from the activation tick and never reads `clamp_tick`. `rate_activation` (line 178) and `build_activations` in `src/app/path_view.cpp` (lines 265-267) read it only for the overfill warning.

Two docs speak to it. `docs/cap-clamped-squeeze-frontend-anchor.md` says the collecting note is the lever and calls re-anchoring "still open". Its claim that the record lacks the anchor has been stale since ADR 0013. `docs/UserGuide.md` line 126 says the squeeze numbers are unaffected. That holds for raw gap ms, but not for the ratio or the eff. figures.

Whether they disagree on a real chart is unknown. The cited Rolling in the Deep (Dirty Loops) case is flat 130 BPM 4/4, so both anchors give r = 1.0. A made-up case, worked by reading, does disagree. Put the activation in a 120 BPM section and the collecting note after a change to 60 BPM. The code gives r = 2.0; the collecting-note anchor gives r = 1.0.

On such a chart, the overfill warning would appear, but the ratio and eff. figures beside it would be measured from the wrong note. The cap doc records the user's video proof. Nobody has recorded accepting the re-anchoring.

`frontend_transfer_scales` should read the stored `clamp_tick` when it is set. The engine already owns that fact.

*Owner proposed: core/squeeze_rating (frontend_transfer_scales, reading stored clamp_tick). Candidates: squeeze-16, spwin-17.*

#### 29. Backend table rates a SqOut on a phrase note the engine never squeezes out

The question is which notes near the end of Star Power can be squeezed out. A squeeze-out (SqOut) means hitting a phrase note late so it lands after SP ends.

The engine answers it in `ScoreGraph::add_deact_edge` in `src/search/graph.cpp` (around line 360). Only the first SP phrase chord within 500 ms of the SP end becomes the edge's squeeze note. The graph still copies every nearby row into the edge's backends (lines 353-358). `resolve_sqout_note` in `src/core/replay.cpp` (around line 234) agrees with the engine: it refuses any other chord as one "the engine never squeezes out". But `BackendSqueeze::summarystr` in `src/core/model.cpp` (lines 311-320) gives every SP phrase row a SqOut label, judged by its offset alone. The details table shows that label as the row rating (`src/app/path_view.cpp`, around line 322).

| Second phrase chord in the window | Engine / replay | summarystr |
|---|---|---|
| Can it be squeezed out? | No | Yes, rated by offset |

They disagree today. The verifier used the song in `tests/test_replay.cpp` (around line 558). It has phrase chords at ticks 2928 and 3036 before an SP end at 3072. The engine picks 2928. The 3036 row still reads "Insane SqOut" at the default 85 ms window. This was shown by reading plus a passing test run, not by driving the GUI.

On screen, the Paths details table offers a SqOut rating no path can achieve.

The graph should own it, because it is the only place that knows which chord it claimed. The label came from upstream commit c9c00cf (2025-08-12, "New ratings for the backends table") and was carried through the 3b69958 port. No user decision was found in docs/adr, CONTEXT.md or the 2026-09-24 plan's decision list.

*Owner proposed: search/graph (ScoreGraph::add_deact_edge), exposed or stored per row; BackendSqueeze::summarystr reads it. Candidates: squeeze-22.*

#### 30. The 500 ms squeeze window is tested in four places and typed as 500 twice

The question is whether a note is close enough to the SP end (the moment Star Power runs out, called D) to count as a backend row or a squeeze-out candidate.

Four code comparisons answer it. Each one reads `kSqueezeWindowMs` (500.0 in `src/core/model.h`, line 75). They are `ScoreGraph::is_recent_to_head` and the SqOut check in `ScoreGraph::build` (around line 131), both in `src/search/graph.cpp`, then `Activation::display_backends` in `src/core/model.cpp` (line 591) and `sqout_candidates` in `src/core/replay.cpp` (line 34). Two more places type the number 500 by hand. The Backend limit box clamps to 0..500 in `render_path_footer` in `src/ui/paths_tab.cpp` (line 503). The `hydra_replay` help text in `tools/replay.cpp` (line 112) says "within 500 ms".

| Place | Measures from | Test |
|---|---|---|
| graph (both) | last chart note, or D | gap < 500, signed |
| display_backends | stored offset from D or final SP end | abs(offset) < 500 |
| sqout_candidates | D | abs(gap) < 500 |
| paths_tab clamp, help text | none | literal 500 |

For ordinary rows on a deactivation edge, all four agree, and a row at exactly 500.0 ms is out everywhere. Tail rows disagree today, shown by reading, not by a run. A tail row belongs to an activation that never ends before the chart does. The graph keeps tail notes within 500 ms of the last note, but `display_backends` measures them from the final SP end. Example: last note at 10,000 ms, tail note at 9,700 ms, SP ends at 10,400 ms. The graph keeps the note (300 ms), but its offset is -700, so the Backends table and the stored record drop it. That contradicts the test name in `tests/test_squeeze_rating.cpp` (line 689). The score is unaffected.

On screen today, such tail rows are missing from the Backends table. If the constant changed, the Backend limit box would still cap at 500 and the help text would still say 500 ms.

One predicate next to `kSqueezeWindowMs` in `core/model.h` should own this, with one agreed anchor for tail rows. The clamp and help text should print the constant. Commit 9a77258's message says the UI's 500 was meant to equal the engine window. No ADR or CONTEXT.md entry decides where the test lives.

*Owner proposed: kSqueezeWindowMs plus one within_squeeze_window(a_ms, b_ms) predicate in core/model.h. Candidates: backend-6, spwin-3, squeeze-20, tempo-9, screenB-17.*

#### 31. Backend limit box and applied limit disagree on negative INI values

The question is what counts as a valid Backend limit. That is the number of milliseconds that decides which backend rows the details table hides.

Three places handle it, and none of them agree on the rule. The box clamps what you type to 0..500 in `render_path_footer` in `src/ui/paths_tab.cpp` (around line 503). `Settings::backend_limit` in `src/app/config.cpp` (around line 161) takes the absolute value and does not clamp. `Settings::load_file` (lines 78-79) reads the INI value with `atoi` and checks nothing, though the settings next to it are range-checked. `build_activations` in `src/app/path_view.cpp` only uses the result.

| Raw value | Box shows | Limit applied |
|---|---|---|
| -30 from hydra_settings.ini | -30 | 30 ms |
| -30 typed in box | 0 | 0 ms |
| 0..500 either way | same | same |

They disagree today for any negative value hand-edited into `hydra_settings.ini`. This was shown by reading, not by a run. The candidates' 900-vs-500 case does not show up on screen. `display_backends` already drops rows at 500 ms or more, so both limits hide the same rows.

On screen, the box can read -30 while 30 ms is the limit actually applied.

`Settings` should own one normaliser, used on load and on edit, with its upper bound taken from `kSqueezeWindowMs`. The 500 clamp is a literal copy of that constant. It came in with 9a77258 (2026-08-28), and the redesign carried it forward in 7afb806/99cbda2 (2026-09-27). The `abs()` and the default of 50 came from 133b52e (2026-08-28). CONTEXT.md lines 51-52 call the limit display-only but set no range or default. No ADR covers it, and no reason for the default 50 was found.

*Owner proposed: Settings in src/app/config.cpp: one normaliser used on load and on edit, bounded by kSqueezeWindowMs. Candidates: backend-15, screenA-17.*

#### 32. Replay decides on its own which chords Star Power pays, and doubles the multiplier on a squeezed-out chord that SP pays nothing

The question is: does Star Power pay this chord? The answer drives two things in the Preview score box. One is the running total. The other is the "xN" multiplier disc, which doubles when the answer is yes.

Three places answer it. The engine answers through the shape of its graph. `ScoreGraph::build` and `add_act_edge` in `src/search/graph.cpp` (around lines 112-185 and 327-344) pay the activation chord, then every later chord up to the deactivation node. `Engine::create_deactivated_path` in `src/search/engine.cpp` (lines 620-637) prices the backends, meaning the notes just after the SP end that still count. `backend_row_value` in `src/core/backend_value.h` (lines 40-45) says what one backend row is worth. For the squeezed-out chord (position `Exact`) it returns `sqout_points`, which can be 0.

The third place is `replay_path` in `src/core/replay.cpp` (lines 110-135). It rebuilds the same answer by walking ticks, from the stored act tick, deact tick and squeeze-out tick. A chord counts if it is at or after the act tick, its offset is treated as zero up to the deact tick, and it passes its own gate. The gate (lines 126-128) skips a row only if it is `After` the squeezed-out chord or past the 3 ms leeway. Those two checks repeat the first two lines of `backend_row_value` word for word. Every row that passes bumps `sp_claims`, and `in_sp = sp_claims > 0`. `shown_multiplier` in `src/core/timing.cpp` then doubles whenever `in_sp` is true. `build_score` in `src/app/preview_view.cpp` (line 213) puts that number on the disc.

The per-row price is shared through `backend_value.h`, so the SP points always match. What is written twice is the yes/no: "is this chord inside the window". The replay never asks the pricing helper whether the row actually paid anything.

| Row | replay_path says in SP? | backend_row_value pays |
|---|---|---|
| After the squeezed-out chord | no | 0 |
| Past the leeway, not counted | no | 0 |
| The squeezed-out chord, counted, `sqout_points` = 0 | **yes** | **0** |
| The squeezed-out chord, counted, `sqout_points` above 0 | yes | sqout_points |
| Everything else in the window | yes | points |

They disagree today, on one row. The earlier passes split on this. Two of them read the code and said the copies agree; one of those described the replay as skipping a squeezed-out chord. The code says otherwise. Line 126 skips only `After`; an `Exact` row still claims the window. So the replay is right about totals and wrong about the yes/no on that one chord.

The disagreeing input was shown by a run. `hydra_replay score` on a hand-built chart with a SqOut at -187.5 ms returned tick 3000, a one-note phrase chord under `first_note`, with `in_sp` true and 0 SP points. By reading `timing.cpp`, its shown multiplier is 2. `replay.h` (lines 217-218) itself says the squeezed-out note lands after Star Power has run out, so it is not doubled.

The agreement on totals is real. On the Epidermis best path the replay marks chords 193920 through 213120 as in SP, matching the stored window; both use inclusive bounds, act tick to deact tick. One finder reported 0 total mismatches over 3 paths on 97 charts; that was not re-run. Nobody tested a chord in one window's leeway that is also the next activation chord, the case the comment at lines 110-113 says both sides sum.

The safety check does not catch this. `PathReplay::faithful` in `src/core/replay.h` (line 206) compares only the six final totals and the window count. `hydra_replay selfcheck` and the Preview both use it. The squeezed-out chord adds 0 points either way, so totals still match and the check passes.

On screen, the Preview score box shows x2 at a 1x combo right after a squeezed-out one-note chord. The running total stays correct. More generally, any split that keeps the totals equal would show a wrong "xN" mid-song with the right final score. Only a split that changes a total would show "Score unavailable" instead (`preview_view.cpp` line 494). The game's own disc was not checked against a video. Whether a multi-note chord under `first_note`, where `sqout_points` is above 0, should show doubled is also unverified.

The engine should own which chords sit inside a window, because it is the scorer of record. For the backend edge, `backend_value.h` already prices the row, so it should also answer "paid by SP" as a named helper. Replay should read that instead of deciding it again. The record could also store per-chord SP membership, or the replay could be checked chord by chord rather than only by totals.

The replay was built in 30c4008 (2026-09-03) on purpose, as a second scorer that checks the engine. The pricing moved into `backend_value.h` in 706782b (2026-09-24). No user decision covers the second walk or the squeezed-out chord's disc. The verifiers checked `docs/adr/`, `CONTEXT.md`, the commit message and the 2026-09-24 plan. Decision 6 there covers `multiplier_after` only, and ADR 0011 covers only the stored deact node. The user should decide whether the replay's own walk counts as an allowed cross-check.

*Owner proposed: Engine (src/search/graph.cpp) owns which chords sit inside a window; core/backend_value.h should expose a named "paid by SP" answer that replay_path reads instead of re-deciding it. The user decides whether the replay stays as an allowed cross-check.. Candidates: backend-1, score-7, screenB-8.*

#### 33. Uncounted Star Power phrase rows show 0 points but no '(uncounted)' tag

The question: when the engine does not count a backend row, does the Backend timings table mark it "(uncounted)"?

The answer depends on the row kind. `BackendSqueeze::summarystr` in `src/core/model.cpp` (lines 311-327) has two ladders. The plain-row ladder calls `counted_without_squeeze` and adds the tag. The Star Power phrase-row ladder rates squeeze-out difficulty with fixed edges (-w, -10 ms, +10 ms, w). It never reads the leeway and never says "uncounted". Meanwhile the Points column, from `backend_row_value` in `build_activations` in `src/app/path_view.cpp` (lines 316-321), shows 0 for both.

| Row, past the 3 ms leeway, not squeezed out | Points | Label |
|---|---|---|
| Plain row | 0 | ... (uncounted) |
| Phrase row at +150 ms | 0 | Free SqOut |
| Phrase row at +5 ms | 0 | Standard SqOut |

This is inconsistent today. The +150 ms case was shown by a run: the test in `tests/test_path_view.cpp` (b[4]) checks exactly this and passes. The +5 ms case was shown by reading.

On screen, an uncounted phrase note reads 0 with no tag, while plain uncounted notes carry one. The lead text `kBackendTimingsLead` (around line 113) says a marked note is uncounted. It does not promise every uncounted note is marked, so this is a display gap rather than a broken promise.

Decision 19 in the 2026-09-24 derivation-fixes plan says uncounted rows show 0 and "The label text stays." So the fix is the user's call: tag in `summarystr`, or reword the lead. The origin was not traced.

*Owner proposed: BackendSqueeze::summarystr in core (or the lead text in path_view), pending the user's call given decision 19. Candidates: backend-8.*

#### 34. Exactly 2.0 ms is Hard in the report but not warned in the GUI

The question is simple: is a timing of exactly 2.0 ms difficult? One constant, `kDifficultMs` (2.0, in `src/core/model.h`), answers it. But two places compare against it with opposite edges.

The GUI side uses strict "greater than". That is `SPSqueeze::is_difficult` in `src/core/model.h` (around line 195), `Activation::is_difficult` and `Path::is_difficult` in `src/core/model.cpp` (around lines 432 and 568). The report side puts a path in Normal only when its ms is below the cutoff. That is the Normal row of `timing_tiers` in `src/core/squeeze_rating.cpp` (line 200), applied by `report::tier_for` in `src/app/report.cpp` (around line 235).

| Hardest ms | GUI warning colour | Report tier |
|---|---|---|
| 1.9 | none | Normal |
| 2.0 | none | Hard |
| 2.1 | orange | Hard |

They split today, only at exactly 2.0 ms. The verifier showed this by reading, and by two passing tests that pin both sides: `tests/test_model.cpp` (line 136) and `tests/test_report.cpp` (line 181).

On screen, that path's timing is uncoloured on its Paths-tab button and badge, while the report chip says Hard. The `model.h` comment claims one constant serves both, but that is a code comment, not a decision. No ADR, CONTEXT.md entry or plan decision picks an edge.

`core/model.h` should own one predicate for "past the difficult floor", and the tier ladder's Normal band should read it. Then the edge is written once. `kDifficultMs` came in ba0885a (2026-08-22); the Normal-below-2 row dates from b257836 and became a table in b9506f9.

*Owner proposed: core/model.h: one 'past the difficult floor' predicate read by the is_difficult functions and timing_tiers. Candidates: squeeze-2, display-4, screenB-10.*

#### 35. Report tile 'Tightest squeeze' can show an early fill

The question is: what does the report's "Tightest squeeze" tile show? The page code in `build_html` in `src/app/report.cpp` (around lines 142-150) takes the largest `r.ms` across rows. `r.ms` is `summarize_path`'s hardest ms, which includes a required early fill through `Activation::difficulty`. The column note on the same page (around line 105) describes `r.ms` correctly, as "the hardest squeeze or early fill".

So the label and the value disagree whenever the largest number is an early fill. The verifier showed this by reading. Take a report whose biggest hardest ms comes from an E0 activation (one that needs its early fill) with offset -30 and no squeezes. The tile reads "Tightest squeeze 30.0 ms", but that 30 is an early fill.

This is a naming mismatch, not a second rule. The user sees the word "squeeze" on a number that is not a squeeze, but only on charts where a fill is the hardest item.

It dates from b257836 (2026-08-09, the Python report) and was carried through the port in 3b69958. No user decision was found in docs/adr, CONTEXT.md or the 2026-09-26 audit plan. `report.cpp` owns the fix: rename the tile to match the "Hardest ms" column, or filter it to squeezes.

*Owner proposed: src/app/report.cpp build_html (rename the tile or filter to squeezes). Candidates: screenB-23.*

#### 36. Report page and app round exact half-way ms values differently

The question: how do we round a timing to one decimal for display?

The app uses printf. `format_ms_spaced` in `src/app/display_format.cpp` (around line 29) prints `%.1f ms`, and the Paths tab path button uses it. The report page uses JavaScript instead. `fmtMs` in `src/app/html_page.cpp` (around line 315) calls `n.toFixed(1)` for the Hardest ms column. The "Tightest squeeze" tile in `src/app/report.cpp` (around line 150) has its own inline `tightest.toFixed(1)`, a second copy inside the same page.

The two rules differ only on an exact tie, meaning a value stored as exactly x.x5. JavaScript rounds a tie away from zero. This build's printf rounds a tie to the even digit.

| value | report page | app |
|---|---|---|
| 12.25 | 12.3 | 12.2 ms |
| -12.25 | -12.3 | -12.2 ms |
| 0.25 | 0.3 | 0.2 ms |
| 163.75 | 163.8 | 163.8 ms |
| 12.3 | 12.3 | 12.3 ms |

The verifier showed the split with a node run and a `ucrtbase` printf probe. Nobody ran Hydra on a tie, and no real chart with an exact tie was found.

On screen, the report would say 12.3 where the path button says 12.2 ms for the same path.

The owner should be `display_format` on the C++ side. It should send already-formatted strings in the payload, so the page only prints them. The verifier also found that `summarize_path` in `src/store/record_store.cpp` repeats `Path::difficulty`'s max loop. The two agree today. No user decision on rounding was found in docs/adr, CONTEXT.md or the 2026-09-24 plan.

*Owner proposed: app/display_format (C++ side pre-formats or pre-rounds the payload). Candidates: display-3.*

#### 37. Deact edge rebuilds the SP extension without the cap, hiding squeeze-outs

The question is where a pending SP end moves when a phrase is collected while SP is running.

Three places answer it. `ScoreGraph::extend_deacts` in `src/search/graph.cpp` (around line 267) moves the end by two measures, but never past the cap ceiling (the phrase plus two measures per cap bar). `ScoreGraph::add_deact_edge` in the same file (lines 363-364) rebuilds the moved end as a plain +1 bar with no ceiling. `ScoreGraph::store_new_backend` (line 246) does the same +1 bar for a late SqIn, but its phrase lies after the node, so it always agrees. `build_sp_meter_curve` in `src/app/preview_view.cpp` (lines 167-169) redraws the capped level for the meter, with its endpoint forced to the engine's node.

| Case | extend_deacts | add_deact_edge |
|---|---|---|
| Ceiling not reached | end + 2 measures | end + 2 measures |
| Ceiling earlier (clamped) | ceiling | end + 2 measures |

They disagree today. A run showed it on a synthetic chart (100 ms measures, cap 2, phrase 300 ms before the end): the graph clamps to tick 2880, the edge expects 3072. `Engine::deactivation_type` (`src/search/engine.cpp`, lines 587-592) needs them equal, so it offers no squeeze-out and no normal end there. At cap 3 the SqOut path appears; at cap 2 it vanishes.

On screen, a path would silently be missing from the list, and the best score could be lower. Real charts need very short measures (under about 250 ms at cap 2). Whether the game agrees was not established.

`extend_deacts` should own the moved end. The deact edge and the late-SqIn bump should read it from the same extension (`ext_map`) instead of adding a bar themselves. The +1 bar dates from the port (3b69958, 2026-08-15); the clamped flag from 396082a (2026-09-21). No user decision covers the deact edge; ADR 0013 and CONTEXT.md "Cap-clamped window" cover only the stored clamp anchor.

*Owner proposed: ScoreGraph::extend_deacts in src/search/graph.cpp. Candidates: spwin-1.*

#### 38. Record reader and JSON reader treat a SqOut with no chord tick differently

The question is how to turn one activation into a replay window (the SP span plus the squeezed-out chord) when a field is missing.

Two readers answer it. `windows_for_path` in `src/core/replay.cpp` (lines 179-196) reads a stored record. `windows_from_json` in `tools/replay_json.cpp` (lines 12-63) reads hydra_replay's JSON, and `cmd_score` in `tools/replay.cpp` (lines 330-339) then fills the gap with `resolve_sqout_note`.

| Missing field | windows_for_path | windows_from_json + cmd_score |
|---|---|---|
| deact_tick | skip window | throw |
| sqout_tick on a SqOut | skip window | resolve to the engine's first chord, then price |

The missing-deact row refuses either way. The SqOut row disagrees today, shown by reading. Take a pre-v6 dump JSON with a SqOut offset of -93.75 ms and no `sqout_tick`. The record reader drops the window, so the Preview score box reads Unavailable and selfcheck reports FAIL. `hydra_replay score --path` resolves the chord and prints a score.

Only an old or hand-edited JSON can carry that input. A dump made today re-analyzes stale records and always stamps `sqout_tick`.

`core/replay` should own the missing-field policy, because the record path is the one users see. The JSON reader should share it, or the CLI resolve step should be named as the deliberate exception. `windows_from_json` came in 2045a21 (2026-09-09); the split came in 798026c (2026-09-24). The 2026-09-24 derivation-fixes plan header records user decisions #5 (old records read Stale, no guessing fallback) and #20 (refuse a SqOut the engine would not squeeze). Neither covers an old JSON dump.

*Owner proposed: core/replay (windows_for_path in src/core/replay.cpp). Candidates: spwin-6.*

#### 39. UserGuide says 'full meter' follows the song; the box uses current tempo only

The question is how long a full SP meter would last if activated at the playhead.

`build_drain_box` in `src/app/preview_view.cpp` (lines 526-551) answers it as cap x 2 x the bar length at the playhead. It ignores tempo or meter changes ahead. `docs/UserGuide.md` (line 162) says the idle box shows "how long a full meter would last from here", which promises the real duration.

| Song ahead | Box | Real duration |
|---|---|---|
| Steady tempo | same | same |
| Tempo change within the next 8 measures | local bar time x cap | follows the change |

They disagree today, shown by reading the test in `tests/test_preview_view.cpp` (lines 1155-1166). At 120 BPM switching to 60 BPM at 6000 ms, the box at 5990 ms reads "full meter 16.0 s". A real 4-bar SP from there lasts about 32 s. The finder's The Sentinel numbers (7.1 s vs 21.8 s) were not re-run.

On screen, near a tempo change, the box number can be many seconds off from what the guide says it means.

The code rule is the user's own choice. The plan `docs/superpowers/plans/2026-09-27-preview-sp-drain-box.md` (line 482) records "yes do the snap". `SongTiming::sp_end_ms` is not an alternative owner; only tests call it. So the fix is wording: the UserGuide line, or the box label, should say "at the current tempo". Introduced by b6c47ec (2026-09-27).

*Owner proposed: build_drain_box keeps the user-chosen rule; docs/UserGuide.md (or the box label) should be reworded. Candidates: screenB-4.*

#### 40. User guide states the early-fill sign backwards

The question is what the sign of the early-fill number means. Does a bigger positive number mean you must hit earlier, or does a more negative one?

The code answers in `early_fill_difficulty` in `src/core/model.h` (around line 181). It flips the engine's `e_offset`, so positive means "hit early". `Activation::e_difficulty` in `src/core/model.cpp` (lines 419-422) calls it. `build_activations` in `src/app/path_view.cpp` (around line 214) prints the result as "Early fill: ...". The report's Early fill column tooltip in `src/app/report.cpp` (line 107) agrees: it is how many ms early you must hit, and negative means slack. `docs/UserGuide.md` line 118 says the opposite: "The more negative the value, the earlier you have to hit."

| Situation | Screen and report | User guide implies |
|---|---|---|
| Must hit 12.3 ms early | `12.3ms (required)` | a negative number |
| 20 ms of slack (activation with skips) | `-20.0ms (optional)` | a positive number |

They disagree today. This was shown by reading the pinned cases in `tests/test_path_view.cpp` (lines 152-176), not by a run. The guide also writes `0ms` where the screen prints `0.0ms`.

A player who trusts the guide reads a required early hit as slack and misses the fill. The guide sentence dates from 6d94ded (2025-02-03). It survived the later sign flip and the 4e230b8 rewrite. User decision 3 in the 2026-09-24 derivation-fixes plan ("positive means hit early, everywhere") backs the code. `early_fill_difficulty` should own the sign, and the guide should describe it rather than restate it.

*Owner proposed: early_fill_difficulty / Activation::e_difficulty (src/core/model.h, src/core/model.cpp). Candidates: display-1, screenA-13, tempo-20.*

#### 41. CLI messages and comments still say results don't carry their fill rule

The question is whether each stored result remembers its fill rule, Clone Hero 1.0 or 1.1. Since 1f3efdf (2026-09-29) it does. The `legacy_fills` column is part of each result's unique key in `src/store/record_store.cpp` (lines 204 and 218). ADR 0010's superseded note and `CONTEXT.md` (lines 121-123) say 1.0 and 1.1 records sit side by side.

Five older texts still say otherwise. `main` in `src/cli/batch.cpp` prints "Legacy results are not tagged as legacy" (line 134) and "The rule is not stored on each result" (line 169). The `usage` text in `tools/replay.cpp` (lines 129-132) says "Every stored row is a 1.1 result". The comment in `activation_fill_deadline_ms` in `src/search/graph.cpp` (lines 61-64) says the legacy mode never writes the GUI's database. The comment above `MsIndex::ms_at_tick_f` in `src/core/timing.h` (lines 47-53) says it is never used in the graph, yet the graph's Ch10 branch has called it since 406d473. That comment also mentions a deleted squeeze solver and a `tick_at_ms` call that does not exist.

They disagree today, shown by reading, not a run. With the GUI's "1.0 fills" setting on, hydra.db holds `legacy_fills=1` rows. batch.cpp's own header comment (lines 15-17) already tells the right story.

CLI users are shown a false reason when hydra_batch refuses. The guards themselves are a kept choice (1f3efdf message, ADR 0010 note). Only the stated reasons are stale. They came from 406d473 (2026-08-31), 27c6856/283e028 (2026-09-26) and fcb0c3b (2026-09-03). The store's `Lens.legacy_fills` owns the fact. The messages should present the guards as a kept choice.

*Owner proposed: store::Lens.legacy_fills (src/store/record_store.cpp); ADR 0010 and CONTEXT.md own the wording. Candidates: display-21, fills-3, screenB-16, store-10, tempo-21.*

#### 42. User guide hard-codes 3 ms as the lower edge of the uncounted backend range

The question is where the uncounted backend range starts. That is the range where a double backend squeeze pays off.

The engine answers with `core::counted_without_squeeze` in `src/core/backend_value.h`. It uses `Rules::backend_leeway_ms` from `src/core/rules.h` (line 28), which defaults to 3.0 and can be set in hydra_rules.ini. `BackendSqueeze::summarystr` in `src/core/model.cpp` (line 324) uses the same edge. `docs/UserGuide.md` line 122 says to look for notes "in the `3ms` to `85ms` range (the upper edge follows the hit-window setting)". It marks the upper edge as a setting but fixes the lower one. Line 131 of the same guide does name `backend_leeway_ms`. So the guide gives the default as a setting in one place and hard-codes it in another.

| Row offset | Leeway | Table label | Guide reading |
|---|---|---|---|
| 3.0 ms | 3 (default) | Hard (uncounted) | candidate |
| 4 ms | 5 | Standard (counted) | candidate |

At the default they agree. With `backend_leeway_ms = 5` they disagree, shown by reading. A user who changed the leeway would hunt for rows the table now marks Standard.

The 3 ms leeway is the user's own choice (decision 13 of the 2026-09-24 plan). `Rules::backend_leeway_ms` owns it. The guide should say "from backend_leeway_ms (3 ms by default)".

*Owner proposed: Rules::backend_leeway_ms (src/core/rules.h). Candidates: backend-13.*

#### 43. User guide says untagged MIDI charts still show dynamics counts; they show zeros

The question is whether the Dynamics tab counts ghost and accent notes for a MIDI chart without the `[ENABLE_CHART_DYNAMICS]` flag.

`docs/UserGuide.md` line 170 says yes. Its words are "Hydra Deluxe shows the counts but notes that the game won't apply them." The code says no. `MidiParser::op_note` in `src/parse/song.cpp` (line 369) turns every note into Normal when the flag is missing. `count_dynamics` in `src/app/dynamics_breakdown.cpp` (lines 104-119) counts those parsed notes. `render_dynamics_panel` in `src/ui/dynamics_tab.cpp` then shows "This chart has no ghost or accent notes." and "Dynamics enabled: no (markings ignored by Clone Hero)".

| Untagged .mid, velocity-127 drum note | Guide | Code |
|---|---|---|
| How it is counted | as an accent | as a normal note |

They disagree today, shown by reading. A user who opens an untagged chart expects counts and sees zeros.

The guide line came in 4e230b8 (2026-09-27). ADR 0012 (lines 38-40) says the flag gates dynamics for kicks and pads alike, which backs the parser. The parser should own this, and the guide should be fixed.

*Owner proposed: MidiParser::op_note (src/parse/song.cpp); fix the guide. Candidates: parse-18.*

#### 44. ADR 0012 names a deleted function and a fixed kick pitch

The question is how the kick gets its ghost or accent dynamic.

ADR 0012 (`docs/adr/0012-kick-dynamics-are-priced.md`) states the decision correctly: the kick uses the same velocity rule as the pads. Its pointers into the code are stale. Line 45 says "`allows_dynamics()` now answers yes for every lane". That function was deleted in a244301 (2026-09-26), and grep over src, tests and tools finds no trace of it. Line 35 calls the kick "pitch 96 (the difficulty's own kick)". Only Expert's kick is 96. The parser uses `difficulty_base_pitch` in `src/parse/song.cpp` (lines 302-309), which gives 96, 84, 72 or 60. `MidiParser::optype` (lines 411-432) computes `vel_dyn` once and applies it to the base pitch, the four pads and pitch 95.

| Difficulty | ADR's kick pitch | Parser's kick pitch |
|---|---|---|
| Expert | 96 | 96 |
| Hard | 96 | 84 |

The ADR's references disagree with the code today, shown by reading. The rule itself matches. Nothing shows on screen. A maintainer using the ADR as a reference would look for a function that is gone, or treat pitch 96 as fixed.

The ADR text dates from 396082a (2026-09-21). It should point at `MidiParser::optype` and `difficulty_base_pitch`.

*Owner proposed: MidiParser::optype and difficulty_base_pitch (src/parse/song.cpp). Candidates: parse-26.*

#### 45. ADR 0011 and a model.h comment describe the SP end without the cap clamp

The question is where an activation's Star Power ends. That point is the deact node, D.

`ScoreGraph::extend_deacts` in `src/search/graph.cpp` (lines 256-278) is the only code that computes it, and the engine stores the result. With no SP cap, each phrase collected during SP pushes D 2 measures later. With a cap, D is the earlier of that and the phrase plus 2×cap measures. Two texts give only the first half. ADR 0011 (`docs/adr/0011-deact-node-is-stored-by-the-engine.md`, lines 4-6) and the comment above `transfer_pre` in `src/core/model.h` (lines 308-310) both say "+2 measures per collected phrase". Neither mentions the cap.

| Case | Docs give D at | Engine stores D at |
|---|---|---|
| No cap | prev + 2 measures | prev + 2 measures |
| Cap, ceiling ties (phrase at 3840) | 6912 | 6912 |
| Cap, ceiling earlier (phrase at 3072) | 6912 | 6144 |

They disagree today. This was shown by reading the test "SP cap overfill: a mid-SP phrase that clamps" in `tests/test_search.cpp` (lines 665-698), not by running it: cap 2, activation at tick 2304 with 2 bars.

Nothing on screen changes, because no code re-derives D. A reader rebuilding D from the ADR would get capped windows wrong, which is the drift ADR 0011 was written to stop. ADR 0011 came in 3859380 (2026-09-03). ADR 0013 later describes the clamp, but ADR 0011's sentence was never qualified. `extend_deacts` should own the rule, and both texts should defer to it.

*Owner proposed: ScoreGraph::extend_deacts (src/search/graph.cpp). Candidates: spwin-18.*

#### 46. Cap-clamped squeeze doc points at three deleted functions

The question is which code prices the frontend of a cap-clamped squeeze window.

`docs/cap-clamped-squeeze-frontend-anchor.md` names four functions as current code (lines 13-14, 47-53, 69-70, 81-82): `sp_end_shift_ms`, `required_frontend_ms`, `exact_even_split_ms` and `frontend_transfer_scales`. Only `frontend_transfer_scales` in `src/core/squeeze_rating.cpp` (line 26) still exists. Decision 21 of the 2026-09-24 derivation-fixes plan deleted the other three. Grep over src, tests and tools confirms they are gone. The doc's claim about `frontend_transfer_scales` still holds: it anchors on the activation tick.

Two more sentences are stale. Lines 85-87 say the record does not carry the clamp anchor. It does: `Activation::clamp_tick` is stored (`src/core/model.h` line 288, ADR 0013). Line 64 says the timing tier can come out wrong. The report's Timing tier column bands raw ms (`src/app/report.cpp` lines 220-224), as ADR 0001 decided. The transfer scale only adds an "eff." figure in path_view. The verifier did not trace every tier consumer.

The doc disagrees with the code today, shown by reading. Nothing changes on screen. A future fixer is sent to code that no longer exists. The doc predates decision 21 (executed 2026-09-25) and was not bisected. The doc should name only `frontend_transfer_scales` and point at `clamp_tick`.

*Owner proposed: docs/cap-clamped-squeeze-frontend-anchor.md (should name frontend_transfer_scales and clamp_tick). Candidates: spwin-19, squeeze-17.*

#### 47. model.h promises an old-record fallback the reader doesn't have

The question is what happens when the app reads a record written in an older format.

The comment on `HydraRecord::rules_fingerprint` in `src/core/model.h` (lines 465-472) says an older blob reads back `core::kNoRulesFingerprint`, a "no rules" placeholder. The code does not do that. `rebuild_record` in `src/store/path_codec.cpp` (lines 340-344) throws `SerializeError` on any non-current format, and `decode_path_node` (lines 270-271) does the same.

ADR 0014 has a related stale reference. Lines 29-31 and 79-80 name `kRowReadySql` with an `IN (?, ?)` test for two fingerprints. Today it is the function `row_ready_sql` in `src/store/record_store.cpp` (lines 349-355), which compares one fingerprint.

| Structure blob in format 5 | model.h comment | Code |
|---|---|---|
| Reading it back | fingerprint = kNoRulesFingerprint | throws SerializeError |

They disagree today, shown by reading. Nothing on screen changes. A maintainer could rely on a fallback that does not exist.

The "6 vs v7" numbering the finder also flagged is not drift. ADR 0017 (lines 37-39) names format 6 "record format v7" on purpose. The sentinel comment came in 046cc0b (2026-09-26). `store/stored_versions.h` holds the stamps (ADR 0018), and `rebuild_record` owns the old-format behavior. The comment and ADR 0014 should describe what they do today.

*Owner proposed: rebuild_record (src/store/path_codec.cpp); stamps in src/store/stored_versions.h. Candidates: store-27.*

#### 48. ADR 0011 says backend rows exist only after the SP end

The question is which notes become backend rows around the SP end, D.

`ScoreGraph::add_deact_edge` in `src/search/graph.cpp` (lines 346-366) stores rows on both sides. It copies the notes from the 500 ms before D, which `set_head_time` and `is_recent_to_head` keep (lines 280-297). A note exactly on D is included at offset 0. `store_new_backend` (lines 223-249) then adds notes up to 500 ms after D. `Activation::display_backends` in `src/core/model.cpp` (lines 579-596) shows rows within 500 ms either side. `docs/UserGuide.md` line 122 matches this ("the notes just before and after it"). ADR 0011 line 19 does not: "A backend row only exists when some note lands within 500 ms after D."

| Activation's notes near D | ADR 0011 | Code |
|---|---|---|
| One note 200 ms before D, none after | no row | row at -200 |

They disagree today, shown by reading. Nothing on screen changes.

The sentence sits in the ADR's "What went wrong" story, not in its decision. It was already inaccurate when written in 3859380 (2026-09-03), because `recent_backends_` dates from 3b69958 (2026-08-15). The graph owns the rule. ADR 0011 needs a correcting note.

*Owner proposed: ScoreGraph::add_deact_edge (src/search/graph.cpp); ADR 0011 needs a correcting note. Candidates: tempo-22.*

#### 49. Multiplier squeeze check uses its own combo list and misses 4- and 5-note chords

The question is: does a chord straddle a multiplier step? A step is where the multiplier goes up, at combo 10, 20 or 30. `to_multiplier` in `src/core/timing.cpp` (line 9) owns those steps. `category_scores` in `src/core/scoring.cpp` (lines 41-42) pays note i at `to_multiplier(combo+1+i)`, always in the best order. `MultSqueeze::applies` in `src/core/model.cpp` (line 345) answers the same question its own way: a fixed list of combos 7, 8, 17, 18, 27, 28, then a mod-10 rule. A test in `tests/test_model.cpp` (around line 405) derives the real rule from `to_multiplier` and pins the gap as it is.

| Chord size | Straddles a step (`to_multiplier`) | `applies` accepts |
|---|---|---|
| 2 | 8, 18, 28 | 8, 18, 28 |
| 3 | 7, 8, 17, 18, 27, 28 | same |
| 4 | 6, 7, 8, 16, 17, 18, 26, 27, 28 | 7, 17, 27 |
| 5 | 5-8, 15-18, 25-28 | none |

They disagree today. The verifier showed it by reading and by running a Python port of both rules. Take kick, red, yellow cymbal and blue cymbal at combo 8. Its notes are paid x1, x2, x2, x2, so the order matters. But `applies` says no, because (4+8) % 10 = 2.

The total score is still right. The Paths tab Multiplier squeeze list, its summary and the verbose path string leave these squeezes out.

The comment above `applies` (lines 336-344) says note i scores at `to_multiplier(combo_ + i)`. It lost the plan's "(from 1)", so read the usual C++ way (counting from 0) it is one note off. No code follows it. It came in with c89596b (2026-09-25).

`to_multiplier` should own this. `applies` should test `to_multiplier(combo+1) < to_multiplier(combo+n)`. Any 4- or 5-note limit should then be its own named rule. That also makes the comment unnecessary.

The combo list came from 3b69958 (2026-08-15, the C++ port). User decision 22 in `docs/superpowers/plans/2026-09-24-derivation-fixes.md` says the combo set stays fixed. That decision came before the 4/5-note mismatch was found. The agent run in c89596b kept the set and marked its reason "not recorded". The verifier found no ADR, no CONTEXT.md entry and no user quote approving the 4/5-note gap itself.

*Owner proposed: to_multiplier in src/core/timing.cpp (MultSqueeze::applies should ask it). Candidates: score-1, score-3.*

#### 50. Shown multiplier-squeeze points are too low on 4-note chords

The question is: how many points does hitting a multiplier-squeeze chord in the right order win? `category_scores` in `src/core/scoring.cpp` (lines 28 and 41-42) sorts the notes by value and pays each one at its own multiplier, so it always pays the best order. `MultSqueeze::points` in `src/core/model.cpp` (line 371) says the gain is the highest note value minus the lowest. `build_multsqueezes` in `src/app/path_view.cpp` (around line 145) shows that number as "(+N pts)", and `multsqueeze_summary` adds them up.

| Chord (sorted n1..n4) | Real gain (best minus worst order) | `points()` |
|---|---|---|
| 2 or 3 notes | high minus low | high minus low |
| 4 notes at combo 7, 17, 27 | (n3+n4) - (n1+n2) | n4 - n1 |

`points()` assumes only one note crosses the step. A 4-note chord at combo 7 has two notes crossing. They disagree today, shown by hand and by a Python port the verifier ran. Kick 50, red 50, yellow cymbal 65 and blue cymbal 65 at combo 7 are paid x1, x1, x2, x2. The best order is worth 360 and the worst 330, a gain of 30. `points()` says 15. Any accepted 4-note chord where n2 differs from n3 shows the gap.

On screen, the Paths tab Multiplier squeeze fold shows "(+15 pts)" where the order is worth 30. The fold's summary total is low by the same amount. The howto text names one edge note when two must be hit last. The total score is right.

`category_scores` should own the gain, as best order minus worst order from its own per-note pricing. This came from 3b69958 (2026-08-15, carried over from the Python original). No user decision was found in `docs/adr/`, the CONTEXT.md "Multiplier squeeze" entry, or the plan headers in `docs/superpowers/plans`.

*Owner proposed: core/scoring (category_scores per-note pricing). Candidates: score-2.*

#### 51. Leaderboard page can show 100.00% for a score below optimal

The question is how the dmleaderboards page rounds a score's "% of optimal".

It rounds twice. `collect_dm_rows` in `src/app/dm_report.cpp` (line 203) computes the exact percentage once. `build_dm_html` (line 234) then writes it into the page data with 4 decimal places (`%.4f`). The page script `kPageJs` (line 109) rounds that again to 2 places with `toFixed(2)`. The average stat (lines 123-125) also averages the 4-place values and rounds again.

| True value | Rounded once to 2 places | Via 4 places, then 2 |
|---|---|---|
| 99.99495 (198,010 of 198,020) | 99.99% | 100.00% |
| 99.994951 | 99.99% | 100.00% |

The two paths disagree today for any true value from 99.99495 up to just under 99.995. The verifier showed this with a run (a Node script in the scratchpad, `dmpct.js`).

On screen, a real score 10 points short of optimal reads "100.00%" of optimal. That looks like a perfect run when it is not.

`collect_dm_rows` should own the one rounding. The page data should carry either the full number or the final string, never a half-rounded value. This came in with 21ddf53 (2026-08-18). No user decision was found in `docs/adr/`, `CONTEXT.md` or the UserGuide's dmleaderboards section.

*Owner proposed: collect_dm_rows in src/app/dm_report.cpp. Candidates: display-9.*

#### 52. Preview can light the wrong offered fill under the 1.0 fill rule

The question is which fills the game showed you before each activation. A shown fill you played through is called "offered"; the one you activated on is "taken"; the rest are "hidden".

Two places answer it. `Engine::branch_activate` in `src/search/engine.cpp` (around lines 531-585) decides fill by fill. A fill counts as a skip only if SP is at least 2 bars and the fill's deadline is no more than 60 ms before SP was ready. The engine stores only the skip count, not which fills. `build_preview_scene` in `src/app/preview_view.cpp` (lines 338-352) then guesses: it marks the `skips` fills nearest before the activation as Offered.

| Fill | Engine | Preview |
|---|---|---|
| A (short, deadline 18968.75 ms) | offered, counted as the skip | Hidden |
| B (long, deadline 18768.75 ms) | refused, no skip | Offered |

They disagree today. The verifier showed it by reading plus arithmetic, not a corpus run: 120 BPM, CH 1.0 rule, fill A ends at 20000 ms (500 ms long), fill B ends at 26000 ms (3600 ms long), SP ready at 18900 ms, activation at a later fill C. Under the 1.0 rule a long fill can have an earlier deadline than a short fill before it. Under 1.1 deadlines rise with fill order, so the two always agree. The finder's probe saw 0 mismatches in about 935 candidates, so the case is rare.

On screen, with "1.0 fills" on, the dim offered fill sits on a fill the game never showed, and the one it did show stays hidden. The nearest-n guess came in at 133b52e (2026-08-28). It reached the screen when 1f3efdf (2026-09-29) added the 1.0 fills setting. CONTEXT.md "Path overlay" says offered fills come from the skip count, but no decision covers the nearest-n mapping.

The engine should own it. It already knows which fills it charged, so it should store their ticks on each activation, the way `collected_phrase_ticks` is stored. The Preview would read them instead of guessing.

*Owner proposed: Engine::branch_activate in src/search/engine.cpp (stamp the skipped fills' ticks on each Activation). Candidates: fills-1, screenB-6.*

#### 53. Generated fill length reads the meter one change late when two meter changes fall between chords

The question is which time signature (meter) is in force just before a chord. The answer sets the length of a generated fill. Generated fills are the ones Hydra makes for charts that have no authored fills. The length is that meter's ticks per measure times `fill_length_measures`, which is half a measure.

Two places answer it. The timing module's `MeasureIndex::section_at`, with `tpm_at`, in `src/core/timing.cpp` (around lines 109-114) looks the meter up directly. `Song::check_activations` in `src/parse/song.cpp` does not call it. It runs its own walk (lines 222-247), ported from Python's `SongIter`. That walk moves forward at most one meter change per chord. The fill length is then written at line 281-282 from the walk's `pre_tpm`.

| Meter changes between two chords | Own walk | `section_at` |
|---|---|---|
| none | same | same |
| one, even exactly on the chord tick | same | same |
| two or more | one change behind | current |

On an exact change tick, both use the meter from before the tick, so that edge agrees.

They disagree today. Two runs of a Python port of both rules showed it, and I re-read the C++ walk and it matches. First input: resolution 480, meter map {0: 1920, 1920: 1440, 3360: 2400} ticks per measure, chords at ticks 0 and 4000. The walk gives 1440 and `section_at` gives 2400. That is a 720-tick fill instead of 1200. Second input: meter changes at 1000 and 2000 (1920, then 1440, then 2400), chords at 0, 500, 3000 and 3500. At 3000 the walk gives a stale 1440 and `section_at` gives 2400. Move the third chord to 2000 and they agree again. Which answer Clone Hero uses is not established. No test pins the case.

On screen, that generated fill is drawn at the wrong length in the Preview, since `build_preview_scene` spans it from `activation_length`. The fill's deadline moves too. One finding named the CH 1.1 rule and the other the 1.0 rule. Both are right: `activation_fill_deadline_ms` in `src/search/graph.cpp` reads the fill length under both rules. The default 1.1 rule is the one the GUI uses. So the E label can change, and so can whether an activation is legal and which path wins.

`MeasureIndex::section_at` should own this. It is already the one place that answers "what meter is in effect at this tick". The same walk also steps through the tempo list (`bpm_pos`) and never reads the result. Its own comment says the value is unused. That dead walk should go too.

The walk came in with the C++ port checkpoint 3b69958 (2026-08-15), copying Python's `SongIter`. No user decision was found in CONTEXT.md, docs/adr/, the plans or the port commit message.

*Owner proposed: MeasureIndex::section_at in src/core/timing.cpp, reached through Song::timing(). Candidates: fills-10, tempo-2.*

#### 54. Four readers interpret the fill-rule stamp four ways, and the migration spells "ch10" itself

The question is which fill rule a database's results hold. The fill rule is Clone Hero 1.0's or 1.1's timing for when drum fills appear.

The answer is stored twice. Each result row carries it in its key, `Lens::legacy_fills` in `src/store/record_store.h` (line 101). The whole file also carries an `engine_mode` stamp, whose text is owned by `engine_mode_stamp` in `src/search/graph.h` (lines 41-43). That function returns "ch10" or "ch11".

Four places read the stamp, each its own way. `RecordStore::add_fill_rule_column` in `src/store/record_store.cpp` (line 635) is the schema 3 migration. It compares the stored mode with the literal "ch10" instead of calling `engine_mode_stamp`. `main` in `src/cli/batch.cpp` (lines 156-179) calls the owner and refuses a run whose rule differs; it treats an unstamped file with rows as ch11. `main` in `src/cli/report.cpp` (line 85) forces the 1.0 lens on a ch10 file. `main` in `src/cli/fillcompare.cpp` (lines 86-99) only warns.

| Stamp | Migration | hydra_batch | hydra_report | hydra_fillcompare |
|---|---|---|---|---|
| ch10 | 1.0 | 1.0 | 1.0 forced | warns if used as --new |
| ch11 | 1.1 | 1.1 | INI setting | warns if used as --old |
| none, has rows | 1.1 | 1.1 | INI setting | silent |

The migration's literal agrees with the owner today, since both spell "ch10". If the stamp text were ever renamed, the migration would stop recognising old 1.0 databases. An old `--legacy-fills` database would then show its 1.0 scores as 1.1 results.

The readers disagree with each other today, shown by reading. Case one: an unstamped file with results is passed as `--old`. Batch and the migration call it 1.1, fillcompare says nothing, and its 1.0 side comes back empty. Case two: hydra_batch stamps `hydra.db` ch11, then the GUI adds 1.0 rows. Running `hydra_fillcompare --old hydra.db --new hydra.db` prints "Warning: hydra.db is stamped engine_mode=ch11, not ch10". Yet the file's own header comment says one file may serve both sides, and the 1.0 rows are found correctly.

The user sees contradictory tool messages and, in case one, a silently empty 1.0 column. The batch refusal text (line 169) also still says the rule is not stored on each result. That has been stale since schema 3.

The store should own one function that turns the stamp into a fill rule, using `engine_mode_stamp` for the text. The other choice is to have the checks ask the rows. The store does not include search headers today, so the fix needs that include or a store-level constant the owner re-exports.

The literal came in at 1f3efdf (2026-09-29), with the "1.0 fills" checkbox. ADR 0010's 2026-09-29 note records the user choosing the row key. It decides that a file stamped ch10 migrates as 1.0, and keeps the batch guards "because the request was for the GUI only". Nothing decides the stamp's spelling, fillcompare's warning, or the unstamped `--old` case.

*Owner proposed: store: one function mapping the engine_mode stamp (and row count) to a fill rule, built on engine_mode_stamp in src/search/graph.h; or checks that ask Lens::legacy_fills on the rows. Candidates: fills-2, fills-4, screenB-15, store-11.*

#### 55. Each fill rule's name and explanation is typed separately in about eight places

The question is what each fill rule is called on screen, and how it is explained. A fill rule is the Clone Hero 1.0 or 1.1 way of deciding when a drum fill can appear. Under 1.1 your Star Power must be ready 4 beats before the fill. Under 1.0 it must be ready about one fill-length before.

Only two things have an owner. The rule itself is computed by `activation_fill_deadline_ms` in `src/search/graph.cpp` and documented at `FillDeadlineRule` in `src/search/graph.h`; CONTEXT.md (lines 114-123) and ADR 0010 define it. The database stamp ("ch10"/"ch11") comes from `engine_mode_stamp` in the same header. Every display name and every prose explanation is typed by hand.

The names live here. `src/cli/batch.cpp` has a `rule_name` lambda at line 163 for its database-mismatch message, and a separate settings printout at line 203. `batch_settings_summary` in `src/ui/library_dialogs.cpp` (line 84) writes the GUI batch confirm's Fills line. `src/app/report.cpp` (line 400) adds a suffix to the text report. `src/app/fill_report.cpp` names the rules in its title (line 26), its heading (line 30), its "Only in 1.0 db" / "Only in 1.1 db" options (lines 48-49) and its column headers (lines 79-83). The checkbox in `render_sp_cap` in `src/ui/settings_bar.cpp` (line 97) and a disabled-button tooltip in `src/ui/library_toolbar.cpp` (line 118) name them again.

| Place | Name for the 1.0 rule |
|---|---|
| GUI batch confirm (library_dialogs.cpp 84) | Clone Hero 1.0 |
| hydra_batch mismatch message (batch.cpp 163) | Clone Hero 1.0 |
| hydra_batch settings printout (batch.cpp 203) | Clone Hero 1.0 (legacy) |
| text report (report.cpp 400) | Clone Hero 1.0 fills |
| fill report title and columns | CH 1.0 |
| settings checkbox, toolbar tooltip | 1.0 fills |

The names already differ today, shown by reading. Some of that may be deliberate wording per surface. But `hydra_batch` prints "Clone Hero 1.0 (legacy)" a few lines after printing "Clone Hero 1.0" for the same rule, and the GUI confirm says "Clone Hero 1.0" where the CLI says "(legacy)".

The rule is also explained in prose twice. The help tooltip in `render_sp_cap` (settings_bar.cpp lines 99-102) and the footer of `generate_fill_report` (fill_report.cpp lines 288-294) each retell it.

| Source | 1.1 | 1.0 | 250-10000 ms clamp |
|---|---|---|---|
| Code | 4 beats before fill start | fill start minus (fill length + 1/16 beat) | yes |
| Settings tooltip | 4 beats before | about one fill-length before | not mentioned |
| Fill report footer | a flat 4 beats | about one fill-length before | not mentioned |

The two explanations agree with the code in substance, shown by reading. Leaving out the clamp is fine for a summary. If the rule changed, both texts would go stale and the user would read an explanation that no longer matches the scores.

On screen, the user sees slightly different names for the same rule depending on where they look. No score is affected. There is also a test gap. The test "the confirm lists the settings a batch runs with" in `tests/test_batch_text.cpp` checks difficulty, cap, score range and path limit, but never the Fills line, so a wrong Fills line would not be caught.

`src/search/graph.h` should own one function that returns each rule's name and its one-line description, because that header already owns the rule and its stamp. The confirm-line test should then check the Fills line. ADR 0010 names the GUI setting "1.0 fills" but fixes no other wording. The fill report footer came in 406d473 (2026-08-31); the settings tooltip and the batch confirm line came in 1f3efdf (2026-09-29).

*Owner proposed: One fill-rule label and description function in src/search/graph.h, beside engine_mode_stamp and FillDeadlineRule. Candidates: display-23, fills-21, sweep-tests1-c16.*

#### 56. An out-of-range score-range setting (depth_mode) is read three different ways, and batch runs carry the result key twice

The question is what the stored `depth_mode` number means. 0 means "within N scores" and 1 means "within N points".

Five places answer it. `Settings::to_analysis_settings` in `src/app/config.cpp` (line 150) tells the search: 1 is points, anything else is scores. `within_label` in `src/app/path_view.cpp` (lines 436-442) uses the same test for the Paths tab heading. `batch_settings_summary` in `src/ui/library_dialogs.cpp` (lines 85-86) uses the opposite test: 0 is scores, anything else is points. (The chart-mode part of that dialog, lines 79-81, is fine; it reads the owner's predicates and only formats them.) `render_score_range` in `src/ui/settings_bar.cpp` (around line 123) indexes a two-item combo box. `Settings::lens` (around line 169) passes the raw number into the result key through `Lens::from` in `src/store/record_store.h`. Behind all of them, `Settings::load_file` (line 74) reads the INI value with a bare `atoi` and no range check.

| depth_mode | Search | Paths tab | Batch summary | Combo | Result key |
|---|---|---|---|---|---|
| 0 | scores | scores | scores | scores | 0 |
| 1 | points | points | points | points | 1 |
| 2 or -1 | scores | scores | points | blank | 2 or -1 |

They agree for 0 and 1. A hand-edited `hydra_settings.ini` with `depth_mode=2` splits them; the combo has only two items, so the UI cannot produce it. This was shown by reading, not by a run.

On screen, the batch confirm says "N points" while the search ran by scores. The results are filed under key 2. Picking "scores" afterwards gives key 0, so those songs read as not analyzed even though the search was identical. A smaller gap sits in the same labels: depth 1000 shows "1,000 scores" in the batch confirm but "Within 1000 scores" on the Paths tab (the grouping rule itself is covered by the count-with-noun finding).

A second, related copy concerns the result key itself. The key is a "lens", meaning the settings part of a stored result's address. `struct BatchRun` in `src/app/analysis.h` (around line 137) holds the lens and the search settings side by side. `Settings::batch_run` in `config.cpp` (around line 179) builds both from one `Settings`, so production always agrees. `run_batch` in `src/app/analysis.cpp` searches with one and files under the other, and never checks that they match. Three hand-built test runs already mismatch: `tests/test_analysis.cpp` around lines 269 and 316, and `test_run` in `tests/test_library_jobs.cpp` around line 60. They file under depth 0 but search with depth 4. Those tests only check cancelling, so nothing breaks.

`Settings` should own the meaning. It should normalize the value on load and expose one accessor that turns the number into a `DepthMode`; the search, the result key and every label should read that, and one label helper should build the "scores"/"points" text. `run_batch` should derive the lens from its own settings, so a hand-built run cannot drift. The batch summary copy came in with a591bba (2026-09-27). No user decision was found in `docs/adr/` (ADR 0009 covers keying by score range, not validation), `CONTEXT.md`, or the commits 515bf37 (2026-08-22), 133b52e (2026-08-28), 577ddc2 and a591bba (both 2026-09-27). No ADR or CONTEXT.md entry covers `BatchRun`; its "for tests" note is only a code comment.

*Owner proposed: app::Settings in src/app/config.cpp: normalize depth_mode on load and expose one accessor and one label helper; run_batch derives its lens from its own settings. Candidates: fills-12, store-2, store-25, sweep-tests1-c6.*

#### 57. Preview time box picks BPM by milliseconds but meter, section and drain rate by the rounded tick

The question is which tempo, meter and section are in force at the Preview playhead. The boxes on the Preview answer it two different ways.

First, the playhead is turned into a whole tick by rounding to the nearest one. A tick is the chart's smallest time step. `tick_at` in `src/app/preview_view.cpp` (around lines 427-430) rounds and clamps at 0. `build_time_box`, `build_drain_box` and `step_tick_ms` all use it. `build_activations` in `src/app/path_view.cpp` (around line 361) inlines the same rounding for the timeline end, without the clamp. Those two rounding copies agree for any time at or after 0.

The split is inside the time box. Its BPM line (`build_time_box`, around lines 463-467) loops over the tempo list by milliseconds. It takes the last tempo whose ms is at or before the playhead. The time signature and section lines in the same function (around lines 471-484) take the last change at or before the rounded tick instead. `build_drain_box` (around lines 526-527) asks `SongTiming::ms_per_measure_at` on the same rounded tick, which reads the engine's `MsIndex::tps_at` in `src/core/timing.cpp`.

| Playhead | BPM line | Signature, section, drain rate |
|---|---|---|
| more than half a tick before the change | old | old |
| within half a tick before the change | old | new |
| on or after the change | new | new |

They disagree today. One finder showed it with a Python port of the code. Take resolution 480 and 120 BPM changing to 180 BPM at tick 960 (1000 ms). At 999.9 ms the BPM line reads 120.000, while the drain box reads "1 bar / 2.7 s", the 180 BPM value. A real-chart example (My Own Summer) was named but not reproduced. The 5-tick step keys and the "< Act" jumps always land exactly on a tick, so only playback or scrubbing reaches the half-tick window.

On screen, for up to half a tick before a change, the time box shows the next tick's meter or section, and the drain box shows the new rate, while the BPM line still shows the old tempo. The drain-box spec and its code comment both say the boxes behave the same, so this contradicts the stated intent.

The engine's tempo rule is tick-based, so `MsIndex::tps_at` should answer the BPM question too. The time box should ask it on the tick it already computes. `SongTiming` should also own the one ms-to-display-tick function, so `preview_view` and `path_view` stop rounding separately. `tick_at` and the ms loop came in with cbb72aa (2026-08-31). The tick loop came in with 34b2e51 (2026-09-27), and the path_view copy with 577ddc2 (2026-09-27). This was an assumption, not a request. No user decision on rounding versus flooring, or on ms versus tick lookup, was found in those commit messages, docs/adr/ or CONTEXT.md.

*Owner proposed: SongTiming / MsIndex::tps_at: one ms-to-display-tick function, and BPM read on that tick. Candidates: screenB-3, tempo-1, tempo-18, tempo-27.*

#### 58. Two measure-position functions read a meter-change tick differently

What decimal measure position does a tick have? Two functions in `src/core/timing.cpp` answer, each with its own section lookup.

`Timecode::Timecode` (around lines 118-141) uses `MeasureIndex::section_at`. That reads a tick sitting on a meter change as the earlier section. `SongTiming::measures_at_tick_f` (around lines 206-215) scans on its own and reads that tick as the new section. Its inverse, `tick_at_measures_f`, follows the same new-section rule.

| Tick, meter map {0: 1920, 2400: 1440} | `Timecode` | `measures_at_tick_f` |
|---|---|---|
| 2399 | 1.24948 | 1.24948 |
| 2400 (the change) | 1.25 | 1.3333 |
| 2401 | 1.33403 | 1.33403 |

They disagree today, but only on the exact tick of a mid-measure meter change. This was shown by a run of a port. At changes that fall on a barline they match.

The engine measures SP length with `Timecode`. The SP gauge curve in `build_sp_meter_curve` (`src/app/preview_view.cpp`, around line 158) uses the other copy. So does the transfer-scale map in `SongTiming::sp_end_ms`. A mid-measure meter change inside an SP window would show as a kink in the gauge's drain line. The end point stays right, because the last stretch is forced to empty at the deact node.

`MeasureIndex::section_at` should own the section rule, so both functions share one boundary side. Introduced by 7d846b9 (2026-08-19, the exact squeeze solver). No user decision was found: CONTEXT.md's "Transfer scale" entry does not pick a side, and the choice lives only in code comments.

*Owner proposed: MeasureIndex::section_at (measures_at_tick_f should call it with the side made explicit). Candidates: tempo-4.*

#### 59. play_chart.py keeps its own chart parser, MIDI reader and tempo math, and disagrees with Hydra

The question is which notes a chart holds, in which lane, and at what time. Hydra answers it in three layers. `hydra::MidiFile` in `src/parse/midi.cpp` (lines 205-270) reads the MIDI bytes. The parser in `src/parse/song.cpp` (`ChartParser::optype` around line 856, `MidiParser::optype` around line 392, `MidiParser::parse` around line 599) decides notes and lanes. `MsIndex` in `src/core/timing.cpp` turns ticks into time.

The live auto-play tool `tools/ch_probe/experiments/play_chart.py` answers all three again on its own. It has its own .chart parser (`parse_chart`, line 89, and `chart_notes_to_lanes`). It has its own MIDI reader (`parse_midi` from line 173, `_read_vlq`, `midi_notes_to_lanes` and `main`). It walks the tempo map itself in `ticks_to_seconds` (lines 290-303).

The tempo formula itself is the same piecewise formula, so on a well-formed .chart the times agree. Everything else can differ.

| Input | Hydra | play_chart.py |
|---|---|---|
| Gem inside a MIDI tom-marker span, after its first tick | tom | cymbal |
| Pad inside an Expert disco section (pro) | flipped | not flipped |
| Flam marker (109) on one pad | two notes | one note |
| MIDI 95 with 2x off | dropped | kick |
| `.chart` `N 5` | no note | green |
| Folder with notes.mid and notes.chart | notes.mid | notes.chart |
| `PART DRUMS_2X` listed before `PART DRUMS` | `PART DRUMS` | `PART DRUMS_2X` |
| Track: note 96 at 0, sysex `F0 04 01 02 03 F7` at +48, note 97 at +48 | note 97 at tick 96 | note 97 at tick 106 |
| `.chart` with no Resolution line | refuses to load | uses 480 ticks per beat |
| No tempo at tick 0 | refuses to load | plays at 120 BPM |

They disagree today. Runs showed 282 of 772 ticks differ on Band Like That at Expert pro. They showed 226 of 294 differ on `testdata/input/test_flammarker/flammarker_ingame.mid`. The tom-span and track-name rows were shown by runs on synthetic files. The sysex row was shown by a run of play_chart.py's own functions, loaded unchanged, on a built two-track file. The other rows were shown by reading.

The sysex row needs a word. Running status is MIDI's shorthand where a message reuses the previous status byte. A sysex is a vendor-specific message with its own length field. Hydra keeps running status through meta events and skips a sysex by its length; `tests/test_midi.cpp` (line 138) pins that skip. play_chart.py treats 0xF0 as a normal channel status with no length skip. So the sysex data bytes are read as delta times, which gives the 10 extra ticks. It also mishandles running status in the tempo track, and takes the track name from the first 200 bytes.

The tool also applies no chart Offset or song.ini delay. Whether Hydra's note times include that offset was not established.

Nothing changes on a Hydra screen. The auto-player plays a different note set, at different times, from the one Hydra scored. On any chart whose drum track carries a sysex, it presses every later note late. So probe scores can differ from Hydra's optimal for reasons that have nothing to do with the thing being probed. Two other scripts named by one finder, `probe_chart.py` and `probe_songs.py`, write their own test charts and are not copies.

Hydra should own all three layers. `hydra_replay` already prints each chord's tick, ms and `chord_code` in its JSON dump (`tools/replay.cpp`, line 372), and reports the notes path it used. The tool should read that instead of re-parsing the chart.

The script arrived in 283e028 (2026-09-26), committed as-is. No decision was found in docs/adr/ or CONTEXT.md.

*Owner proposed: Hydra's parser (src/parse/song.cpp, hydra::MidiFile) and MsIndex, consumed through hydra_replay's per-chord JSON dump. Candidates: parse-2, parse-3, parse-4, parse-5, sweep-tests2-c3, tempo-13.*

#### 60. Blank artist or charter shows blank for .ini/.sng but placeholder for .srb

The question is what a chart's artist and charter are when the metadata leaves them out or blank.

Three readers in `src/app/analysis.cpp` answer it, each with its own copy of the `<unknown artist>` and `<unknown charter>` literals. `read_metadata_ini` (around line 173) and `parse_sng_metadata` (around line 201) let an empty value overwrite the placeholder. `parse_srb_metadata` (around line 226) keeps the placeholder when the value is empty. The title, by contrast, has one owner: `title_or_unknown` in `src/parse/song.cpp` (around line 188), applied once in `discover_charts`.

| Artist in metadata | song.ini | .sng | .srb |
|---|---|---|---|
| key missing | `<unknown artist>` | `<unknown artist>` | `<unknown artist>` |
| present but empty | blank | blank | `<unknown artist>` |
| "X" | X | X | X |

They disagree today, shown by reading. An .srb always carries the field, so for .srb "empty" and "missing" are the same case.

On screen, the library Artist and Charter columns and the reports show a blank for some charts and `<unknown artist>` for others in the same situation.

The literals come from Python Hydra (f3b0e36, 2024-12-24) and were ported in 3b69958. The .srb empty check came with 21ddf53 (2026-08-18). No user decision was found in CONTEXT.md, docs/adr/, or plan 2026-09-24, which gave only the title an owner. Artist and charter should get the same kind of single owner as the title.

*Owner proposed: artist_or_unknown / charter_or_unknown beside title_or_unknown in src/parse/song.h, applied once in discover_charts. Candidates: parse-6.*

#### 61. A container's notes entry is recognised by name in .sng but by extension in .srb

The question is what chart format a container's notes entry has. A container is a single-file song (.sng or .srb) holding the notes and audio inside it.

Two places answer it, both in `src/parse/song.cpp`. `load_songpath_sng` (around line 1101) uses `notes_file_format`, which accepts only the exact names `notes.mid` and `notes.chart` in any case. `load_songpath_srb` (around lines 1137-1144) uses `chart_format_of` on the file extension, and falls back to sniffing the `MThd` MIDI header bytes.

| Entry name | .sng | .srb |
|---|---|---|
| `notes.mid` | MIDI | MIDI |
| `song.mid` | not a notes entry | MIDI |
| `x.bin` with MIDI bytes | ignored | MIDI (sniffed) |

They disagree today on non-standard names, shown by reading. No real container with such names was found; the bundled .srb files use the standard names. The questions only partly overlap. A .sng must pick one entry among many, while an .srb's notes are always stream 2. The overlap is the format decision.

On screen, a .sng whose notes entry is not named `notes.*` fails with "No chart files found in SNG file.", while an equivalent .srb loads.

The finder attributes this to 21ddf53 (2026-08-18); the `MThd` sniff also traces to Python-era commits. No user decision was found in CONTEXT.md, docs/adr/, or plans 2026-09-24 and 2026-09-26. `chart_files.cpp` already owns "which format is this file", so it should own this rule too.

*Owner proposed: src/parse/chart_files.cpp (one 'container entry name -> chart format' rule). Candidates: parse-12.*

#### 62. songmeta keeps one song length per chart, overwritten by whichever difficulty was analyzed last; backfill only fills gaps

The question is how long the song is for the record being viewed, and which write wins when two writes give a length. "Length" here means the onset time of the last note.

The formula has one owner: `store::song_length_ms` in `src/store/record_store.cpp` (around line 462). It works from a song parsed at one difficulty.

Two writes store the result, with opposite rules. `RecordStore::upsert_song` (lines 861-876) is called from `save_analysis` and `add_song`. Its `COALESCE(excluded.length_ms, songmeta.length_ms)` keeps the newest non-null value, so every new analysis overwrites the old length. Its comment says this is on purpose: "a new analysis may be of another difficulty". `RecordStore::set_song_length` (lines 851-859) writes only when the stored length is missing. It is called only from `update_song_length` in `src/ui/app_state.cpp`, fed by `SongLengthJob::run` in `src/ui/song_length_job.cpp`, which also parses the current difficulty.

| Stored | Incoming | `upsert_song` keeps | `set_song_length` keeps |
|---|---|---|---|
| 200000 | 201000 | 201000 | 200000 |
| 200000 | none | 200000 | not called |
| none | 201000 | 201000 | 201000 |

The first row is pinned by `tests/test_store.cpp` (lines 1870-1871). In the app the two writes rarely meet, because the backfill job only starts when the length is missing.

The real gap is the key. `songmeta` has one row per hyhash (the chart's content hash), not per difficulty. `RecordStore::get_record` (around line 1216) hands that one length back with every difficulty's record. `build_activations` in `src/app/path_view.cpp` (around lines 206-208 and 360) divides by it to place activation dots on the Paths timeline.

| Last analysis | Length the Expert record uses |
|---|---|
| Expert | Expert's last note |
| Easy or Hard | that difficulty's last note |

They disagree today. This was shown by reading plus a scratch scan of a real chart. In `testdata` IB24/T6 "Levitating" notes.chart, Expert's last drum note is at tick 170760 and Easy's is at 149160. Analyze Expert, then Easy, then open Expert. The Expert timeline now ends at Easy's last note, and every Expert activation after tick 149160 piles up at the right edge.

The members disagree on which commit brought the overwrite rule. Git shows the `COALESCE` line and its "another difficulty" comment arrived in bc01fa6 (2026-09-27, "each result keeps its best path's star count"). f1c6a3d (same day) is the UI-redesign plan and mockup docs, not the code change. The fill-only-if-missing rule came in with bbf9345 (2026-09-27), whose commit message is agent-written.

No user decision was found in the 2026-09-26 and 2026-09-27 plan decision lists, CONTEXT.md or docs/adr. The user should decide whether length is kept per song or per difficulty.

`RecordStore` should own one write rule. `song_length_ms` stays the formula. The stored length should be keyed by chartmode, or kept on the result row, so each record reads its own last note.

*Owner proposed: RecordStore write policy in src/store/record_store.cpp; store::song_length_ms keeps the formula, and the stored length is keyed per chartmode or kept on the result row. Candidates: parse-17, store-5, store-6, sweep-tests3-c12.*

#### 63. Duplicate chart copies are named by the first copy at scan, the last saved after analysis

The question is which copy names a chart when the same notes file sits in two folders. The two copies share one md5 (a fingerprint of the file), so they share one `songmeta` row.

Two rules answer it. `RecordStore::rebuild_chart_library` in `src/store/record_store.cpp` (around lines 1634-1641) takes the names from the first copy the scan listed. `RecordStore::upsert_song` (around lines 871-876) overwrites the names with whichever copy was saved last. That runs from `AppState::store_finished_analysis` in `src/ui/app_state.cpp` (around line 534) and from `run_batch` in `src/app/analysis.cpp`. `run_batch` (around lines 548-555) also never removes duplicate md5s from one run, so it analyzes the same chart once per copy.

| Step | Name shown |
|---|---|
| After a scan | A (first listed) |
| After analyzing copy B by hand | B |
| After a batch | A or B, by worker finish order |
| After the next rescan | A again |

This was shown by reading, not by a run. Take two byte-identical notes.mid files whose song.ini names are "A" and "B".

On screen, reports and the dmleaderboards page show such a chart under a name that flips between scans and analyses. A batch also wastes time on the second copy.

The first-copy rule came in with 3e5d634 (2026-09-26). User decision 1 in `docs/superpowers/plans/2026-09-26-codebase-audit-fixes.md` says names follow song.ini. It does not cover which duplicate wins.

`RecordStore` should own one rule that both functions call. `run_batch` should dedupe by md5.

*Owner proposed: RecordStore (src/store/record_store.cpp): one 'which copy names an md5' rule; run_batch should dedupe its todo list by md5. Candidates: parse-16.*

#### 64. A late dynamics tag leaves early notes plain while the tab says dynamics are on

The question is whether a MIDI chart has dynamics turned on. Dynamics means ghost and accent notes, which score differently. The `[ENABLE_CHART_DYNAMICS]` text event turns them on.

Two answers exist in `src/parse/song.cpp`. `MidiParser::op_enable_dynamics` (around line 342) sets the flag at the event's tick. `MidiParser::op_note` (around line 369) prices each note by the flag as it stands at that note's tick. `MidiParser::parse` (around line 619) then reports the final flag for the whole chart. The Dynamics tab shows that chart-level flag as "Dynamics enabled: yes".

| Tag position | Notes before the tag | Chart flag |
|---|---|---|
| tick 0 | priced | yes |
| same tick as a note | that note priced | yes |
| after some notes | counted as Normal | yes |
| none | all Normal | no |

They disagree today, shown by reading. Put a velocity-127 red at tick 0, the tag at tick 480, and another at tick 960. The first note is Normal, the second is an Accent, and the tab says enabled.

On screen, the Dynamics counts and the score treat early ghosts and accents as plain notes while the tab says dynamics are on. Whether that score is wrong depends on Clone Hero's behaviour, which nobody has checked.

The per-note gate came from 3b69958 (2026-08-15). The chart flag came from ec3ade9 (2026-09-21). ADR 0012 says the tag gates the whole thing, but not from which tick. No user decision was found.

`MidiParser` should decide once per track and use that one answer for every note and the flag. A Clone Hero check comes first.

*Owner proposed: MidiParser (src/parse/song.cpp): decide 'enabled' once per track, after a Clone Hero ground-truth check. Candidates: parse-19.*

#### 65. Results made under other rules are deleted, though the docs promise they come back

The question: if you analyze a song under one set of rules, then change the rules, does the old result survive until you switch back?

The docs say yes. ADR 0014 (around line 38) says such a record "is not deleted" and reads Ready again when the rules are switched back. `docs/UserGuide.md` (line 61) says switching the rules back "brings the result back". User decision 15, in the header of `docs/superpowers/plans/2026-09-24-derivation-fixes.md`, says the same.

The code says no. `RecordStore::write_row` in `src/store/record_store.cpp` deletes old rows twice. Purge 1 (line 937) removes every row of that chart and mode that is not Ready, at any SP cap or lens (the settings bundle a result is filed under). A row made under other rules counts as not Ready. Purge 2 (line 948) removes the row with the same key. The unique key has no rules fingerprint (a hash of the rules file), so an A-rules row and a B-rules row cannot sit side by side.

| Under rules B, was this chart written? | Docs say on return to A | Code gives |
|---|---|---|
| No | Ready | Ready |
| Yes, any cap or lens | Ready | Not analyzed |

This was shown by a run. The verifier used `hydra_batch` on a scratch db with 'Allister - Overrated': A, A (skipped), B, then A analyzed again instead of skipping.

A user who tries a rules edit, re-analyzes a few songs, then reverts sees them as Not analyzed. Their other caps and lenses are gone too.

`RecordStore::write_row` owns the purge policy. Either it changes to match decision 15, or the user decides again and the docs are rewritten. Purge 1 came in 3859380 (2026-09-03) and began deleting other-rules rows in 2a5691c (2026-09-24), the same commit that wrote ADR 0014.

*Owner proposed: RecordStore::write_row (the purge policy), or the docs after the user decides again. Candidates: store-1.*

#### 66. Two INI line readers treat comments and bad lines differently

The question is how a Hydra `key=value` INI line is split.

Two readers answer it. `Settings::load_file` in `src/app/config.cpp` (lines 59-66) skips a line only when `#` is its first character. It skips a line with no `=`, and reads numbers with `atoi`. `load_rules_file` in `src/app/rules_file.cpp` (lines 47-55) cuts from any `#`, throws on a line with no `=`, and reads numbers strictly.

| Line | hydra_settings.ini | hydra_rules.ini |
|---|---|---|
| `key=Hard # practice` | value keeps "# practice" | comment stripped |
| no `=` | skipped | error |
| `depth_value=abc` | 0 | error |

They give different answers today. This was shown by reading. The two files have different contracts, so "they should agree" is not proven. The settings leniency is pinned only by the test "malformed INI lines are tolerated" (`tests/test_config.cpp`, line 109).

The user would see this after a hand edit. `view_difficulty=Hard # practice` fails to parse, so the difficulty silently becomes Expert.

One line splitter should do the splitting. The settings reader dates from 3b69958 (2026-08-15); the rules reader from 7e14401 (2026-09-24). The rules syntax is decided (2026-09-24 plan, line 40, decisions 14 and 18). Nothing was found for the settings syntax in docs/adr, CONTEXT.md, plan headers or the UserGuide.

*Owner proposed: core/strutil: one line splitter (trim, cut comment, split at '='); each file keeps its own error policy. Candidates: store-19.*

#### 67. Library search and report-page search match rows by different rules

The question is: does this row match what the user typed in a search box?

Two rules answer it. The Library uses `fold_into` and `query_matches` in `src/app/library_query.cpp` (around line 103). It folds accents and full-width letters, lowercases ASCII only, and matches each word separately, in any order. The HTML report pages use a JS filter instead. It lives in `src/app/report.cpp` (around line 121), `src/app/dm_report.cpp` and `src/app/fill_report.cpp` (both around line 94), and the shared `visible()` script in `src/app/html_page.cpp` (around line 328). That filter joins the fields, lowercases them with full-Unicode JS `toLowerCase`, and looks for the whole typed string as one piece. The three page filters agree with each other.

| Row and query | Library | Report page |
|---|---|---|
| Title "Beyoncé", query "beyonce" | match | no match |
| Song "Halo", artist "Beyonce", query "beyonce halo" | match | no match |
| Title "МИР", query "мир" | no match | match |
| Query "title artist" across two fields | match | match |

They disagree today. The verifier showed this by reading both implementations, not by a run. The user can type the same text in the Library and on a report page and get different songs back.

CONTEXT.md defines "Library search" for the Library only. No ADR or plan says report pages must follow it or must differ. The owner should be `fold_for_search`: the report payload could carry pre-folded text and match per word. Otherwise the user should record that report search is a separate rule.

*Owner proposed: fold_for_search in src/app/library_query.cpp (or a recorded decision that report pages use a simpler rule). Candidates: sweep-core1-c2.*

#### 68. hydra_replay accepts "true" and "yes" for on/off flags; the settings file accepts only "1"

The question is: which text counts as "on" for a yes/no option?

Two places answer it. `flag_bool` in `tools/replay.cpp` (around line 90) takes "1", "true" or "yes". It is used only for `--prodrums` and `--bass2x`. `Settings::load_file` in `src/app/config.cpp` takes only "1". It repeats that test seven times, at lines 69, 71, 72, 75, 77, 93 and 94.

| Text | `flag_bool` | `load_file` |
|---|---|---|
| "1" | on | on |
| "true" | on | off |
| "yes" | on | off |
| "0", "", "TRUE", "on" | off | off |

They disagree today on "true" and "yes". The verifier found this by reading; nothing was run. The two read different sources. One is a command-line flag. The other is an ini file the app writes itself as 1 or 0. So the GUI shows nothing wrong. The visible effect is that hydra_replay quietly accepts two words its own help text ("0|1") does not mention.

`flag_bool` came in 30c4008 (2026-09-03). The ini pattern dates from 3b69958 (2026-08-15). No user decision was found in docs/adr or CONTEXT.md. If one rule is wanted, a `parse_bool` in `src/core/strutil` could serve both and fold the seven repeated tests into one call.

*Owner proposed: a small parse_bool in src/core/strutil, if one rule is wanted. Candidates: sweep-core1-c13.*

#### 69. Progress bars disagree on how full a bar is when the total is 0

The question is how full a progress bar should be when it shows done/total and the total is 0.

Three places answer it. `progress_bar_counted` in `src/ui/widgets.h` (line 54) falls back to 1, a full bar. Its comment says an empty batch is finished, not stuck. Its only caller is the Scanning modal. `render_batch_strip` in `src/ui/library_dialogs.cpp` (line 389) computes the same fraction inline and falls back to 0, an empty bar. `PreviewLoadJob::Progress::fraction` in `src/ui/preview_load_job.cpp` (lines 82-83) gives the per-stem part 0, so its bar would sit at 10%.

| done / total | Scan modal | Batch strip | Preview decoding |
|---|---|---|---|
| 0 / 0 | full (1.0) | empty (0.0) | 0.10 |
| 0 < done < total | done/total | done/total | 0.10 + 0.75 x done/total |
| done = total > 0 | full | full | step moves to Mixing (85%) |

They disagree today at total = 0, shown by reading. A scan that finds zero charts shows a full "0/0" bar. The batch strip reaches total = 0 in the gap before the first progress callback, and during a 0-of-0 batch just before it finishes. In both cases its thin bar sits empty for a moment where the scan modal would draw it full. A stemless Preview never actually sits at 10%; it jumps from Reading 0% to Mixing 85%.

The batch strip used to call `progress_bar_counted`. Commit a591bba (2026-09-27, the UI redesign) replaced that call with an inline copy and flipped the fallback. The helper itself dates from 4ce409a (2026-08-18). No user decision on the zero-total rule was found in `CONTEXT.md`, `docs/adr/`, or the 2026-09-26 and 2026-09-27 plans.

`widgets.h` should own one done/total fraction helper that all three call. The strip needs its own size and colours, so the helper should return the fraction, not draw. The zero-total answer should be a parameter or decided once. In the strip, total 0 means "not reported yet"; in the scan it means "nothing to do".

*Owner proposed: src/ui/widgets.h: one done/total fraction helper (split out of progress_bar_counted) that the scan modal, batch strip and Preview job all call. Candidates: sweep-ui1-c5, sweep-ui2-c1.*

#### 70. Two ellipsis rules cut text differently at the edge and around a trailing space

The question is how a line of text gets cut to fit a width and ended with an ellipsis ("...").

Two rules answer it, in three routines. The first rule is ImGui's `RenderTextEllipsis`. `ui::text_ellipsized` in `src/ui/widgets.h` (lines 136-156) calls it, for the details panel, library table and library dialogs. `draw_title_ellipsized` in `src/ui/library_table.cpp` (lines 216-230) is a hand copy of the ImGui cut. It exists only to hand back the kept width for the search highlight. The same file re-types the fit test again around line 379. The second rule is `render::ellipsize` in `src/render/overlay_layout.cpp` (lines 135-159). Only the Preview's path picker (`render_path_picker`) uses it.

The rules differ in two ways. The ImGui rule uses a strict "less than" at the edge, so an exact fit is cut. `render::ellipsize` allows an exact fit. The ImGui rule also keeps a trailing space before the ellipsis, while `render::ellipsize` trims it.

| Input | ImGui rule (`text_ellipsized`, `draw_title_ellipsized`) | `render::ellipsize` |
|---|---|---|
| "abcdef" in 50 px (10 px per char, 10 px ellipsis) | abc... | abcd... |
| "ab cd" in 45 px | "ab ..." (keeps space) | "ab..." (trims space) |
| "3- 1 2 41", room for 7 characters plus the ellipsis | "3- 1 2 ..." | "3- 1 2..." |

They disagree today. One verifier showed this with a Python port of both rules; another by reading. Text that fits with room to spare, and widths under one character, give the same result in all three. The two ImGui-based copies agree with each other. `row_selectable` in `widgets.h` only decides a tooltip and has no caller, so it is dead code, not a cut.

No string goes through both rules today. On screen, a cut label ends in " ..." in the library table and song panel, but in "..." with no space in the Preview path picker. The picker's label can also end one character later than the same text would in a library cell.

`render::ellipsize` should own the rule, because it has no device dependency and is already tested in `tests/test_overlay_layout.cpp`. It should be fed an ImGui text measure and gain a version that returns the kept width, which is what `draw_title_ellipsized` needs for the search highlight. No decision was found in CONTEXT.md or docs/adr/. The copies came in with 21ddf53 (`text_ellipsized`), 9b4352f (the title copy, 2026-09-27) and 79ea401 (`render::ellipsize`, 2026-09-27).

*Owner proposed: render::ellipsize in src/render/overlay_layout.cpp (device-free and unit-tested), extended to return the kept width; the ImGui-based cuts call it. Candidates: sweep-tests2-c10, sweep-ui2-c5.*

#### 71. The no-notes error is worded differently by the search, and recognised by retyping its words

The question has two halves. Does the chart have notes at the chosen difficulty, and if not, what sentence does the user read? And later, is a given error that sentence, so it can be shown as is?

`no_notes_message` in `src/parse/song.cpp` (line 183) owns the sentence. It builds "No <difficulty> [Pro ]Drums notes in this chart.". Two places use it: `analyze_chart_file` in `src/app/analysis.cpp` (line 503) and `PreviewLoadJob::run` in `src/ui/preview_load_job.cpp` (line 40). Both test `Song::is_empty` and throw the owner's sentence. `analyze_chart` in `src/search/pather.cpp` (lines 179-180) has its own empty check and types "No drum notes in this chart." by hand, skipping the owner. A fourth place, `is_no_notes_message` in `src/app/user_messages.cpp` (line 75), recognises the sentence by retyping its shape: it starts with "No " and ends with " notes in this chart.".

| Message | Made by | Recognised |
|---|---|---|
| No Expert Pro Drums notes in this chart. | song.cpp (GUI analysis, Preview) | yes |
| No drum notes in this chart. | pather.cpp (hydra_replay, bench) | yes |
| No Foo notes in this chart. | nobody | yes |
| No notes in this chart. | nobody | no |

The two GUI copies agree today because both call the same owner. The pather copy differs today, shown by reading, not by a run. An empty Expert Pro Drums chart reads "No Expert Pro Drums notes in this chart." in the GUI, but "No drum notes in this chart." from hydra_replay or bench. The recogniser is looser than the owner, but nothing produces the loose cases, so every real message matches. `SongLengthJob::run` is not part of this; an empty song there just means no length.

In the GUI nothing differs today. The command-line tools show the generic wording for the same condition. If the parser's wording changed, `is_no_notes_message` would stop recognising it. A batch failure line would then say "Something went wrong". Single-song and Preview errors would show the generic "couldn't read this chart file" text.

`parse/song.cpp` should own both the sentence and the check that recognises it, or throw a typed error so no text matching is needed. One function next to `load_songpath` should load the song and throw the no-notes error. Analysis, the Preview job and `pather.cpp` would all call it.

The recogniser came in with e6d8159 (2026-09-27). Who introduced the pather sentence was not checked. No user decision was found in `docs/adr/` or `CONTEXT.md`.

*Owner proposed: src/parse/song.cpp: no_notes_message plus its recogniser (or a typed NoNotesError), and one load-and-check function next to load_songpath that analysis, the Preview job and the tools all call. Candidates: sweep-tests4-c3, sweep-ui2-c15.*

#### 72. Preview volume range 0 to 100 is enforced three different ways, and a typed slider value escapes it

The question is what the Preview volume starts at, what values it may take, and how a percent becomes a playback gain.

The default 40 lives twice: `preview_volume` in `src/app/config.h` (around line 85) and `volume_pct_` in `src/ui/preview_controller.h` (around line 231).

The 0..100 range lives three times, each with a different rule. `Settings::load_file` in `src/app/config.cpp` (lines 80-83) accepts 0 to 100 and otherwise keeps the default 40. The slider in `render_preview_panel` in `src/ui/preview_tab.cpp` (line 280) is a `SliderInt` from 0 to 100 with no flags, and it writes the setting directly. `PreviewController::set_volume` in `src/ui/preview_controller.cpp` (lines 253-256) clamps to 0..100 before turning the percent into gain.

Percent-to-gain (divide by 100) is typed twice in `preview_controller.cpp`, in `poll` and in `set_volume`. The "below 0 becomes 0" gain floor appears in both `PreviewTransport::set_gain` and `Playhead::set_gain` in `src/audio/player.h`.

| Typed value | Slider shows | Playback gain | After restart |
|---|---|---|---|
| 0 to 100 | that value | value / 100 | same value |
| 150 | 150% | 1.0 | 40 |
| -5 | -5% | 0 | 40 |

They agree for every value from 0 through 100. They disagree when the user Ctrl+Clicks the slider and types a number.

The members disagree on whether that can happen. One says the slider stops at 100, so 150 can never reach Settings. The vendored ImGui source (1.93.0 WIP) settles it the other way. `third_party/imgui/imgui_widgets.cpp` (around lines 2705-2708 and 3377-3378) clamps Ctrl+Click input only when `ImGuiSliderFlags_ClampOnInput` is set, usually through `ImGuiSliderFlags_AlwaysClamp`. The Preview slider passes no flags. So typed input is not clamped. This was shown by reading, not by a run.

On screen, typing 150 stores 150 in the setting and the slider shows "150%". The audio plays at full volume, because `set_volume` clamps it. The settings file saves 150. On the next launch `load_file` rejects it, and the volume falls back to 40%.

The drift does not reach playback for in-range values. `render_preview_panel` calls `set_volume` with the setting every frame before `poll`. Only a controller used without `set_volume`, such as a test, would see the controller's own stale 40.

All copies came from ba0885a (2026-08-22). The slider line was reworked in eeb5d38 (2026-08-23). No user decision was found in CONTEXT.md, docs/adr/ or those commit messages.

`app::Settings` should own the default and the range, as one min/max pair or clamp function in `config.h`. The loader, the slider (with `ImGuiSliderFlags_AlwaysClamp`) and `set_volume` would all use it. One percent-to-gain helper should serve both controller sites.

*Owner proposed: app::Settings in src/app/config.h: one volume min/max pair or clamp function, plus one percent-to-gain helper. Candidates: sweep-tests1-c4, sweep-ui2-c2.*

#### 73. Jumping back after the audio ends leaves the highway moving in silence

The question: is the Preview's audio playing right now?

Two flags answer it. The clock's flag (`clock_.playing()`) is what the play button and `PreviewTransport::playing()` report. The audio playhead keeps its own `playing_` flag, which `Playhead::read_frames` in `src/audio/player.cpp` (around line 64) obeys. That function switches its own flag off when the audio runs out. `PreviewTransport::seek_ms` in `src/ui/preview_transport.cpp` (around line 60) seeks the playhead but never turns it back on. Only `PreviewTransport::play` does. `jump_ms`, `jump_activation` and `seek_activation` in `src/ui/preview_controller.cpp` all seek while playing without a pause and play.

They disagree today. The verifier showed it with a line-for-line Python port plus reading the C++. Take audio of 1000 ms with the last note at 2000 ms. Play to 1500 ms, then jump back 1000 ms. The clock says playing, the playhead says stopped, and `read_frames` serves silence. Pause then play brings the sound back. No test covers this case.

On screen: on a chart that runs past its audio, a jump back or '< Act' while playing scrolls the highway in silence. The play button still shows playing.

The auto-pause came in ba0885a (2026-08-22); the offset-aware seek line came in 1b7eeb3 (2026-09-25). `PreviewTransport` should own the play state. CONTEXT.md 'Transport' says the clock is the master and the audio follows it, so a seek should make the playhead match the clock.

*Owner proposed: PreviewTransport (src/ui/preview_transport.cpp). Candidates: sweep-media1-c9.*

#### 74. Two magic-byte rules disagree on which bytes count as playable audio

The question is whether some bytes are an audio container Hydra can play. Both answers look at magic bytes, the fixed tag at the start of a file that names its format.

Two functions answer it with different lists. `sniff_format` in `src/audio/decode.cpp` (line 49) is the decoder's rule. It needs RIFF followed by WAVE at byte 8. For Ogg it needs "OpusHead" or "vorbis" within the first 64 bytes. `looks_like_audio` in `src/app/preview_source.cpp` (line 181) is the .srb extractor's rule. It accepts any file that starts with RIFF or OggS. `extract_srb_audio` uses it twice. First, at line 253, it decides whether to keep a compressed stream as a stem; at line 266 the function returns early as soon as it has any stem. Second, at line 299, it judges whether a decrypted blob worked. A third function, `is_audio_filename`, judges file names, not bytes, so it answers a different question and is not a copy.

| First bytes | looks_like_audio | sniff_format |
|---|---|---|
| real WAV, Opus, Vorbis, MP3, fLaC | yes | known format |
| RIFF holding AVI | yes | Unknown |
| bare 4-byte RIFF | yes | Unknown |
| Ogg page carrying Theora, Ogg FLAC or Speex | yes | Unknown |
| Ogg with OpusHead past byte 64 | yes | Unknown |
| one 0xFF byte, PNG, empty, junk | no | Unknown |

They disagree today. One verifier filled the table by running a Python port of both functions; the other by reading both bodies. The rows agree with the code as it stands. No real chart that hits the gap was found.

If one did, the Preview would likely play no sound for that .srb. The odd stream is kept as a stem. From the compressed-stream path, the early return then stops `extract_srb_audio` before it decrypts the real audio. Later `decode_audio` throws on the stem. `decode_and_mix` in `src/audio/mixer.cpp` skips it without a word, but still counts it in the progress.

`sniff_format` should own the rule, because the decoder is what must actually accept the bytes. `looks_like_audio` should become `sniff_format(bytes) != AudioFormat::Unknown`, or be deleted.

Both rules came in ba0885a (2026-08-22). 5a48104 (2026-09-15) added the second call, on decrypted blobs. `CONTEXT.md` under "Mixer" records that an undecodable stem is skipped. No decision covers which bytes count as audio; none was found in `docs/adr/`, `CONTEXT.md` or those two commit messages.

*Owner proposed: audio::sniff_format in src/audio/decode.cpp; looks_like_audio becomes sniff_format(bytes) != AudioFormat::Unknown or is deleted. Candidates: sweep-media1-c1, sweep-tests1-c1.*

#### 75. widest_word re-walks the line breaks that wrap_words decides

The question: where can a line break fall in a Preview text box, and so how narrow can the box get?

Two functions in `src/render/overlay_layout.cpp` answer it. `wrap_words` (around line 87) breaks only at a word end outside the kept tail. (The kept tail is the last few words that must stay on one line.) `widest_word` (around line 118) promises 'the narrowest wrap_words can make it', but does its own walk instead of asking `wrap_words`. Both share `tail_start` for the tail. The word walk is written twice. `preview_tab.cpp` calls both with the same arguments (around lines 432 and 490).

| Text, keep_last | wrap_words widest line | widest_word |
|---|---|---|
| 'Next: activation 2 of 3', 3 | 10 | 10 |
| '   ba a', 3 | 4 | 7 |
| ' ', 0 | 1 | 0 |

They disagree today. The verifier ran a Python port over every short string. All 21,875 strings without leading or trailing spaces agree. Every mismatch on non-blank text needs leading spaces and keep_last larger than the word count.

The Preview's real headers never hit that case. An all-space detail line would size the box from 0 while the wrap draws a blank line.

Both came in 085555c (2026-09-27). No user decision was found in docs/adr/ or CONTEXT.md. `wrap_words` should own the break points, and `widest_word` should read its widest line.

*Owner proposed: render::wrap_words in src/render/overlay_layout.cpp; widest_word becomes the widest line of wrap_words(text, 0, width_of, keep_last). Candidates: sweep-media1-c16.*

#### 76. Draw code adds its own tie-break for which pad colours a taken fill

The question: which lane colour does a taken fill light on the highway?

The lane itself is derived once, in `build_track_state` in `src/render/track_state.cpp`, from the activation's lane. `TrackState::synthesize` (around lines 143-171) then answers per instant, and a span that starts at a moment wins over one that ends there. `build_highway_draws` in `src/render/highway_draw.cpp` (around lines 293-302) adds a second tie-break. For each merged on-span it takes the pad of the first instant inside it, and colours the whole span with that.

| Fills A=[1,2] Red, B=[2,3] Blue | synthesize | draw code |
|---|---|---|
| t = 1 | Red | Red |
| t in (2,3] | Blue | Red |

They agree whenever a merged span holds one fill, which is every case the verifier could show reachable. They disagree only when two taken-fill lanes touch or overlap. That was shown by reading, not by a run. Touching would need a fill to start on a half tick. Overlap would need a second taken fill to activate while the first SP still runs. Neither was proven impossible.

On screen, if it happened, the second fill's lane would light in the first fill's colour.

Both rules came in ba0885a (2026-08-22). No user decision was found. `TrackState` should carry the pad on each span, so the draw code only reads it.

*Owner proposed: render::TrackState (src/render/track_state.cpp) carrying the pad on each span. Candidates: sweep-media1-c7.*

#### 77. Probe scripts disagree on the total hit-window cap: 171.43 ms in watch_window, 170 ms (2 x back) in passive_probe and poll_windows

The question is where Clone Hero stops the total hit window when it caps it. A second question rides along: how close must a reading be to count as "at the cap"?

Three probe scripts in `tools/ch_probe/experiments` answer it two ways.

`watch_window.py` sets `CAP_EXPECT_MS = 171.43` (line 47). Its `window_report` (around line 183) accepts a steady reading only within 0.01 ms of that, and only for runs with a gap of 170 ms or more.

`run_passive_probe` in `passive_probe.py` (lines 164-168) judges the "whole window" against 2 x `EXPECT_NORMAL_BACK_MS`. That is 170 ms in normal mode and 80 ms in precision mode. It hands that cap to `clamp_verdict` in `analysis.py` (line 153), whose tolerance is 1.0 ms.

`main` in `poll_windows.py` (line 124) uses the same 2 x back rule plus 0.5 ms. It makes its own clamp test instead of calling `analysis.clamp_verdict`. It cannot call that function as written, because it never sees the raw value.

The test file `test_hit_window_scripts.py` repeats 171.43 and the 170 gap threshold as literals (lines 47, 81, 89, 101, 103).

| Stored window (raw 180) | watch_window | poll_windows | passive_probe (cap 170) | clamp_verdict at cap 171.43 |
|---|---|---|---|---|
| 171.43 | matches the cap | NO CLAMP | inconclusive | clamp |
| 170.0 or 171.0 | DIFFERENT from 171.43 | clamp | clamp | not checked |

They disagree today. This was shown three ways. Reading the code gives the table above. The committed `results/poll_windows.csv` peaks at 171.43 six times, and 171.43 is above 170.5, so poll_windows prints NO CLAMP for a real capped reading. Running the real `clamp_verdict` on five rows confirmed the passive-probe column. Raw values just above the cap (171.2 to 172.4) even give "no_clamp" at cap 170.

On screen, the probe console prints opposite verdicts for the same song. At the measured cap, the passive probe's "whole window" verdict can never say clamp. The Hydra GUI is untouched.

Nobody has settled which cap the engine really uses. The 171.43 came in with 283e028 (2026-09-26). The 2 x back rule came in with aa3ad89 (same day). The plan `docs/superpowers/plans/2026-09-25-hit-window-testing.md` (lines 52-53) expects 171.43 in its body, not as a user quote. Nothing chooses 2 x back.

One measured whole-window cap constant and tolerance in `constants.py` should own this, because it is one game fact. constants.py already holds the back-window edges. Both scripts and the tests would read it. `analysis.clamp_verdict` stays the judge of raw against stored.

*Owner proposed: tools/ch_probe/constants.py: one measured whole-window cap constant and tolerance; analysis.clamp_verdict stays the raw-vs-stored judge. Candidates: sweep-probe1-c11, sweep-probe1-c12, sweep-probe2-c1.*

#### 78. Experiment scripts each rewrite the live-engine pick their own way

The question is which engine object found in a memory scan is the live one. Clone Hero leaves frozen engines behind after a restart, so this matters.

`find_live_engine` in `tools/ch_probe/engine_finder.py` (lines 136-183) owns the answer. It drops hits inside the game DLL, using `MODULE_SPAN = 0x4000000` (line 27). It skips a total window below 0.001 s. It returns the first candidate whose song clock moves by more than 1e-6 within 0.12 s.

Four older experiment scripts pick their own way. Each writes `module_base + 0x4000000` as a literal: `find_engine.py` (lines 65, 75), `find_clock3.py` (lines 34, 64), `poll_windows.py` (line 55) and `first_engine_with_window` in `hit_detect.py` (lines 46-52).

| Script | Window test | Liveness test |
|---|---|---|
| `find_live_engine` (owner) | skip if below 0.001 | song clock moved |
| `hit_detect` `first_engine_with_window` | keep if above 0.001 | none, first match |
| `find_engine` `main` | keep if above 0.001 | none, lists every hit |
| `poll_windows` `main` | none | none, first heap hit |
| `find_clock3` `main` | keep if above 0.001 | window value moved more than 0.0001 in 0.2 s |

The two findings disagreed on whether this matters today. Both are right about different rows. The window threshold edge does not come up in practice: at exactly 0.001 s the owner keeps a candidate and three copies drop it, but real windows are about 0.17 s. The liveness test does come up. A run with the real functions and a fake process showed it, using a heap ordered empty, frozen, live. The owner picks the live engine. `hit_detect` picks the frozen one. `poll_windows` picks the empty one. `find_clock3` never picks a live engine whose window stays steady. A frozen engine ahead of the live one is exactly what a restart leaves behind.

Nothing reaches the Hydra GUI. On the probe side, those scripts can attach to a dead engine and print a frozen window or score. If the module span changed in one place, the experiments could also pick a different object than `find_live_engine`.

`engine_finder` should own the span and one shared filter. The newer runners already call `find_live_engine`. The owner arrived in 288b07a (2026-09-26). The experiments came in 283e028 (2026-09-26) and were never repointed to it. The 2026-09-26 audit-fixes plan (Task 20) kept these scripts, and Task 21 Step 10 asks the user whether to delete three of them. That defers removal; it does not approve the separate rules.

*Owner proposed: engine_finder.find_live_engine and MODULE_SPAN in tools/ch_probe/engine_finder.py, as one shared candidate filter. Candidates: sweep-probe1-c3, sweep-probe2-c3.*

#### 79. Probe runners wait for song time with separate loops; play_chart presses up to 2 ms early and stalls differ

Two questions share one loop in the Clone Hero probe tools. When has the target time arrived on the song clock, so a key press fires? And has the song quit, paused or restarted? None of this touches the Hydra app itself.

The intended owner is `InputDriver.schedule_hit` in `tools/ch_probe/input_driver.py` (lines 210-229), but only its tests call it. The live runners in `tools/ch_probe/experiments/` each write their own loop. `wait_until` inside `main` in `walk_edges.py` (lines 224-242) fires when the `SongClock` estimate reaches the target. `wait_until` inside `drive_inputs` in `active_probe.py` (lines 141-159) is a near line-for-line copy of it. The main loop in `play_chart.py` (lines 387-431) fires when the raw engine clock reaches the target minus 0.002 s (line 409). `main` in `watch_window.py` (lines 277-287) runs its own health checks with `STALL_S = 8.0` (line 50).

When a press fires:

| Time left before target | schedule_hit | walk_edges | active_probe | play_chart |
|---|---|---|---|---|
| 0.0021 s | wait | wait | wait | wait |
| 0.001 s (or 1.5 ms) | wait | wait | wait | fire |
| 0 s | fire | fire | fire | fire |

When the song is judged stopped:

| Situation | active_probe, walk_edges | play_chart | watch_window | schedule_hit |
|---|---|---|---|---|
| Clock frozen 6 s | stop at 5 s | stop at 5 s | keeps going (8 s limit) | waits to its 30 s timeout |
| Clock jumps back 2 s | stop | re-sync, keep playing | stop | no check |
| Clock steps of 1 ms or less | counts as movement | counts as frozen | counts as movement | n/a |

They disagree today. The verifiers showed it by reading and by a scratch truth-table run. Nobody stops at exactly 5.0 or 8.0 s, because every check uses a strict greater-than. One finder's example of 0.035 s left was wrong: at that point the loops only sleep differently, and none fires.

The effect is on probe input timing and probe runs. `play_chart` presses up to 2 ms earlier than the other probes would for the same target. The same pause or restart stops some runners and not others.

One shared helper should own both rules, so every probe presses at the same moment and agrees on when a song has stopped. It can be `schedule_hit` extended, or a helper next to `walk_edges.SongClock`, which `active_probe` already imports. The runner loops came in with 283e028 and aa3ad89 (2026-09-26); `schedule_hit` is older (396082a). No user decision was found that chooses a 2 ms lead, different fire margins, or 5 s versus 8 s stall limits.

*Owner proposed: One shared wait / clock-watch helper: InputDriver.schedule_hit extended, or a helper beside walk_edges.SongClock. Candidates: sweep-probe1-c8, sweep-probe1-c9, sweep-probe2-c9.*

#### 80. Starting mid-song, three runners skip different notes

The question is: when a runner starts partway through a song, which notes are already too late to play?

Three runners each write their own rule. `main` in `walk_edges.py` (lines 215-217) keeps notes at least 150 ms ahead of the clock. `drive_inputs` in `active_probe.py` (line 161) keeps notes more than 150 ms ahead. `main` in `play_chart.py` (lines 361-363) keeps notes up to 50 ms behind the clock, and repeats that rule in its jump-back re-sync (lines 398-400).

| Note position vs clock | walk_edges | active_probe | play_chart |
|---|---|---|---|
| 30 ms behind | skip | skip | play |
| 100 ms ahead | skip | skip | play |
| exactly 150 ms ahead | play | skip | play |
| 200 ms ahead | play | play | play |

They disagree today. The verifier showed this by reading. Even the two "+150" copies split at the boundary, because one uses less-than and the other greater-than.

On screen, the runners start on different notes for the same clock reading. Their probe output starts at a different row. The Hydra GUI is untouched.

The owner should be one start-cursor helper shared by all three runners. The rules came in with 283e028 and aa3ad89 (2026-09-26). No user decision was found in `docs/adr/`, `CONTEXT.md`, those commit messages or the 2026-09-25 hit-window plan.

*Owner proposed: one start-cursor helper shared by walk_edges, active_probe and play_chart. Candidates: sweep-probe1-c10.*

#### 81. Probe scripts report a hit's timing offset by two different formulas

The question is: how far from the note did a hit land, in ms?

`walk_edges.py` computes it in `main` (lines 257-259) and returns it through `Row.measured_ms`. The rule is the engine's stored hit time (`+0x2e0`) minus the note time if that field changed, otherwise the estimated send time minus the note. `drive_inputs` in `active_probe.py` (lines 176-179) writes the same rule again inline, and its docstring calls it "walk_edges.py's rule". `main` in `hit_detect.py` (line 142) answers differently: the song clock (`+0x100`) minus a field at `+0x28`.

| Note 10.000 s, hit stored 10.010 s | Offset reported |
|---|---|
| walk_edges | +10.0 ms |
| active_probe | +10.0 ms |
| hit_detect | about 18.2 ms |

They disagree today, shown from run data. In `results/hit_detect.csv`, hit_detect's value sits at 17.5 to 18.3 ms on almost every hit, whatever the press timing. It measures a near-constant gap between two fields, not a per-hit offset. walk_edges and active_probe agree on every input.

On screen, probe outputs print different "delta" numbers for the same kind of hit. The Hydra GUI is untouched.

The owner should be `walk_edges.Row.measured_ms`, or better a shared function in `experiments/live.py` that both runners call. The copies came in with 283e028 and aa3ad89. `hit_detect.py` is one of the three scripts the audit-fixes plan kept pending a user yes to delete.

*Owner proposed: walk_edges.Row.measured_ms, or a shared function in experiments/live.py. Candidates: sweep-probe1-c7.*

#### 82. Two functions find the hit/miss edge and disagree when results overlap

The question is: given rows of (offset, hit or miss), where does hit turn into miss?

`find_window_edge` in `tools/ch_probe/experiments/analysis.py` (line 66) owns it. It tries every threshold and returns the one with the fewest errors, taking the middle one on ties. `active_probe` reaches it through `summarize_active`. `summarize` in `walk_edges.py` (lines 158-175) uses its own rule instead. It takes the widest hit and the narrowest miss, and gives an edge only if they don't overlap.

| Rows | find_window_edge | walk_edges summarize |
|---|---|---|
| hits 80,82,84; misses 86,88,90 | 85.0 (0 errors) | between 84 and 86 |
| hits 80,82,84,91; misses 86,88,90,92 | 85.0 (1 error) | "overlap", no edge |
| hit 85 and miss 85 | 85.0 (1 error) | "overlap", no edge |
| all hits 80,82 | 83.0 | above 82 |

They disagree today whenever any hit lands at or beyond any miss. The verifier showed this by a run that imported both functions. On clean data they agree.

On screen, walk_edges would print no edge while active_probe's summary prints a number with an error count. The two scripts feed on different experiments, so no single set of rows reaches both today. The Hydra GUI is untouched.

The owner should be `analysis.find_window_edge`. `summarize` could print its edge and error count and keep the overlap warning as a note. The copies came from 396082a (owner) and 283e028 (summarize). No user decision was found.

*Owner proposed: analysis.find_window_edge. Candidates: sweep-probe1-c14.*

#### 83. test_attach attaches its own debugger and can kill Clone Hero on Ctrl+C

The question is: how does the probe attach a debugger to Clone Hero without risking the game?

`Debugger.attach` in `tools/ch_probe/debugger.py` (line 461) owns the safe answer. It attaches, then turns off "kill on exit" right away. Kill on exit is a Windows default: if the debugging program dies while attached, Windows ends the game too. If turning it off fails, attach detaches and raises. `main` in `experiments/test_attach.py` instead calls `DebugActiveProcess` itself (line 40) and runs its own 3 s event loop (lines 53-70). It never turns kill on exit off. It has no try/finally, so the detach at line 83 is skipped if anything raises in between.

| Python dies while attached | Result |
|---|---|
| Debugger.attach path | Windows detaches, game keeps running |
| test_attach | kill on exit still on, game is terminated |

They disagree today on the crash path. The verifier showed this by reading; it was not run, because that needs a live game. On a clean run both end detached.

On screen, pressing Ctrl+C during the diagnostic can close Clone Hero. The Hydra GUI is untouched.

The owner should be `debugger.Debugger`. test_attach.py came in with 283e028. The kill-on-exit fix landed later in 3e71f50 (2026-09-26, Task 19) and didn't touch test_attach. The audit-fixes plan names kill on exit as the reason for Task 19, and nothing exempts test_attach.

*Owner proposed: debugger.Debugger (attach/run/stop). Candidates: sweep-probe1-c18.*

#### 84. Five scripts use five tolerances to decide the window value changed

The question is: has the engine's stored total window taken a new value?

Five experiment scripts each decide it their own way. `changes` in `watch_window.py` (line 69) uses a step over 0.000001 ms, and the same file repeats that test in its live loop (lines 290-294). `main` in `poll_windows.py` (line 95) uses 0.001 ms. `main` in `hit_detect.py` (line 118) uses 0.0001 s, which is 0.1 ms. There a window change also means "a new note arrived, press now". `find_engine.py` (line 110) rounds to 4 decimal places of a ms. `milestone2.py` (line 75) rounds to 2.

| Step | watch_window | poll_windows | hit_detect | find_engine | milestone2 |
|---|---|---|---|---|---|
| 85.0 to 85.05 ms | yes | yes | no | yes | yes |
| 85.0 to 85.0005 ms | yes | no | no | yes | no |
| 85.0 to 85.1 ms | yes | yes | no | yes | yes |

They disagree today, shown by a run of a Python port of each rule. The 85.1 case catches hit_detect out by float rounding: the step comes out just under its 0.0001 s line.

On screen, distinct-value counts and change logs differ between scripts. hit_detect can skip a key press when the step is 0.1 ms or less. The Hydra GUI is untouched.

No owner exists yet. One helper with one stated tolerance would fix it. The rules came in with 283e028 and 288b07a. The audit-fixes plan kept hit_detect and poll_windows but chose no tolerance.

*Owner proposed: none yet; candidate: one 'window changed' helper with one stated tolerance (e.g. in watch_window or analysis). Candidates: sweep-probe1-c22.*

#### 85. hit_detect presses chords its own way and plays 2x kick on O, not L

Two questions are answered differently here. How long does the probe hold keys for a chord? And which key plays a 2x kick?

`InputDriver.press_chord` in `tools/ch_probe/input_driver.py` (line 163) owns chord presses. It holds the keys 3 ms. `main` in `experiments/hit_detect.py` (lines 124-130) runs its own press loop with a 5 ms hold. Its pad keys do come from the shared key table through `get_binding`, so those follow a rebind. But hit_detect adds a literal `0x4F`, the O key, for 2x kick (line 80). The shared table `DEFAULT_BINDINGS` (line 61) has no 2x-kick entry at all. Meanwhile `play_chart.py` (line 163) maps the 2x-kick gem to the normal kick, L, and `tests/test_play_chart.py` pins that.

| Question | input_driver / play_chart | hit_detect |
|---|---|---|
| Chord hold time | 3 ms | 5 ms |
| 2x kick key | L (normal kick) | O |

They disagree today, shown by reading. The original claim that a rebind would break hit_detect was refuted, because hit_detect never rebinds and its pad keys follow the table.

At the game, hit_detect holds keys 2 ms longer and also presses O. The Hydra GUI is untouched.

The owner should be `input_driver`: call `press_chord`, and put the 2x-kick key in the table if one is needed. The literal came in 283e028. The audit-fixes plan left hit_detect as-is but chose neither the key nor the hold time.

*Owner proposed: tools/ch_probe/input_driver.py (press_chord, plus a 2X-kick entry in DEFAULT_BINDINGS/Lane if needed). Candidates: sweep-probe1-c23.*

#### 86. Two constants decide when a report means every path

The question: when does a path report count as showing every path? Two constants answer it. `kEveryPathSentinel` in `src/app/report.h` (line 105) is 1,000,000,000. That is what `--all-paths` asks for in `src/cli/report.cpp` (line 49). `kEveryPathLabelThreshold` (line 107) is 100,000,000. `generate_report` in `src/app/report.cpp` (line 394) prints "every path" whenever `max_paths` is above it.

| `--paths` value | Threshold rule (today) | Sentinel rule |
|---|---|---|
| 100000000 | top 100000000 paths per chart | top 100000000 paths per chart |
| 200000000 | every path | top 200000000 paths per chart |
| 1000000000 (`--all-paths`) | every path | every path |

They agree for `--all-paths`. They disagree for any `--paths N` between the two values, because `src/cli/report.cpp` (line 50) passes N straight through with no clamp. This was shown by reading; `hydra_report` was not run. A user typing `hydra_report --paths 200000000` sees "every path" in the subtitle. The GUI always sends `kDefaultReportPaths`, so it is unaffected. If the sentinel were ever moved below the threshold, `--all-paths` would read "top 1000000000 paths per chart".

`kEveryPathSentinel` should own it: label on `max_paths >= kEveryPathSentinel`, or derive the threshold from it. The user should say whether a huge `--paths` value also counts as every path. The 100,000,000 literal dates to 3b69958 (2026-08-15, the C++ port). Both were named as constants in 06a178c (2026-09-25). No user decision was found in CONTEXT.md, docs/adr/, or the header of the 2026-09-24 derivation-fixes plan.

*Owner proposed: kEveryPathSentinel in src/app/report.h. Candidates: sweep-tests3-c10.*

#### 87. Test tool prints different status words than the app shows

The question is: what word names a stored record's status?

The app answers it in `render_chips` in `src/ui/library_table.cpp` (around line 154). `chip_of` in `src/ui/library_model.cpp` maps the status to a chip first. `best_path_label` in the same file spells two of the words again for the Best path cell. The headless test tool answers it separately in `dump_state` in `tests/ui/uitest_harness.cpp` (around line 379).

| Status | App | `hydra_uitest` state dump |
|---|---|---|
| Ready | Analyzed | current |
| Stale | Stale | stale |
| NotAnalyzed | Not analyzed | new |

They disagree today for two of the three statuses, and Stale differs in case. This was shown by reading.

Nothing changes on the app screen. But the text from the `state` script verb, documented in `docs/agents/ui-testing.md`, uses words the app never shows. Anyone reading a dump has to translate. The dump words came first, in ba0885a (2026-08-22). The chip words came with the redesign in 99cbda2.

One status-to-label function in `library_model.cpp`, beside `chip_of`, should own the words. `render_chips`, `best_path_label` and `dump_state` would all call it.

*Owner proposed: one RecordStatus-to-label function in src/ui/library_model.cpp beside chip_of. Candidates: sweep-tests4-c8.*

#### 88. A Ready chart with no paths counts as analyzed in some places, not others

The question is: does a chart have a usable result? Five places answer it, using two different signals.

The library uses status. `chip_of` in `src/ui/library_model.cpp` (line 13) puts every Ready row under the Analyzed chip. `facts_of` in the same file (line 35) hands filter facts to any Ready row. The other places use "has a score". `query_matches` in `src/app/library_query.cpp` (line 385) rejects a row whose stars are unset, and the `RowFacts` comment in `src/app/library_query.h` (line 68) calls that "analyzed". `collect_dm_rows` in `src/app/dm_report.cpp` (line 198) needs a score to say matched. `collect_fill_rows` in `src/app/fill_report.cpp` (line 195) counts a side only if it has a score.

| Ready record | chip / facts_of | query_matches | DM page | Fill page |
|---|---|---|---|---|
| with paths | analyzed | analyzed | matched | counted |
| zero paths | analyzed | not analyzed | "not analyzed" | one-sided, or "only 1.1" |

They split on one input: a Ready record with zero paths. `summarize_record` in `src/store/record_store.cpp` (line 498) gives it no score and no stars. This was shown by reading. Tests write such rows, but nobody showed a live analysis producing one.

On screen, such a chart sits under Analyzed, yet `stars:` and `squeeze<=` never match it. The DM page would tell the user to analyze it. The two-layer library design comes from the v1.9.0 UI redesign plan (`docs/superpowers/plans/2026-09-27-ui-redesign.md`, line 1394), which assumed every Ready row has stars. That is a plan, not a user decision. CONTEXT.md lines 36-37 do decide that a Stale record reads as absent in listings; that part is settled. The store's Ready status should own "has a result", with the no-paths case handled explicitly once.

*Owner proposed: The store's RecordStatus (Ready), plus one explicit 'has a scored best path' flag decided once (for example in facts_of) and read by query_matches, dm_report and fill_report. Candidates: P1-1, P2-3.*

#### 89. A tied path shows its leader's leftover SP, not its own

The question is how many bars of Star Power a path still holds when the song ends. Two places answer it. The Paths tab summary line in `build_activations` in `src/app/path_view.cpp` (around line 355) reads the stored `leftover_sp`. The Preview gauge in `build_sp_meter_curve` in `src/app/preview_view.cpp` (around lines 183-198) never reads it. It recounts the bank from the chart's phrase list after the last activation.

The stored value is wrong for some tied paths. A variant is a tied path stored as a branch of its leader. `Engine::reduce_iteration_paths` in `src/search/engine.cpp` (around line 895) groups every finished path under one key, whatever SP it has banked. So two finished paths with equal scores fold into a leader and a variant. `emit_path` stores only the leader's bank. `Path::prepare_variants` in `src/core/model.cpp` (around line 553) copies it onto every variant. ADR 0017 describes that copy but never decides that a variant's bank may differ.

| Don Broco - Actors, depth 40 | Real bank at song end | Paths tab says | Gauge ends at |
|---|---|---|---|
| Path 29 [7], leader | 1 | 1 bar | 1 |
| Path 30 [3], variant | 3 | 1 bar | 3 of 4 |

They disagree today. A `hydra_replay dump` run at depth 40 showed both paths at 233100, ending with 1 and 3 bars. The Paths tab text comes from reading the code; the GUI was not driven. The finder reported no case at default depth, which was not re-checked.

A user who picks path 30 sees "1 bar of SP left over" while the gauge ends at 3 of 4.

The engine should own this. It should store each variant's own bank, or put the bank in the finished-path key. Both screens should then read the one stored `leftover_sp`.

*Owner proposed: Engine (src/search/engine.cpp): emit each variant's own SP bank, or key finished paths by bank. Candidates: R3-A1-1.*

#### 90. A path folded mid-SP keeps a stale SP end, clamp note and phrases

The question is where a tied variant's last activation ends. That includes which phrases it collected, which note pinned its cap clamp, and which notes are its backend rows (the notes just after the SP end, timed for squeeze ratings). It matters when the engine folds the variant into its leader while that activation's Star Power is still running.

The engine keeps two answers for one SP window. The leader holds the real, closed activation. The variant holds a snapshot from the fold moment. `Engine::reduce_group` in `src/search/engine.cpp` (around line 761) saves `sp_end`, `clamp_tick` and `col_tail` at that moment. `emit_acts` (around line 984) then treats the open activation as running to the song's end. `rebuild` (around lines 1196-1256) stamps the fold-time end as `deact_tick` and attaches the song's last notes as backends. `Path::prepare_variants` in `src/core/model.cpp` (around line 540) never shares the leader's closed activation. `windows_for_path` in `src/core/replay.cpp`, `build_sp_meter_curve` and `build_activations` all read the snapshot.

| Synthetic chart, cap 2 | Window | Stored deact_tick | Replay total | Stored total |
|---|---|---|---|---|
| Path '2' | X2 at 11520 | 16128 | matches | matches |
| Path '0 1' (variant) | X2 at 11520 | 14976 | 6950 | 7150 |

A run showed this on a constructed chart (`midsp/song/notes.chart`). Replaying '0 1' with the end at 16128 gives 7150. So the score is right and the end is wrong. The corpus showed 0 replay mismatches over 3,441 paths.

On screen, read from code: the Preview score box says "Score unavailable", the SP floor and gauge stop early, a late phrase banks as squeezed out, and the overfill note names an earlier note.

The activation the engine actually closes should own these facts. The user picks the fix: stop folding paths while SP runs, or give the variant the leader's activation at `var_point-1`. The snapshot grew field by field in 9113e76 (2026-08-28), 396082a (2026-09-21) and 2a5691c (2026-09-24). No user decision was found; ADR 0017 is silent on the SP end.

*Owner proposed: Engine: the activation the engine actually closes (Engine::reduce_group / emit_variant / Path::prepare_variants). Candidates: R4-A1-1.*

#### 91. User Guide promises the overfill warning more often than the code shows it

The question is when a Paths tab activation row shows the "SP overfilled" warning. The code answers in `rate_activation` in `src/core/squeeze_rating.cpp` (around lines 174-192). `build_activations` in `src/app/path_view.cpp` (around line 264) shows the warning only when that function sets `cap_clamped`. The rule has two parts. First, the window must be cap-clamped: `clamp_tick` is set, meaning a mid-SP phrase hit the cap and pinned the end. Second, the activation must have a SqIn/SqOut, or a backend row that is squeezed out or uncounted. `docs/cap-clamped-squeeze-frontend-anchor.md` agrees with the code. `docs/UserGuide.md` (line 126) states only the first part. ADR 0013 is silent on the second part. The guide and CONTEXT.md's "Cap-clamped window" entry also say "fills the meter to the cap". A phrase that only ties the cap does not clamp (`tests/test_search.cpp`, around line 700).

| Case | Code | User Guide |
|---|---|---|
| Clamped, with a SqIn/SqOut | warns | warns |
| Clamped, squeezed-out or uncounted row | warns | warns |
| Clamped, only counted rows | no warning | warns |
| Phrase ties the cap exactly | no warning | warns |
| No clamp | no warning | no warning |

They disagree today. Reading the unit test fixture `clamped_no_squeeze` in `tests/test_squeeze_rating.cpp` (around line 739) shows it: clamp at 3072, one counted row at -30 ms, no warning.

A guide reader who opens such an activation sees no warning. They may conclude the meter never overfilled.

`rate_activation` should stay the one owner. The guide and CONTEXT.md should state its real conditions. The second gate came in with 396082a (2026-09-21), whose message describes the warning without it. No user decision was found in `docs/adr/`, CONTEXT.md or that commit message.

*Owner proposed: rate_activation in src/core/squeeze_rating.cpp (cap_clamped); UserGuide.md and CONTEXT.md should restate it. Candidates: R3-A1-2.*

#### 92. Docs say a clamped SP end can't move; the engine moves it

The question is whether a phrase collected after a cap clamp can still push an activation's SP end later. ADR 0013 (`docs/adr/0013-cap-clamp-anchor-is-stored-by-the-engine.md`, lines 6-7) says no. CONTEXT.md's "Cap-clamped window" entry (lines 144-145) also says no. The engine says yes. `ScoreGraph::extend_deacts` in `src/search/graph.cpp` (around lines 267-276) runs for every later phrase. After the clamp the meter keeps draining. Once it is below full, the normal one-bar extension (2 measures) wins and the end moves. `build_sp_meter_curve` in `src/app/preview_view.cpp` (around line 167) follows the engine and adds a bar at each later phrase.

| Cap 2, activation at 2304, meter 2 | Engine | ADR 0013 and CONTEXT.md |
|---|---|---|
| Phrase at 3072 (clamps) | end 6144 | end 6144 |
| Then a phrase at 5760 | end 7680 | stays at 6144 |

They disagree today. The test "SP cap overfill: a later unclamped extension keeps the earlier clamp_tick" in `tests/test_search.cpp` (around line 730) asserts 7680. This was shown by reading that test.

No number on screen is wrong, because no code reads the docs. A glossary reader would expect a clamped end to stay put. They would then see the deact node, backend rows and gauge move. Code written from that wording would freeze the end and disagree with the record.

`extend_deacts` owns the rule. The ADR and the glossary should say the end is pinned while the meter is full, and that later phrases still extend it from there.

*Owner proposed: ScoreGraph::extend_deacts in src/search/graph.cpp; ADR 0013 and CONTEXT.md should be reworded. Candidates: R5-A1-2.*

#### 93. Paths row's 'N bars' is the bank at activation, but docs call it bars spent

The question is what the "N bars" on a Paths tab activation row means. `build_activations` in `src/app/path_view.cpp` (around line 202) shows `act.sp_meter`. That is the SP banked at the activation note. The `sp_bars` comment in `src/app/path_view.h` (line 82) calls it the bars the activation spends. `docs/UserGuide.md` (line 112) says the same. But SP lasts longer when a phrase is collected mid-SP. `ScoreGraph::extend_deacts` in `src/search/graph.cpp` (around lines 256-278) adds one bar per collected phrase, unless the cap clamps it. `build_sp_meter_curve` in `src/app/preview_view.cpp` (around line 155) drains that larger amount.

| Phrases collected mid-SP | Row shows | Bars SP lasts | Gauge drains |
|---|---|---|---|
| 0 | sp_meter | sp_meter | sp_meter |
| 1, no clamp | sp_meter | sp_meter + 1 | sp_meter + 1 |
| k | sp_meter | sp_meter + k, less any cap clamp | same as the engine |

They disagree today. Reading the fixture "SP past the last note: a mid-activation phrase extends the end" in `tests/test_search.cpp` (around lines 469-510) shows it. `sp_meter` is 2 and the end is 6912, which is 3 bars. The row reads "2 bars" and the gauge drains 3.

The row and the gauge each read engine truth correctly. They answer different questions. The defect is the description. On any activation that collects a phrase during SP, the guide tells the user the row is what they spend, while the gauge drains more.

The engine record should own this, since `sp_meter`, `collected_phrase_ticks` and `deact_tick` are already stored. The user decides what the row means. One option is to reword the guide and comment to "bars banked when you activate". The other is to show bars spent, read from the record. No decision was found in `docs/adr/` or CONTEXT.md.

*Owner proposed: The engine record (sp_meter, collected_phrase_ticks, deact_tick); user decides what the row means. Candidates: R6-A1-1.*

#### 94. The 2W squeeze budget is written three times with opposite edge rules

The question is: what is the normal two-hit squeeze budget (twice the hit window, 2W), and is a squeeze past it?

The owner exists. `squeeze_budget_ms` in `src/core/squeeze_rating.cpp` (around line 67) returns W×(1+r), which is 2W at r = 1. `src/app/path_view.cpp` (around line 299) already calls it for the "normal" scale tooltip.

Three other places write the number again. `timing_tiers` (around line 202) uses the literal `2 * w` as the Insane+ cutoff. `effective_backend_ms` (around line 64) hard-codes `2.0`. The report's Timing column help in `src/app/report.cpp` (around line 106) says "Beyond means at least twice the hit window" as fixed prose.

The edges also differ. `transfer_is_material` (around line 76) calls a gap over budget only when gap > budget. The report's `tier_for` calls it Beyond when ms >= 2W.

| Gap (W = 85, r = 1) | Details view over budget? | Report tier |
|---|---|---|
| 169.9 ms | no | Insane+ |
| 170.0 ms | no | Beyond |
| 170.1 ms | yes | Beyond |

Shown by reading, not a run. At exactly 170.0 ms the report says Beyond, "past the 170 ms window", while the details view shows no over-budget line. If the budget formula changed, the report tiers and column text would not follow.

`squeeze_budget_ms` should own it. `timing_tiers` should call `squeeze_budget_ms(1.0, w)`, and the user should pick one edge rule. The literals came from 7dded44 and dea4228 (both 2026-08-19). No user decision was found in `docs/adr`, `CONTEXT.md` or those commit messages.

*Owner proposed: squeeze_budget_ms in src/core/squeeze_rating.cpp. Candidates: R3-A2-2.*

#### 95. Tie limit counts per score and limit side, not per score as the guide says

The question: how many paths at one score does the search keep? The setting is `max_tied_paths` in hydra_rules.ini.

`Engine::reduce_group` in `src/search/engine.cpp` builds the tie key as `(score << 1) | filtered` (around line 748). "Filtered" means the path fails the Path limit. So paths inside and over the limit at the same score fold into two separate leaders. A leader is the one path that its ties fold into. The cap check (line 760) then runs per leader. The `max_tied_paths` entry in `docs/UserGuide.md` (line 249) promises a per-score limit. The comment on `Rules::max_tied_paths` in `src/core/rules.h` (line 30) says "one leader" and never mentions the split.

| Ties at the top score | Code keeps | Guide promises |
|---|---|---|
| Limit 4, all 5 inside | 4 | 4 |
| Limit 4, 4 inside and 1 over | 5 | 4 |
| Limit 4, 4 inside and 4 over | 8 | 4 |
| Limit 1, 1 inside and 1 over (run) | 2 | 1 |

They disagree today. A run on testdata "black midi - Sugar／Tzu" showed it. The command was `hydra_replay dump --legacy-fills --ms 0` with `max_tied_paths = 1`. It kept two paths at 655,190: '0 E3+ E5 E1' and '0 E3+ E5 E0'. Only the E0 path's early fill put it over the limit. At `--ms 10` just one is left. The 8-path row comes from reading the code, not a run.

On screen, the Paths tab's Optimal group can list more tied paths than the setting allows. The verifier did not check this in the GUI.

`Engine::reduce_group` should stay the one owner. The user needs to decide between one shared tie group (drop the filtered bit) and a per-leader limit stated in the guide and in rules.h. No ADR or CONTEXT.md entry covers tied paths. The key dates from 3b69958, the C++ port checkpoint.

*Owner proposed: Engine::reduce_group in src/search/engine.cpp (one place that folds ties); UserGuide.md line 249 and the rules.h comment must match whatever the user decides. Candidates: R3-A3-1.*

#### 96. Guide says fill slop decides whether a fill counts; code only picks its chord

The question: what does `fill_land_slop_beats` decide, and which fills does it touch?

The entry in `docs/UserGuide.md` (line 253) calls it the closeness "for the fill to count", and says it applies to authored fills "too". That reads as a pass/fail gate that also tunes generated fills. The code does neither. `fill_lands_on_chord` in `src/parse/song.cpp` (around line 103) only chooses the chord: the late chord within the slop, or the previous one. `apply_fill_end` (around line 126) attaches the fill to the previous chord if it sits inside the fill. Otherwise it drops the fill. The only readers are the authored-fill handlers at lines 559 and 957. `Song::check_activations` (around line 206) builds generated fills and never reads the slop. The comment on the field in `src/core/rules.h` (line 38) matches the code.

| Situation | Guide implies | Code does |
|---|---|---|
| Next chord within the slop | counts | lands on that late chord |
| Next chord past the slop, a chord inside the fill | does not count | lands on the chord inside the fill |
| Next chord past the slop, no chord inside | does not count | dropped; if all are dropped, generated fills take over |
| Chart with only generated fills | slop applies | never read |

They disagree today. The verifier showed this by reading. Example: resolution 192, a fill ending at tick 3436, chords at 3264 and 3456. The next chord is 20 ticks late, past the default 6-tick slop, so the fill lands on 3264.

On screen, changing the slop moves an authored fill to the next chord. That changes the activation chord, the deadline and the score. It can also swap authored fills for generated ones. The guide line came in afb49c5 (2026-09-24), the same day 7e14401 made the slop a setting at the user's request. `fill_lands_on_chord` should stay the owner, and the guide should match rules.h.

*Owner proposed: fill_lands_on_chord in src/parse/song.cpp (the rule); UserGuide.md line 253 must say what rules.h says. Candidates: R4-A3-1.*

#### 97. A tied variant shows its leader's skip count and E mark, not its own

The question: how many fills did this path pass before an activation? And was the first one E-critical? E-critical means skipping that fill makes it an early fill. This sets the skip digit and "E" in the notation, plus E0, the required-early-fill line and the all-0 test.

Two places answer it. The first is the variant row. `Engine::reduce_group` in `src/search/engine.cpp` (around lines 760-773) folds a tied path into a leader. It keeps the follower's activations but drops its `currentskips` and `skipped_e_offset`. The group key in `Engine::reduce_iteration_paths` (around line 894) leaves skip state out. `Engine::branch_activate` (around line 568) then stamps the next activation with the leader's skips. `Path::prepare_variants` in `src/core/model.cpp` (around line 545) copies that activation into the variant. `notationstr`, `is_E0`, `is_allzero` and `build_preview_scene` in `src/app/preview_view.cpp` (around line 345) all read the copy. The second is `search_target` in `src/search/pather.cpp` (around line 95), which prices the same tick set exactly.

| Path | Variant row (dump) | Exact pricing (target) |
|---|---|---|
| song3e leader, 3840/18432 | 1 1 | 1 1 |
| song3e variant, 3072/18432 | 0 1 | 0 E2 |
| song3 variant | 0 1 | 0 2 |

They disagree today. The verifier re-ran this on a scratch test chart (`scratchpad\r5skips\song3e\notes.chart`) with `hydra_replay dump --cap 2` and `target`. The two agree whenever both paths have the same skip state at the fold.

On screen, the variant's Paths tab row and its Ctrl+C string read '0 1' for a real '0 E2'. Its early-fill line, badge and difficulty can be wrong too. From reading the code, the Preview would show fill F3 (7788) as Hidden. Scores, SP ends and the leader row are correct.

The engine should own skip state. It could carry the follower's skips into the variant, or add skip state to the group key. The fold dates from 1f38f07 (2026-08-13) and 3b69958. No ADR covers this.

*Owner proposed: The engine (src/search/engine.cpp) owns per-path skip state; Path::prepare_variants in src/core/model.cpp should stop copying skips and e_offset from the leader. Candidates: R5-A3-1.*

#### 98. No-path best score is 0 in hydra_replay and hydra_bench but blank in hydra_batch

The question: what is a record's best score when the analysis kept no path?

Two answers exist. `summarize_record` in `src/store/record_store.cpp` (line 498) says there is none. An empty paths list gives an empty summary with no score, and `src/cli/batch.cpp` (lines 249-250) prints "-". Three other places say 0. `emit_dump` in `tools/replay.cpp` (lines 475-476) uses `rec.paths.empty() ? 0 : best_path().totalscore()`. `cmd_target` in the same file (lines 659-661) repeats those two lines word for word, even though `emit_dump`'s comment says "Written once here". `corpus_bench` in `tools/bench.cpp` (line 84) has the same 0 fallback. `build_path_buttons` in `src/app/path_view.cpp` (line 448) has it too, but that part is already covered by known finding display-17.

| Paths list | store / hydra_batch | hydra_replay dump and target | hydra_bench |
|---|---|---|---|
| empty | no score, prints "-" | `result.score` 0, `bestpath` "" | "best score 0" |
| not empty | best path's total | best path's total | best path's total |

They disagree today, shown by reading. `hydra_replay target` with an activation set that `search_target` cannot realize returns an empty list, so its JSON says score 0 with only the `realized` flag as a hint. `build_record_status` (line 125) also says a Ready record can legitimately hold nothing.

The GUI is not affected, because it stops at "No paths found." first. In the CLIs, a reader comparing `result.score` with a video score sees a real-looking 0, while `hydra_batch` prints "-" for the same state.

The 0 came from 3b69958, 30c4008 and 577ddc2. No user decision for it was found in docs/adr, CONTEXT.md or those commit messages. One optional best-score helper should own this, and `emit_dump` and `cmd_target` should share one result-block writer that calls it.

*Owner proposed: src/store/record_store.cpp summarize_record (optional score), or one HydraRecord best-score helper returning std::optional. Candidates: R3-A4-2.*

#### 99. Report pages group thousands by browser locale; the app always uses commas

The question: how is a score or count written with thousands separators?

The C++ side has one answer. `group_thousands` in `src/core/model.cpp` (line 652) always puts a comma every three digits. The app, the CLIs and every report subtitle use it. The report pages' JavaScript has a second answer. The shared formatter `fmt` in `src/app/html_page.cpp` (line 314) calls `toLocaleString()`, so the browser's locale picks the separator. The "#" column and the "N of M" line there do the same. The tiles and cells in `src/app/report.cpp`, `src/app/fill_report.cpp` and `src/app/dm_report.cpp` call it too. The page's `lang="en"` attribute does not change this.

| Number | C++ | en-US | de-DE | fr-FR | es-ES |
|---|---|---|---|---|---|
| 1234 | 1,234 | 1,234 | 1.234 | 1 234 | 1234 |
| 1234567 | 1,234,567 | 1,234,567 | 1.234.567 | 1 234 567 | 1.234.567 |
| 999 | 999 | 999 | 999 | 999 | 999 |

They disagree today in any non-English browser, shown by a run in node. (de-CH also gives 1'234'567.)

On one page in a de-DE browser, the subtitle says "1,234 charts" while the Charts tile says "1.234". On the DM page, "Points left on table" uses "." for thousands right next to "Avg % of optimal", where `toFixed(2)` uses "." as the decimal point.

The call dates to b257836 (2026-08-09). No decision to follow the browser locale was found in docs/adr, CONTEXT.md or the commit messages. `group_thousands` should own the rule. The page data could carry pre-formatted strings, or the shared script could get one fixed-comma formatter that every call goes through.

*Owner proposed: src/core/model.cpp group_thousands. Candidates: R4-A4-1.*

#### 100. A zero time signature is skipped in .chart but crashes Hydra in .mid

The question is: does a time signature whose top number is 0 get ignored, or does it set the measure length to 0?

The two parsers answer differently. `ChartParser::optype` in `src/parse/song.cpp` (around line 863) skips it with a bare `!= 0` check. `MidiParser::optype` (around line 513) passes every time signature on, and `meta_message` in `src/parse/midi.cpp` hands the 0 through unchecked. The shared handler both call, `apply_timesig` (around line 118), has no guard. It writes ticks-per-measure as resolution times numerator, which is 0. `Timecode::Timecode` in `src/core/timing.cpp` (around line 126) then divides by it. The Preview's `build_preview_scene` and `build_beat_events` in `src/app/preview_view.cpp` skip zero-length measures defensively, but that does not save the engine.

| Numerator | .chart | .mid |
|---|---|---|
| 4 | 768 ticks per measure, scores | 768 ticks per measure, scores |
| 0 | line ignored, scores normally | measure length 0, process dies |

The verifier showed this by a run. The .chart file scored byte-identically to its 'TS 4' control. The .mid file made `hydra_replay score` exit with an integer divide-by-zero fault and write nothing.

On screen, such a .mid (or a .sng/.srb holding one) would kill Hydra outright on Analyze, batch, Preview, Dynamics or song length. The GUI crash is inferred from the shared parse path, not run. The app's exception catches cannot see a hardware fault.

`apply_timesig` should own the rule, since both parsers already call it. The .chart check came in with the port, 3b69958 (2026-08-15), from Python truthiness in 8bec164. No user decision was found in CONTEXT.md, docs/adr or commit messages.

*Owner proposed: apply_timesig in src/parse/song.cpp. Candidates: R3-A5-1.*

#### 101. A .sng's preview clip is mixed into the song; a loose folder's is left out

The question is: which of a chart's audio files belong in the song mix the Preview plays? In particular, is a short `preview.*` clip one of them?

Two functions in `src/app/preview_source.cpp` decide. `find_loose_audio` (around line 206) drops any file whose name, lower-cased, is 'preview'. `sng_audio_from` (around line 157) keeps every audio entry in a .sng with no such check. `resolve_preview_source` (around line 344) sends all those stems to `decode_and_mix`, which sums them from the start. The header comment in `preview_source.h` documents the exclusion for loose folders only. The test file pins the loose case, and its .sng fixture has no preview entry.

| File name | Loose folder | .sng |
|---|---|---|
| song.ogg | keep | keep |
| Preview.OGG / preview.opus | drop | keep |
| preview2.ogg | keep | keep |
| album.jpg | drop | drop |

They disagree by reading, not by a run. A .sng holding song.ogg and preview.ogg gives two stems, while the same files loose give one. Nothing differs on screen today: a scan found no preview entry in any of the 54 .sng files in C:\Clone Hero. If one appeared, the Preview would play the clip on top of the start of the song.

One stem predicate in `preview_source.cpp` should own the rule, and both functions should call it. The loose exclusion came with ba0885a. No user decision was found.

*Owner proposed: One song-mix stem predicate in src/app/preview_source.cpp called by find_loose_audio and sng_audio_from. Candidates: R6-A5-1.*

#### 102. ADR 0012 says a ghost kick draws narrowed; the Preview draws it full width

The question is: how does the Preview draw a ghost kick (a very soft kick in a chart with dynamics turned on)?

The ADR says one thing and the code says another. The closing paragraph of `docs/adr/0012-kick-dynamics-are-priced.md` (around line 81) says a ghost kick 'draws as a narrowed bar with the ghost overlay'. `build_highway_draws` in `src/render/highway_draw.cpp` (around line 353) narrows ghost notes only when they are not kicks. The test 'a ghost kick keeps full width and overlays' in `tests/test_highway_draw.cpp` (around line 307) pins the code.

| Note | ADR 0012 | Code and test |
|---|---|---|
| Ghost pad | 70% width plus overlay | 70% width plus overlay |
| Ghost kick | narrowed bar plus overlay | full width plus overlay |
| Accent or normal kick | full width | full width |

They disagree today, by reading. Any ghost kick shows it, such as the 36 in the ADR's own Won't Get Fooled Again example. Commit f3ff7ec (2026-09-27) changed the drawing two days after the ADR was last edited. Its reason was that a shrunken kick stops short of the highway edge.

Nothing on screen is wrong. A reader who trusts the ADR gets the wrong shape. The ADR's claim that this is Clone Hero's own rendering is also unverified.

`build_highway_draws` owns the rule. The ADR should point to it or record the f3ff7ec change. f3ff7ec is a main-session commit with no user quote, so it is not a recorded user decision.

*Owner proposed: build_highway_draws in src/render/highway_draw.cpp (ADR 0012 should point to it). Candidates: R6-A5-3.*

#### 103. hydra_replay dump can never read a stored 1.0-fills result

The question: under which fill rule does a `hydra_replay` run search, and under which rule does it look up the stored record? In the app, one setting answers both. `Settings::legacy_fills` feeds the search through `to_analysis_settings` and the record key through `lens` in `src/app/config.cpp` (around lines 154 and 172).

`hydra_replay` skips that owner. `settings_from` in `tools/replay.cpp` (around line 140) never copies the `--legacy-fills` flag into the Settings. So the key it builds always names the 1.1 rule. Instead, the flag is applied to the search by hand, twice. `cmd_dump` does it around line 516, and `cmd_target` does it again around line 639. By contrast, `hydra_batch` routes the flag through `settings.legacy_fills`, so its search and key agree.

| Run | Search rule | Record looked up |
|---|---|---|
| dump, no flag | 1.1 | 1.1 row |
| dump, `--legacy-fills` | 1.0 | none ("stored rows are not read") |
| GUI, "1.0 fills" on | 1.0 | 1.0 row |

They disagree today, and a run proved it. The verifier stored a 1.0 result for one chart with `hydra_batch --legacy-fills`. `dump` without the flag reported NotAnalyzed and ran a fresh 1.1 analysis. With the flag it reported `analyzed-ch10` and read nothing.

The user sees this in the CLI output. A 1.0 record the GUI shows is never dumped, so dumped paths can differ from the GUI's. No user decision covers `hydra_replay`. ADR 0010's note and commit 1f3efdf cover `hydra_batch` only.

`Settings::legacy_fills` should own it. Set it once in `settings_from`, and drop both hand overrides.

*Owner proposed: Settings::legacy_fills (src/app/config.cpp), set once in settings_from. Candidates: R5-A6-2.*

#### 104. Path report lists every chart mode, but the docs say current mode only

The question is which chart modes the path report lists. A chart mode is the difficulty plus the Pro Drums and 2x Bass options.

The code lists every chart mode stored at the current SP cap and lens. `collect_rows` in `src/app/report.cpp` (line 258) passes `std::nullopt` as the chart mode to `for_each_blob`, which means no filter. That matches user decision 2 in `docs/superpowers/plans/2026-09-26-codebase-audit-fixes.md` (line 81): "The path report gets a Mode column and keeps every row." Two docs still say otherwise. `docs/UserGuide.md` (line 203) says the report "follows the current analysis settings", and the settings bar includes difficulty. `docs/development.md` (lines 37-38) says `hydra_report` reports "at the same chart mode" the app is set to. The other two report pages do filter by chart mode: `collect_dm_rows` in `src/app/dm_report.cpp` (line 157) and `collect_fill_rows` in `src/app/fill_report.cpp` (line 155).

The subtitle from `generate_report` says "top N paths per chart". The cut is really per record, meaning per chart and mode (`shown = min(max_paths, ...)` at line 271). A chart stored in two modes can show 2N rows.

| settings on Hard, Expert and Hard records stored | rows |
|---|---|
| path report code | both modes, ranks 1..N each |
| development.md promise | Hard only |
| fill and DM pages | Hard only |

They disagree today whenever a chart is stored in more than one mode. The verifier confirmed this by reading. The candidate's `hydra_uitest` run on Burnout (6 rows for 3 per mode) was not re-run.

The user sees several rank-1 rows per chart, while the docs promise one mode. The Mode column does tell them apart.

`collect_rows` owns this, per the user's decision. The two doc sentences and the subtitle should describe it: every mode at the current cap and lens, top N per chart and mode.

*Owner proposed: collect_rows in src/app/report.cpp (per user decision 2); the docs and subtitle should follow it. Candidates: R6-A7-1.*

#### 105. Empty report says nothing is stored when records exist under other settings

The question is whether the store holds any analyzed records, as the empty-report message tells the user.

Four places answer it from a zero-row test. `main` in `src/cli/report.cpp` (line 93) prints "No records stored yet. Run hydra_batch first." `ReportJob::run` in `src/ui/library_jobs.cpp` (line 368) throws "no records stored yet". `plain_error_text` in `src/app/user_messages.cpp` (line 42) turns that into "There are no analyzed songs to put in a report yet. Analyze some songs first." `main` in `src/cli/fillcompare.cpp` (line 107) prints "No records to compare. Run hydra_batch into both databases first." But zero rows only means nothing is Ready under this cap, lens, rules and path count. Ready means stored and current for these settings. The rows come from `collect_rows` in `src/app/report.cpp` (line 250).

| store state | message | correct? |
|---|---|---|
| stored and Ready | rows written | yes |
| stored, not Ready under these rules, cap or lens | "nothing stored" | no |
| `--paths 0` | "nothing stored" | no |
| nothing stored | "nothing stored" | yes |

This was shown by a run. `hydra_batch` on Allister - Overrated printed "Store now holds 1 records across 1 songs." `hydra_report` with a different rules file (max_tied_paths = 2) then printed "No records stored yet". So did `--paths 0`. A plain `hydra_report` afterwards still wrote 5 rows. In the GUI the report runs right after a batch with the same settings, so the message shows only when that batch left nothing Ready.

The user is told to run `hydra_batch` right after it reported a stored record. Following that advice does not help.

`generate_report` and `generate_fill_report` should say why there are zero rows. Each caller would print that one answer. Introduced by 3b69958 (2026-08-15), 21ddf53 and e6d8159 (2026-08-18, 2026-09-27) and 406d473 (2026-08-31).

*Owner proposed: generate_report / generate_fill_report in src/app, returning why there are zero rows. Candidates: R4-A7-1.*

#### 106. Batch and report both say 'records across' but count different things

The question is how many records, across how many charts, the database holds. Two lines answer it in the same words but count different things.

`main` in `src/cli/batch.cpp` (line 260) prints "Store now holds N records across M songs". It reads `RecordStore::counts` in `src/store/record_store.cpp` (line 1593), which counts every row in the whole database. That includes rows for other SP caps, other chart modes and stale rows. `generate_report` in `src/app/report.cpp` (around lines 377-403) writes "N records across M charts". It counts only the Ready records it lists under the current cap and lens.

| database | batch line | report subtitle |
|---|---|---|
| one cap, all Ready | N records, M songs | N records, M charts |
| rows at cap 4 and cap 5 (run) | 3 records, 2 songs | 2 records, 2 charts |
| a stale row (by reading) | counted | skipped |

They disagree today on any database holding more than one setting. The verifier ran it on a scratch copy with an extra cap-5 row. `counts()` gives 3 results, and `hydra_report` printed "2 records across 2 charts". This is not one fact derived twice. It is two scopes with no label.

The user sees a larger record count in the console than in the path report for the same database. Neither line says what it counts. The app's real hydra.db holds several caps and modes, so this shows up right after a normal batch.

The batch line should either name its scope ("rows for every setting") or ask the store for the same Ready count `generate_report` uses. The batch line came from 3b69958 (the C++ port). The report subtitle came from 5457d7d and was reworked in ba0885a. No user decision was found for either scope in docs/adr, CONTEXT.md or the commit messages.

*Owner proposed: src/cli/batch.cpp closing line (scope it) or RecordStore, sharing generate_report's Ready scope. Candidates: R3-A7-3.*

#### 107. hydra_replay dump's 'not analyzed' line echoes typed settings, not the ones it used

The question is which cap, ms limit and depth mode `hydra_replay dump` looked a record up under, as it tells you on stderr when nothing is stored. Two places answer it. `cmd_dump` in `tools/replay.cpp` (around line 549) prints the raw command-line text: `a.cap`, `a.ms`, `a.depth_mode`, `a.depth`. `settings_from` in the same file (lines 140-160) decides what the lookup really uses. It runs `atoi` on the cap and ms, and treats depth mode as Points only if the text is exactly lowercase "points".

| typed | used by lookup | printed |
|---|---|---|
| --depth-mode points | Points | points |
| --depth-mode Points | Scores | Points |
| --ms 10.5 | 10 ms | 10.5 |
| --ms abc | 0 ms, limit on | abc |
| --cap 4x | 4 | 4x |

They disagree today for any input not already in canonical form. A run showed it: `dump --depth-mode Points --ms 10.5 --cap 4x` printed "cap 4x, ms 10.5, depth Points/4", while the output JSON said `sp_cap` 4 and the lookup used Scores. The line came in with 30c4008 (2026-09-03).

This is CLI text only. A user who typed `Points` is told no Points row exists, when dump really searched for a Scores row and then printed fresh Scores paths without saying so.

The line should be built from the parsed Settings (`s.sp_cap`, `s.lens()`), using the wording the app already has (`within_label` in path_view or `batch_settings_summary` in library_dialogs). Then it can only describe what was actually looked up.

*Owner proposed: tools/replay.cpp cmd_dump, describing the parsed Settings with the app's own settings wording. Candidates: R3-A8-3.*

#### 108. bench --dump-db writes absolute backslash paths; --scan --dump writes relative ones

The question is how a scanned chart's path is written in the bench tool's comparison JSON. `tools/bench.cpp` has two writers for the same row. `scan_mode` (lines 147-168) runs each path through a `relify` step: it strips the `--dump-rel` root and turns backslashes into forward slashes. `dump_db` (lines 220-227) writes `notespath` and `rootfolder` exactly as stored. Its own comment (lines 210-211) promises "the same JSON shape --dump writes, so two scans' results can be diffed".

| case | scan_mode path | dump_db path |
|---|---|---|
| --dump-rel root given | relative, forward slashes | absolute, backslashes |
| no --dump-rel | absolute, forward slashes | absolute, backslashes |
| nested folder a\b | a/b | a\b |
| top-level folder "." | . | . |

They disagree today, shown by a run. Scanning `testdata\input\common\IB24\T1` with `--dump-rel` gave `"Allister - Overrated/notes.mid"`, while `--dump-db` on the same database gave the full `C:\\Users\\...` path with backslashes. The keys match; the values do not. Both writers arrived in 21ddf53 (2026-08-18).

Nothing changes in the app. A developer diffing a scan dump against a database dump sees every row flagged as different.

One row-to-JSON function in `tools/bench.cpp` should own this, applying `relify` the same way for both modes. `dump_db` would need a `--dump-rel` argument too.

*Owner proposed: tools/bench.cpp, one row-to-JSON function shared by scan_mode and dump_db. Candidates: R3-A8-4.*

#### 109. hydra_uitest wait-idle keeps its own job list and misses three background jobs

The question is whether every background job the GUI started has finished, so a test script can read the screen. `jobs_busy` in `tests/ui/uitest_harness.cpp` (line 310) answers it with its own list. It checks the scan, batch, analyze, report and two DM jobs, plus `preview->loading()`. For batch and analyze it retypes the expressions inside `AppState::batch_running` and `analyze_running` (`src/ui/app_state.cpp` lines 256-258) instead of calling them. It leaves out `dynamics_job`, `length_job` (the song-length backfill) and the Preview path-switch `scene_job_`, because `loading()` only looks at `job_`. The `wait-idle` command in `tests/ui/uitest_script.cpp` (line 75) waits on this list. `docs/agents/ui-testing.md` names the same list but calls it "every background job".

They disagree today, shown by a run. Opening Acid Romance unanalyzed, clicking Dynamics, then `wait-idle` and `text` showed "Reading chart..." while `state` reported every job idle. After `wait 2.0` the tab showed "Ghosts: 5". The backfill only runs for pre-1.9 records with no stored song length. The parked DM jobs are superseded, so skipping them has no effect.

A check script can read stale text and report it as the result: the Dynamics tab mid-load, or a Preview overlay not yet rebuilt.

One `AppState` query, for example `any_job_running()`, should own the list. It sits next to the two existing queries, so a new job gets added in one place, and the doc can describe that one list.

*Owner proposed: src/ui/app_state.cpp, one any_job_running() query next to batch_running/analyze_running. Candidates: R5-A8-2.*

#### 110. hydra_replay's squeeze-out warning assumes first_note even when whole_chord is chosen

The question: how many points does an unrecorded squeeze-out overstate a replayed score by? A squeeze-out is a timing trick that drops a chord out of Star Power, so its SP doubling is lost.

`category_scores` in `src/core/scoring.cpp` (lines 74-76) owns the cost. It follows the user's `sqout_rule` from `hydra_rules.ini` (`src/core/rules.h`, lines 20 and 29). Under `first_note` only the first note loses its doubling. Under `whole_chord` every note does, as `docs/UserGuide.md` (line 248) says. `ambiguous_window_warnings` in `src/core/replay.cpp` (line 293) states the cost in words instead: the score is "high by that note's first-hit share". It takes no rules. `tools/replay.cpp` prices the path with the loaded rules (line 340) but calls the warning without them (line 345). The comments in `src/core/replay.h` (lines 47-48 and 222) repeat the first-hit wording.

| Chord at 1x | Rule | Real overstatement | Warning implies |
|---|---|---|---|
| Red + Blue | first_note | 50 | 50 |
| Red + Blue | whole_chord | 100 | 50 |
| any 1-note chord | either | one note | one note |

Today no `hydra_rules.ini` sets `whole_chord`, so they agree in practice. With `whole_chord` set, they split for any chord of two or more notes. This was shown by reading the code and the test in `tests/test_rules.cpp` (lines 140-146), not by running the tool.

Only `hydra_replay score`'s stderr warning is affected. It would name a smaller error than the real one. The GUI uses the rule-aware value.

The text was written in 2045a21 (2026-09-09), when `first_note` was the only rule. It went stale when 7e14401 (2026-09-24) added the user's rule choice. The rule itself is the user's choice; nothing says the warning should assume `first_note`. `category_scores` should own the cost, and the warning should take the rules and quote that chord's `sqout_reduction`.

*Owner proposed: src/core/scoring.cpp category_scores / CategoryScores::sqout_reduction. Candidates: R4-A2-1, R4-A4-2.*

#### 111. Chart titles are cleaned up for display in several places, each a different way

The question is: what title text does Hydra show for a chart? That means removing Clone Hero markup (colour tags), trimming spaces, and falling back to '(unknown)' when nothing is left.

The '(unknown)' fallback runs at different points. `discover_charts` in `src/app/analysis.cpp` (around line 479) applies `title_or_unknown` to the raw title at scan time. The library row (`LibraryModel::set_charts` in `src/ui/library_model.cpp`) and the details header (`render_header` in `src/ui/details_panel.cpp`) strip markup afterwards with no second fallback. The path, fill and DM report pages strip, trim, then fall back again (`collect_rows` in `src/app/report.cpp`, `collect_fill_rows`, `collect_dm_rows`).

Four GUI sentences skip the markup removal entirely. They are the analyze-busy tooltip in `render_headline`, the 'Could not analyze' line in `AppState::reap_analyze_job`, and the batch dialog's 'Now:' line, whose title and artist come raw from `BatchJob::note_started`.

| Title in song.ini/.sng | Library row and header | Report pages | Tooltip, status, batch line |
|---|---|---|---|
| `Song` | Song | Song | Song |
| `<color=#ff0000></color>` | blank | (unknown) | raw tags |
| `   ` (.sng only) | blank | (unknown) | spaces |
| `<color=#FFD700>Gold</color> Song` | Gold Song | Gold Song | raw tags |

This comes from reading the code, not a run. The user would see one chart as blank in the library and '(unknown)' on the reports, or see raw colour tags in the tooltip, status line and batch dialog.

One display-title helper should own this, and every surface should call it. The fallbacks came in with 06a178c (2026-09-25), the library strip with f1c6a3d/ee6e34c (2026-09-27). No user decision was found.

There is a recorded decision here, and the code no longer follows it. Commit 06a178c (2026-09-25) records your decisions 10, 22 and 27: a song with no usable name reads "(unknown)" in the library, Song Details and all three reports, through one fallback. Commit ee6e34c (2026-09-27) gave the library model `strip_rich_tags` with no fallback, which broke that. The report side was shown by an earlier run, whose report.html holds "song":"(unknown)" for a markup-only title; the library side comes from reading `strip_rich_tags`. The natural home is a display-title function in `src/parse/song.h`, next to `title_or_unknown`, that strips, trims and then falls back.

*Owner proposed: One display-title helper (title_or_unknown on trimmed, strip_rich_tags'd text) called by every surface that names a chart. Candidates: R3-A5-2, R3-A5-3, R3-A7-1.*

### Visibility 3: visible only if the copies drift later

#### 112. Score range box assumes 0 is the widest digit

The question is how wide six digits can get, so a number box never clips.

`widest_digits` in `src/ui/widgets.h` (around line 70) answers it properly. It measures each digit in the current font and repeats the widest one. The library toolbar and the Preview tab use it. `render_score_range` in `src/ui/settings_bar.cpp` (around line 113) answers it again with the literal "000000". That bakes in the guess that 0 is the widest digit.

They disagree today. The verifier read the shipped font, `resource/ShipporiAntiqueB1-Regular.ttf`, with a small script. In that font 8 is widest (637 units against 630 for 0). At 18 px, "888888" is about 47.5 px and "000000" is about 47.0 px. So the Score range box is about half a pixel (times DPI scale) narrower than six 8s.

On screen this is practically invisible, because frame padding absorbs it. The real cost is that the width silently depends on the font's digit shapes; a font change could make it clip.

It came in with 9b4352f (2026-09-27, "Score range fits six digits"). `widest_digits` already existed since eeb5d38. The commit says only "wide enough for six digits"; nothing chose 0 as the sample.

`widest_digits` should own this. `render_score_range` should call `widest_digits(6)`.

*Owner proposed: widest_digits in src/ui/widgets.h. Candidates: sweep-ui2-c14.*

#### 113. A second, unused library search in SQL matches different charts

The question: which library charts match a typed search? The library's own answer lives in `src/app/library_query.cpp`. `term_matches` (around line 283) matches a plain word against title, artist, charter or folder. `make_searchable` strips color tags, and `fold_into` folds case and accents. `parse_library_query` turns words like `stars:5` into filters.

A second answer lives in SQL. `RecordStore::chart_library_count` and `RecordStore::list_chart_library` in `src/store/record_store.cpp` (around lines 1674 and 1691) each spell a bare `LIKE` over name, artist and charter. `LIKE` is SQL's wildcard text match.

| Search | Library | SQL |
|---|---|---|
| `metal`, only the folder says Metal Pack | match | no match |
| `a_b` vs title "axb" | no match | match (`_` is a wildcard) |
| `cafe` vs "Café" | match | no match |
| `color` vs `<color=red>X</color>` | no match | match |
| `stars:5` | stars filter | literal text |

They disagree today, shown by reading. But the SQL path is dead. Only the `BatchJob` search-string constructor in `src/ui/library_jobs.cpp` reaches it, and nothing builds that. Every live caller passes no search. So the user sees nothing now. A future caller would analyze a different set of charts than the library shows.

The SQL rule came with the C++ port (3b69958). Commit a591bba steered the batch around it but left it in place. The only reason on record is a code comment. `query_matches` in `library_query.cpp` should own it, with the SQL search parameter and the unused constructor removed.

*Owner proposed: src/app/library_query.cpp query_matches / term_matches. Candidates: R3-A6-1.*

### Visibility 4: not visible (internal or test-only)

#### 114. The code reuses the glossary term 'frontend squeeze' for a different idea

The question is what "frontend squeeze" means.

CONTEXT.md (around lines 90-92) defines "Frontend squeeze" as hitting the activation note first, so the chord's other notes score under SP. The code means something else. The comments in `src/core/squeeze_rating.h` (around lines 120-124) and `rate_activation` in `src/core/squeeze_rating.cpp` (around lines 174-177) say "a squeeze the frontend decides". That is any SqIn/SqOut, or a backend row that was squeezed out or is not counted. The comment in `build_activations` in `src/app/path_view.cpp` (around line 262) uses the same phrase. The variable `has_frontend_squeeze` in `rate_activation` uses the glossary's exact term for the code's meaning.

They disagree today, but only in naming. The verifier established this by reading. No rule or number is computed twice, and no behavior differs.

Nothing shows on screen. A reader who trusts the glossary would misread the variable.

CONTEXT.md owns the term, so the variable should be renamed to match the comments' wording. The glossary entry came from ba0885a (2026-08-22). The comment wording came from 396082a (2026-09-21), also touched in 283e028. No decision behind the code's meaning was found.

*Owner proposed: CONTEXT.md (glossary owner); rename the code variable to match. Candidates: squeeze-33.*

#### 115. Preview comment says the ms index stops short; it covers every tick

The question is what millisecond a tick past the last note falls at.

`MsIndex::at` in `src/core/timing.cpp` (lines 47-54) answers it for every tick, and it extrapolates past the last tempo mark. The comment in `build_preview_scene` in `src/app/preview_view.cpp` (lines 296-299) says otherwise. It says the ms index "does not reach" a section marker past the last note, and that "the timing's own timecode extrapolates instead". The code then calls `song.timecode(t).ms()`. That call ends in the `Timecode` constructor (timing.cpp line 140), which sets `ms_ = ms.at(ticks_)`. That is the same `MsIndex::at`. Line 283 of preview_view.cpp already calls `timing.ms_index().at` directly.

The two paths agree for every tick because they are one call. This was shown by reading. Nothing shows on screen. The risk is that the comment invites someone to write a third, separate extrapolation.

`MsIndex::at` owns this. Only the comment needs fixing.

*Owner proposed: MsIndex::at (src/core/timing.cpp). Candidates: tempo-19.*

#### 116. The same stored result decodes with two different fill-rule flags

The question is which fill rule a decoded result carries in its `legacy_fills` field. Decoding means turning a stored result back into a `HydraRecord` in memory.

The field is set at analysis by `analyze_chart` in `src/search/pather.cpp` (line 193). When a result is read back, `RecordStore::get_record` in `src/store/record_store.cpp` (around line 1210) sets it from the row's key. `RecordStore::for_each_blob` (around line 1407) does not. It leaves the field at its default of false, even when the walk was asked for 1.0 rows. `reindex` and `fill_missing_stars` also skip it, but they never read the field.

They disagree today, shown by reading. Walk a 1.0 row with `for_each_blob` and the record says `legacy_fills=false`. Fetch the same row with `get_record` and it says true.

Nothing shows on screen today. The report walk in `collect_rows` (`src/app/report.cpp`) never reads the field, and the only reader is `prepare_row` (around line 525). If a record from the walk were ever passed back to `prepare_row` under a 1.0 lens, the write would be refused.

Introduced in 1f3efdf (2026-09-29). ADR 0010's note says only that `get_record` sets it from the key. `RecordStore` should own one decode helper used by every read path.

*Owner proposed: RecordStore (one decode helper that rebuilds the record and sets legacy_fills from the row's lens). Candidates: store-13.*

#### 117. .sng and .srb test fixtures are hand-encoded in several test files and disagree on the .sng metadata-length field

The question is how .sng and .srb song containers are laid out byte by byte when a test builds one.

The readers own the layout. They are `src/parse/sng.cpp` (lines 30-91) and `src/parse/srb.cpp`, with constants in `src/parse/sng.h` (lines 31-34) and `src/parse/srb.h` (`kSrbHeaderSize`). The .sng constants put the XOR mask at offset 10, make it 16 bytes, put the metadata length at offset 26, and start the metadata at 34.

Four test files build these containers by hand with bare numbers instead. `tests/test_sng.cpp` has `push_u32` and `push_u64` (lines 27 and 30) and `make_sng` (lines 38-73). `tests/test_srb.cpp` has `deflate_raw` (line 66), `push_str` (line 78), `make_metadata` with the `4b4\x01` prefix (lines 84-88) and `make_srb` (lines 103-121). `tests/test_preview_source.cpp` repeats all of them: `push_u32` and `push_u64` (lines 62 and 65), `deflate_raw` (69), `make_sng` (lines 94-146), `push_str` (150), `make_metadata` (155) and `make_srb` (lines 167-181). The fourth file is `write_sng_with_metadata` in `tests/test_analysis.cpp` (lines 327-344).

The members disagreed on whether these helpers match today. One said they all match the readers and differ only in fields the readers ignore. The other said one header field differs. I checked the code, and the second is right. `write_sng_with_metadata` writes a 34-byte header of zeros after "SNGPKG", so the metadata-length field at offset 26 is 0. Both `make_sng` copies write the real metadata size there.

| Helper | Writes at offset 26 | File section after metadata |
|---|---|---|
| `make_sng` (test_sng) | real metadata size | length 8 + entries |
| `make_sng` (test_preview_source) | real metadata size | length 8 + entries |
| `write_sng_with_metadata` | 0 | none |

Nothing fails today. The test that uses `write_sng_with_metadata` only calls `sng_read_metadata`, which ignores offset 26. But `sng_read_file_table` (sng.cpp lines 57-59) does read it. With 0 there, it would start the file section at byte 34 and misread the table. The other differences really are ignored: the file-section length field is skipped by the reader (line 61), filler bytes vary, and `srb_parse_metadata` in `srb.cpp` (lines 51-52) does not check the `4b4\x01` prefix at all.

Nothing changes on screen; these are test fixtures. A real format change would need edits in every copy, and a missed one could keep passing against a wrong layout.

One shared fixture header beside `tests/midi_util.h` should own the byte writers, `deflate_raw`, `make_sng` and `make_srb`, built on the readers' constants. The copies date from 21ddf53 (2026-08-18), ba0885a (2026-08-22, the test_preview_source `make_sng`), 5cb82ae (2026-09-25) and 06a178c (2026-09-25, `write_sng_with_metadata`).

*Owner proposed: One shared test fixture header beside tests/midi_util.h, with one encoder per container built on the constants in src/parse/sng.h and src/parse/srb.h. Candidates: parse-24, sweep-tests3-c8.*

#### 118. One comment says the gem light sits at the bottom centre; code uses the top

The question: what point on a gem is its light offset measured from?

`light_for` in `src/render/highway_draw.cpp` (around line 104) adds the offset to the top of the gem's box. Y points up in this config, so the larger y is the top. Three comments agree: `highway_draw.cpp` line 101 and `highway_draw.h` lines 54 and 100 all say 'top centre'. `tests/test_highway_draw.cpp` (around lines 137-140) pins the top too. One comment disagrees: `PreviewConfig::Gems::light` in `src/render/preview_config.h` (line 64) says 'relative to the gem's bottom centre'.

| Box y from -1.25 to -0.75, offset 1.0 | Light y |
|---|---|
| code and test (top) | 0.25 |
| preview_config.h comment (bottom) | -0.25 |

The code and the comment disagree today, shown by reading the code and the test. Nothing changes on screen, because only a comment is wrong. A reader who trusts that comment while tuning the light in `3d-config.json` would be misled.

Both wordings came in ba0885a (2026-08-22). No decision is needed; this is a comment error. Onyx's own source was not checked. `light_for` owns the rule, and the comment in `preview_config.h` should say 'top centre'.

*Owner proposed: light_for in src/render/highway_draw.cpp; fix the comment in src/render/preview_config.h. Candidates: sweep-media1-c14.*

#### 119. Test helpers match chart extensions case-sensitively while chart_format_of ignores case

The question is which chart format a file is, judged by its extension. `chart_format_of` in `src/parse/chart_files.cpp` (line 9) owns the answer, and it ignores case.

Several tests ask again with a case-sensitive suffix check. `first_chart_with_suffix` in `tests/corpus_util.h` (line 58) uses `ends_with`. About fifteen test files call it, including `test_srb.cpp` for .sng and .srb. `small_chart` in `tests/test_cli.cpp` (around line 97) compares the last six characters with ".chart". Three tests check for .mid directly: "song parse holds its invariants at Hard too" in `tests/test_song.cpp` (line 75), ".mid: each difficulty reads its own pitch base" (line 139), and "midi: every corpus .mid reads with a sane structure" in `tests/test_midi.cpp` (line 59).

| Path | `chart_format_of` | Test helpers |
|---|---|---|
| a/notes.chart | Chart | match |
| a/notes.CHART | Chart | skipped |
| a/notes.mid | Mid | match |
| a/NOTES.MID | Mid | skipped |
| mid (no dot) | not Mid | not .mid |

The rules differ today on any upper- or mixed-case extension, shown by reading. `tests/test_strutil.cpp` (line 31) pins `ends_with("song.MID", ".mid")` as false. No real file triggers it yet: a case-sensitive scan of testdata/ found 105 chart files, all lowercase.

Nothing changes on screen. These are test counters, filters and chart pickers. If a file named NOTES.MID or notes.CHART joined the corpus, discovery would list it but these tests would silently skip it.

`chart_format_of` should own this. The tests can compare `chart_format_of(path) == ChartFormat::Mid`, and `first_chart_with_suffix` could take a `ChartFormat` instead of a suffix.

`first_chart_with_suffix` came in e80d267 (2026-08-18). The test_song.cpp line 75 check came in 133b52e (2026-08-28). `small_chart` came in 27c6856 (2026-09-26). 682e924 (2026-09-26) moved the helper onto `ends_with` but kept it case-sensitive. Plan Task 16 in `docs/superpowers/plans/2026-09-26-codebase-audit-fixes.md` names the helper but does not decide on case. No user decision on case handling was found in docs/adr/ or CONTEXT.md.

*Owner proposed: chart_format_of in src/parse/chart_files.cpp. Candidates: sweep-tests1-c15, sweep-tests3-c5.*

#### 120. Panel-split test re-derives the library width and drops one limit each time

The question is: how wide is the library when the song panel is open?

The app answers it in `render_library_and_panel` in `src/ui/library_view.cpp` (around line 44). It clamps the share of the room between a 320 px floor and "room minus 820 px for the panel". The 320 has no name. `test_panel_split` in `tests/ui/uitest_details.cpp` re-derives the width twice. Line 408 keeps the 820 ceiling but drops the 320 floor. Line 443 keeps the floor but drops the ceiling. `test_library_layout` in `tests/ui/uitest_library.cpp` (line 419) types 321 px, which is the floor plus 1 px of slack.

| Room (px) | App | Test line 408 | Test line 443 (share 0.3) |
|---|---|---|---|
| 1264 (harness) | 444 | 444 | — |
| 1160 | — | — | app 340, test 348 |
| 1100 | 320 | 280 | — |

They agree at the harness's 1280 px window, and both tests pass. Below about 1140 px of room, or at a larger UI scale, they split. That was worked out by reading.

Nothing changes in the app. The test would fail falsely on a narrower harness. The 320 came in 99cbda2 (2026-09-27). The redesign plan sets it in task code; no ADR or user decision names it.

A named minimum width beside `kMinSongPanelW`, or one function that returns the clamped width, should own this. The app and both tests would call it.

*Owner proposed: a named minimum library width (e.g. kMinLibraryW in src/ui/details_view.h), ideally a split-width function in library_view. Candidates: sweep-tests4-c5.*

#### 121. Codec test's path-summary equality check skips stars; the store test's checks it

The question is which `PathSummary` fields must match for two stored summaries to count as the same. A path summary is the short per-result record: score, path count, best path's stars and so on. Two test helpers, both named `diff_summary`, answer it. The one in `tests/test_path_codec.cpp` (line 67) compares nine fields and has no stars line. The one in `tests/test_store.cpp` (line 70) compares the same nine plus stars. `PathSummary` in `src/store/record_store.h` (line 54) has ten fields and no shared compare.

| summaries differ only in | test_path_codec | test_store |
|---|---|---|
| stars 5 vs 4 | same | "stars" |
| stars set vs unset | same | "stars" |
| any of the other nine | differ | differ |

They disagree today, shown by reading. Commit bc01fa6 ("each result keeps its best path's star count") added stars to the struct and to the store test's copy only. The codec copy was never updated. `summarize_path` does fill stars, so the codec test's summaries carry a field it ignores.

Nothing changes on screen. But a codec round trip could change the stored star count and still pass. The Stars tab, the `stars:` filter and the library row all read that count.

`PathSummary` itself should own its equality, through an `operator==` or a field list beside the struct. Then adding a field updates every compare at once.

*Owner proposed: src/store/record_store.h, an operator== or field list beside PathSummary (or one shared test header). Candidates: R5-A8-1.*

#### 122. Two container-read tests define 'same Song' with different fields

The question is whether a chart read out of a container (.sng or .srb, bundled song files) parses into the same Song as the notes file read directly. Two tests answer it differently. `songs_equal` in `tests/test_srb.cpp` (lines 123-138) compares tick resolution, meter and tempo maps, features, sequence size, and per note the tick, chord code, solo and SP flags and activation length. The case "sng: the note loader reads the chart through the shared reader" in `tests/test_sng.cpp` (line 139) checks only sequence size and each chord's tick. Neither compares `sp_phrase_start`, `timesig_changes` or `practice_sections` from `src/parse/song.h`, which the app reads.

| Songs differ only in | test_sng | test_srb |
|---|---|---|
| chord code, solo/SP flag, fill length | same | different |
| tempo, meter, features | same | different |
| sp_phrase_start, time signature, practice sections | same | same |

Both tests pass today. The disagreement is between the two definitions, shown by reading; no loader bug was run.

Nothing shows directly, since this is test code. A .sng loader regression that kept ticks but lost cymbal flags, SP phrase ends or fill lengths would pass the .sng test. It could then reach the Paths tab and the Preview.

One shared helper, such as `tests/song_equal.h`, should own "same Song". It should cover every field the app reads, so both container tests check the same thing.

*Owner proposed: one shared test helper (for example tests/song_equal.h) used by test_srb.cpp and test_sng.cpp. Candidates: R6-A8-1.*

## Double readings of one stored fact (13)

Here one fact is stored in two places and code reads both copies. They agree today, because both copies come from one computation.

### Visibility 1: wrong or contradictory on screen today

#### 123. Song panel keeps an old result after a batch updates the same chart

The question is simple: does the open chart have a current result, and what is it? Two cached copies answer it. The library row reads its summary from the store. The song panel reads `AppState::viewed`, plus a stash of earlier lookups called `parked_lookups_`.

They refresh on different triggers. `refresh_viewed_record` in `src/ui/app_state.cpp` (around line 200) runs only on select, a chart-mode change, or a finished single-chart analysis. During a batch, `tick_library` (around line 112) re-reads the library rows about once a second but never touches `viewed`. When the batch finishes, `update_background_jobs` (around line 456) starts the report and refreshes nothing else. The single-chart path, `store_finished_analysis` (around line 539), clears the stash and refreshes both copies. So the rule exists, but only one writer follows it.

| Case | Library row | Song panel |
|---|---|---|
| Panel open on an unanalyzed chart, batch stores it | Ready | "Not analyzed yet." |
| Same, chart was Out of date | Ready | "Out of date ... Re-analyze" |
| Viewed at cap 5 (Out of date), batch at cap 4 purges that row, switch back to 5 | Not analyzed | "Out of date" |

The copies disagree today. The verifier showed this by reading the code, not by a GUI run. The purge in case 3 is `write_row` in `src/store/record_store.cpp` (around line 937), which deletes every unreadable row for the chart at any cap.

On screen, the panel asks the user to analyze a chart the batch just finished. That invites a wasted re-run.

`AppState` should own one "the store changed for chart X" hook. Both the single-analyze path and the batch would call it, so both copies always refresh together.

*Owner proposed: AppState (src/ui/app_state.cpp): one 'store changed for chart X' hook. Candidates: R4-A6-1.*

### Visibility 2: visible in an edge case, in docs or in CLI text

#### 124. Post-batch report reads live settings instead of the batch's own snapshot

The question is which SP cap and lens the path report should list after a batch finishes.

The batch takes a snapshot when it starts. `AppState::start_batch` in `src/ui/app_state.cpp` (around line 443) hands `settings.batch_run()` to the `BatchJob`. The report does not use that snapshot. `AppState::update_background_jobs` (around line 464) builds the `ReportJob` from the live `settings.cap_query()` and `settings.lens()`. So the same fact is held twice, and each side reads its own copy.

While the batch runs, the settings lock keeps the two equal. They can split in a one-frame race, shown by reading and not by a run. The lock and the report start read the batch's finished flag at two different moments in a frame. Say the batch ran at SP cap 4 and finishes mid-frame. A single click on the SP cap "+" button in that frame makes the next frame's report ask for cap 5. The "1.0 fills" and "Path limit" checkboxes can do the same.

On screen, the report would list records at another cap or lens than the batch just analyzed, so some charts would look missing or out of date. The race window opened in a591bba (2026-09-27), which moved the report start out of the modal batch dialog. ADR 0009 says every report states which settings it wants, but not whose snapshot.

`BatchJob` should keep and expose its `BatchRun`, and the report should be built from it.

*Owner proposed: BatchJob (keep and expose its BatchRun for update_background_jobs). Candidates: sweep-ui1-c7.*

#### 125. Batch confirm counts cached library chips; the batch re-asks the store

The question is how many charts in the batch already have a current result under the active settings.

The confirm modal and the batch answer it by two reads. `AppState::open_batch_confirm` in `src/ui/app_state.cpp` (around line 431) takes `library.counts().analyzed`, which counts Ready chips in the cached library summaries. `render_batch_confirm` in `src/ui/library_dialogs.cpp` (around line 282) turns that into "Analyze N charts that have no result yet?" and the Start button's state. `run_batch` in `src/app/analysis.cpp` (around line 550) instead asks SQLite fresh through `analyzed_hashes`, using `analyzed_filter` and `row_ready_sql` in `src/store/record_store.cpp`.

The two Ready rules agree on every row state the verifier could build. Three existing tests pin them together, and they passed in a run. So the claim that the rules themselves differ was refuted.

What remains is a cache against a fresh read. The library snapshot refreshes only at reload, on a settings change, on batch progress and after a single analyze. If something else writes the same `hydra.db` in between, such as `hydra_batch`, the numbers split. That case was found by reading and was not run. On screen, the confirm question would promise N charts while the strip's "skipped" count says something else.

The store's Ready rule should be queried once. Either the confirm asks `analyzed_hashes` over the batch scope, or `run_batch` takes the skip set the confirm counted.

*Owner proposed: the store's Ready rule, queried once (analyzed_hashes over batch_scope, or one shared skip set). Candidates: sweep-ui1-c3.*

#### 126. Preview re-parses the chart file without checking it matches the viewed record

The question is which copy of the chart the screens draw from. The Paths tab reads the stored record. The Preview reads the chart file again.

`RecordStore::get_record` in `src/store/record_store.cpp` (around line 1213) decodes the stored tempo map and restores the activation times. `render_path_panel` in `src/ui/paths_tab.cpp` (line 554) hands that map to `build_activations` in `src/app/path_view.cpp`. The Preview instead re-parses the file at `entry.notespath` (`src/ui/preview_load_job.cpp`, lines 34-35). `build_preview_scene` in `src/app/preview_view.cpp` (around line 320) then works out measures and times from that fresh parse.

They agree today for any file that still matches its scan. The stored tempo map is a lossless copy of the parse. Both sides build it from the same three inputs, so the measures always match. The candidate's other two worries do not hold. A record with no songmeta row cannot happen in the shipped app, because `save_analysis` writes both rows in one transaction. The two reads inside `build_activations` (lines 197 and 207) use one map.

The one real gap is an edited chart file that was not rescanned. Nothing checks the file's md5 against the record's hash. The verifier found no such check in `preview_load_job.cpp` or `preview_source.cpp`. This was shown by reading, not by a run.

In that case the Preview's notes, measures and times come from the new file. The overlaid path and the Paths tab come from the old record. The user would see activations land on the wrong notes.

The Preview load should own the check. It should compare the parsed chart's md5 with `viewed.hyhash` and treat a mismatch as stale. This is close to the known parse-17, which covers chart length, not the notes or tempo map. No user decision was found; docs/adr has no mention of tempomap or songmeta.

*Owner proposed: the Preview load path (src/ui/preview_load_job.cpp / src/app/preview_source.cpp), pinning the parse to viewed.hyhash. Candidates: R4-A7-2.*

### Visibility 3: visible only if the copies drift later

#### 127. The SqOut's offset is stored twice and the display reads both copies

The question is how far the squeezed-out note sits from the SP end.

`ScoreGraph::add_deact_edge` in `src/search/graph.cpp` (around lines 355-362) computes that number once. It then writes it twice: into the backend row's `offset_ms` and into the edge's `sqinout_timing`. The engine's rebuild in `src/search/engine.cpp` (around lines 1195 and 1209-1216) copies the row and the `SPSqueeze` out separately. `path_codec.cpp` stores both as f64.

Two readers use both copies. `rate_activation` in `src/core/squeeze_rating.cpp` rates the row from `offset_ms` and the note from `sq.difficulty()`. `squeeze_sentences` in `src/app/path_view.cpp` writes the SqOut sentence from `sq.timing()` (around line 66) and prices it from the row's `offset_ms` (around line 80). `windows_for_path` in `src/core/replay.cpp` is not a second reader; it only checks that a SqOut exists.

They agree today. Both come from one double, and the verifier found no input where they differ.

If the copies ever split, through a codec change or a trim, the SqOut sentence's ms and the row's timing or eff. figure would disagree for one squeeze. The "judged at two ends" half of this candidate is the separate SqOut-end finding.

The engine copy-out should store one SqOut offset that both the row and the sentence read. The double write was not traced past the C++ port. No decision about keeping two copies was found; ADR 0014 covers `sqout_tick`, and the verifier did not open it.

*Owner proposed: engine copy-out / Activation: one stored SqOut offset. Candidates: squeeze-5.*

#### 128. Whether an activation squeezed out is stored twice and readers mix both

The question is whether an activation ended on a squeeze-out.

The record stores the answer twice. `rebuild` in `src/search/engine.cpp` (lines 1208-1232) writes a SqOut entry into the squeeze list and also sets `sqout_tick`. Readers pick one or the other. `windows_for_path` in `src/core/replay.cpp` (around line 188) takes the offset from the SqOut entry, then skips the window if `sqout_tick` is missing. `Activation::is_sqout_backend` in `src/core/model.cpp` (around line 576) looks only at `sqout_tick`. `squeeze_sentences` in `src/app/path_view.cpp` (around line 68) picks its sentence from the SqOut entry but finds the row through the tick. `windows_from_json` in `tools/replay_json.cpp` (around line 46) reads both separately. The SqOut offset can also be worked out from `sqout_tick` and the SP end tick, so it is in effect stored twice too.

They agree today. The engine writes both in one place. The corpus test in `tests/test_search.cpp` (around line 858) checks they match and passes. Older dumps without the tick are handled on purpose: the replay resolves the chord itself, and stored records read Stale.

If they disagreed, the details view would print a squeeze-out sentence with no squeezed-out row, or the reverse.

`sqout_tick` should be the one stored fact, because ADR 0014 made it the record's truth. The tick arrived in 2a5691c (2026-09-24) beside the older SqOut entry from the port. ADR 0014 does not cover keeping the SqOut entry as a second encoding.

*Owner proposed: the stored sqout_tick (ADR 0014), read through one accessor. Candidates: spwin-5.*

#### 129. Headline shows the score from the record but stars from the stored cache

The question is how many stars the optimal path earns, as shown in the details headline.

There is one star rule, `path_stars` in `src/core/stars.cpp`. Its answer is saved in the `results.stars` column by `summarize_path` in `src/store/record_store.cpp` (around line 493). `fill_missing_stars` (line 1471) fills only empty rows and never rewrites a filled one. The headline in `render_headline` in `src/ui/details_panel.cpp` mixes two sources: the score comes from the live record (line 121) and the star count from that cached column (lines 135-139). The Stars tab (`stars_tab.cpp` line 22) works its cutoffs out live from the record. The library `stars:N` filter (`library_query.cpp` line 384) reads the cache.

They agree today. The verifier's Burnout check gives 7 stars from both: score 378,315 against a 7-star cutoff of 335,500. The star files have not changed since the column was added (git log shows only da26438 and bc01fa6).

They can only part if someone changes the star math in `src/core` without bumping `kResultsStamp`. Then the headline and the `stars:N` filter would lag behind the cutoffs shown live in the Stars tab.

This was a deliberate choice. The 2026-09-27 ui-redesign plan (lines 189 and 331) says the headline reads the stored number. ADR 0018 and `src/store/stored_versions.h` require the stamp bump for any scoring change. `path_stars` should stay the owner. Either the headline calls it on the record it already holds, or the column stays a cache that the stamp rule guards.

*Owner proposed: core/stars path_stars stays the owner; the results.stars column stays a stamp-guarded cache. Candidates: score-13, screenA-12.*

#### 130. SP cap and ms limit are stored in both the key columns and the blob

The question: which SP cap and which ms limit (the timing filter) did a stored result run under?

The answer is stored twice. `RecordStore::write_row` in `src/store/record_store.cpp` writes them as key columns (lines 969-970). `flatten_record` in `src/store/path_codec.cpp` also writes `ms_limit` and `sp_cap` into the structure blob (314-315). Readers split. `render_preview_panel` in `src/ui/preview_tab.cpp` (around 194) and `emit_dump` in `tools/replay.cpp` (481) read the blob's cap. `for_each_blob` (1314) reads the column. `build_record_status` in `src/app/path_view.cpp` (135-141) shows both from the blob, but only tests call it.

They agree today, shown by reading. The cap cannot differ, because `prepare_row` copies the column from the record (535) and checks the key against it (512). The ms check runs one way only. It is skipped when the key has the filter off, so a record with a 10 ms limit could in principle be filed under "off". Both real callers build the key and the search from one `Settings`, so no reachable input does this.

If they ever split, the Preview meter or a status line could show a cap or limit different from the settings that filed the row.

The key columns should own it, because lookups find rows by them. Displays should read the key, or `prepare_row` should check the ms limit both ways. The one-way ms check came in 133b52e (2026-08-28), per the finder.

*Owner proposed: The results key columns written by RecordStore::write_row. Candidates: store-12.*

#### 131. Library chart count cached in AppState and read from the model

The question is how many charts the library holds.

The answer is stored twice. `AppState::reload_library` in `src/ui/app_state.cpp` (around line 73) calls `set_charts` and then caches the count in `library_total`. The model itself also knows it, through `LibraryModel::rows().size()`.

Four places read the cache. They are the empty-library message in `render_library_pane` (`src/ui/library_view.cpp`, around line 25), the early return in `render_library` (`src/ui/library_table.cpp`, around line 471), and the Analyze button's count and width sample in `render_actions_row` (`src/ui/library_toolbar.cpp`, around lines 75 and 80). One place reads the model: `render_heading` in `library_table.cpp` (around line 73).

They always agree today. Only one code path sets the charts, and it updates the cache straight after. The verifier found this by reading.

If a new `set_charts` caller skipped the cache, the heading would show one count while the empty-library message or the Analyze button showed another.

`library_total` predates the C++ port (3b69958). The direct read in `render_heading` came with the 2026-09-27 library rework.

`LibraryModel::rows().size()` should be the one source, or a single AppState accessor that wraps it.

*Owner proposed: LibraryModel::rows().size(), or one AppState accessor wrapping it. Candidates: sweep-ui2-c8.*

#### 132. Best path score, notation and counts read from stored columns and from the blob

The question is: what are the best path's score, notation, activation count and note count? Each fact is stored twice. One copy is a results column. The other is the structure blob (the saved record that every path is rebuilt from).

`prepare_row` in `src/store/record_store.cpp` (line 537) writes the `bestpath` text column and the summary columns from `summarize_record`. Two screens read the columns. `best_path_label` in `src/ui/library_model.cpp` (line 71) builds the library Best path cell from them. `collect_fill_rows` in `src/app/fill_report.cpp` (lines 183-191) reads old/new path, Acts and Notes from them. Three screens rebuild the facts from the blob instead: `render_headline` in `src/ui/details_panel.cpp` (line 121), `build_path_buttons` in `src/app/path_view.cpp` (line 454), and `collect_rows` in `src/app/report.cpp` (lines 274-298).

They agree today, shown by reading. Every row one build writes matches itself. They would split if the notation rule or `summarize_path` changed without a `kResultsStamp` bump. The E label is a live example of that risk: it compares the stored offset with `kEarlyFillWindowMs` at draw time. Commit 396082a changed that window but invalidated records, so nothing split. `RecordStore::reindex` (line 1437), run by `hydra_batch --reindex`, would not save the user either. It rewrites the score and count columns but never `bestpath`.

If they drifted, the library cell and the fill page would show a different notation or count from the song panel and Paths buttons for the same chart. The blob should own these facts. The columns are only for sorting, so reindex should rewrite all of them.

*Owner proposed: The structure blob via Path::pathstring, Path::totalscore and store::summarize_path; the results columns treated as a cache that reindex always rewrites, bestpath included. Candidates: P1-2, P2-2.*

#### 133. Hardest timing is read from a stored column by the library but recomputed elsewhere

The question is: what is the best path's hardest squeeze or early fill, in ms?

The library reads a stored copy. `prepare_row` in `src/store/record_store.cpp` (around line 538) writes `summarize_path`'s `hardest_ms` into the results row once, at analysis time. `RecordStore::get_summary` reads it back. `facts_of` in `src/ui/library_model.cpp` (line 37) passes it to `query_matches` in `src/app/library_query.cpp`, where `squeeze<=N` tests it.

Two screens recompute it from the record instead. `collect_rows` in `src/app/report.cpp` (around line 274) calls `summarize_path` again for the report's ms column and tier. `build_path_buttons` in `src/app/path_view.cpp` (around line 456) calls `Path::difficulty()` for the button timing.

They agree today. All three use the same path and the same rule. `summarize_path` changed since the 1.8.2 stamp only in bc01fa6, which added stars, not `hardest_ms`. No run was made.

They would drift if `summarize_path`'s own max loop in `src/store` changed without a `kResultsStamp` bump. Only `RecordStore::reindex`, run by hand via `hydra_batch --reindex`, rewrites the column. Then `squeeze<=N` could keep or drop a song whose report ms and Paths tab button sit on the other side of N.

The user should pick one owner: the stored column, read by the report too, or `summarize_path` with a rule that any change forces a stamp bump. This is the same pattern as known finding P2-2 at the same call site.

*Owner proposed: Undecided: the stored hardest_ms column, or summarize_path with a stamp-bump rule (same decision as known P2-2). Candidates: R6-A2-1.*

#### 134. Record key pastes the raw difficulty word; two writers clean it by hand first

The question is which difficulty word goes into the record key, the chart-mode string that picks a stored row. `Settings::chartmode_key` in `src/app/config.cpp` (around line 137) pastes `view_difficulty` in exactly as stored. In the same class, `difficulty()` (line 129) and `effective_bass2x()` read that string ignoring case and fall back to Expert. So the key is only right if every writer cleans the word first. Two writers do it by hand: `Settings::load_file` (line 99) and `settings_from` in `tools/replay.cpp` (around line 144), which retypes the parse-and-Expert rule. The settings bar in `src/ui/settings_bar.cpp` is not a copy; it picks a name `difficulty_name` already made.

| view_difficulty | difficulty() | key prefix |
|---|---|---|
| "Hard" | Hard | Hard |
| "hard", set in memory | Hard | hard (no row matches) |
| "Legendary" via load_file | Expert | Expert |
| "Legendary" via replay | Expert | Expert, only because settings_from repeats the rule |

They agree today, because every shipped writer cleans the word. Reading shows the split: `view_difficulty = "easy"`, as `tests/test_config.cpp` line 210 sets it, gives Easy but a key starting "easy". No test pins the key for an uncleaned word. The raw paste dates from 3b69958, the clean-up from 133b52e, the replay copy from 06a178c.

If a writer skipped the clean-up, the library, reports and `hydra_replay dump` would find no row. Every chart would read Not analyzed.

`chartmode_key` should build from `difficulty_name(difficulty())`. Then the key cannot disagree with `difficulty()`.

Everything else reads the parsed value: the analysis, the dynamics key and the Expert-only 2x Bass rule all go through `Settings::difficulty()`. Even inside `chartmode_key`, the bass word comes from the parsed value while the difficulty word is raw. Commit 06a178c records your choice that a lowercase INI word selects that difficulty; it does not cover how the key is built.

*Owner proposed: Settings::chartmode_key in src/app/config.cpp, building from difficulty_name(difficulty()). Candidates: R3-A6-4, R3-A8-1.*

### Visibility 4: not visible (internal or test-only)

#### 135. Preview scene stores tick resolution twice; production reads only one copy

The question is how many ticks per quarter note the Preview scene's song has. `PreviewScene` in `src/app/preview_view.h` holds it twice: inside `timing` (line 219, read as `timing->tick_resolution()`) and as a plain `tick_resolution` field (line 220). `build_preview_scene` in `src/app/preview_view.cpp` (lines 272-273) fills both from the same `SongTiming`. Every production reader uses `timing`: `build_time_box`, the SP-meter box, the step helper and `render/track_state.cpp`. The plain field's only reader is one test check, `tests/test_preview_view.cpp` line 382. Test fixtures in `test_highway_draw.cpp`, `test_track_state.cpp` and `test_preview_renderer.cpp` set both copies by hand.

They agree for every scene the app builds. A hand-built scene can set `timing` to 480 and the field to 1000, and nothing notices, because no `src/` or `tools/` code reads the field. This was shown by reading.

Nothing shows on screen today or would if they drifted, since production never reads the field.

History explains it. ba0885a added the field before `timing` existed. cbb72aa ("Preview time box reads the engine's timing") added `timing` and left the old field behind. No ADR or CONTEXT.md entry asks for both.

`SongTiming` should own it through `scene.timing`. Remove the field, its write and the test assignments, and point the one test check at `scene.timing->tick_resolution()`.

*Owner proposed: SongTiming::tick_resolution() through PreviewScene::timing. Candidates: R5-A8-3.*

## Duplicates that agree today (166)

The same question is answered in two or more places by separate code. The copies give the same answers today, so nothing is wrong on screen yet. Each one is a place where a future change can split them.

### Visibility 1: wrong or contradictory on screen today

#### 136. Preview shows activations in two different golds

The question is which gold colour marks Star Power and activations.

`kBestPathColor` in `src/ui/theme.h` (line 19) is (250,210,0). The Preview uses it for the activation marks on the scrubber, in `src/ui/preview_tab.cpp` (around line 169). The same file also types a bare `IM_COL32(255,204,51)` four times. That literal is used for the next-activation box header (line 503), the "SP" label (526), the gauge fill (548) and the drain box accent (571). No named constant exists for it.

This was shown by reading. The meter, label and drain box arguably mean "Star Power", which is a different idea from best-path gold. But the scrubber marks and the next-activation header both mean "an activation". On the Preview tab today they show in two slightly different golds.

theme.h should own a named SP gold. The user should decide whether activation text uses it or `kBestPathColor`. The literal dates from aab5ff1 on 2026-08-31.

*Owner proposed: src/ui/theme.h (a named SP gold next to kBestPathColor). Candidates: display-37.*

### Visibility 2: visible in an edge case, in docs or in CLI text

#### 137. The plain-row scale branch writes its own sign test instead of the owner's

The question is which side of the SP end a backend row sits on. That decides which frontend direction could lose it.

The owner is `src/core/backend_value.h`. `paid_by_sp_walk` (around line 30) says a row is inside SP when its offset is `<= 0.0`. `counted_without_squeeze` (around line 34) adds rows below the leeway. `rate_activation` in `src/core/squeeze_rating.cpp` (around lines 108-132) uses raw sign tests instead.

| Offset | owner | squeezed-out arm | plain arm |
|---|---|---|---|
| below 0 | inside | early scale | early scale |
| exactly 0 | inside | early scale | no scale |
| 0 up to leeway | counted | late scale | no scale |
| leeway or more | not counted | late scale | late scale |

The squeezed-out arm matches the owner everywhere. The plain arm's `< 0.0` differs only at exactly 0 ms. That happens when SP ends on a chord. No scale can be material at 0, so the screen agrees today. This was shown by reading.

The real behavior is that counted rows between 0 and the leeway get no scale and no eff. figure. The default leeway is 3 ms (`rules.h`), but the rules file can change it. With leeway 50, a row at +40 ms and early scale 0.5 would be material yet shows nothing. The verifier says no test or run settles whether that is wrong.

`squeeze_rating` should branch on the owner's predicates. The plain arm came from 4f5fcc1 (2026-09-28). Its commit message covers rows inside SP only, not rows between 0 and the leeway.

*Owner proposed: core/squeeze_rating: branch on paid_by_sp_walk / counted_without_squeeze. Candidates: backend-11.*

#### 138. Backend and Path limit boxes type 500 instead of the engine window

The question is: what range can the Backend limit and Path limit boxes take?

The intended answer is the engine's squeeze window. The squeeze window is how far, in ms, the engine searches for a squeeze. It is `kSqueezeWindowMs` = 500 in `src/core/model.h` (line 75). Commit 9a77258 (2026-08-28) says so directly. It raised both boxes from 200 to 500 because "the engine's squeeze window is 500 ms".

The code does not read that constant. It types 500 by hand in three places. In `src/ui/paths_tab.cpp` (line 503), `render_path_footer` clamps the Backend limit to 0..500. In `src/ui/settings_bar.cpp` (line 144), the Path limit box clamps to -500..500. In `tests/ui/uitest_paths.cpp` (lines 184-188), `test_paths_backend_timings` types 500 and then 600 into the Backend limit and expects 500 both times. That test is the third copy.

| Typed into Backend limit | Box result |
|---|---|
| -50 | 0 |
| 500 | 500 |
| 600 | 500 |

All three copies agree today, and the GUI test passes. They split only if the window changes. If `kSqueezeWindowMs` became 750, both boxes would still stop at 500, and the user could not filter out to the full window. The test would keep passing, because it pins the old number too.

There is a second gap, found by reading the code. The INI loader does not apply the range at all. `Settings::load_file` in `src/app/config.cpp` reads `backendlimit_value` (line 79) and `mslimit_value` (line 76) with a bare `atoi`. An INI value of -50 for the Backend limit loads as -50. Then `backend_limit()` (line 161) takes its absolute value and uses 50. Typing -50 into the box gives 0 instead, which hides every backend row with a nonzero offset. So the same number shows different rows depending on where it came from. The Path limit has the same hole: an INI value of 900 loads as 900, though the box would never allow it.

On screen, the 500 is the most either box will accept. Because the backend clamp feeds `backend_limit()`, it also decides which backend rows the Paths tab shows.

`kSqueezeWindowMs` in `core/model.h` should own the range. The engine defines the window, so the boxes and the loader should ask it rather than repeat it. Both clamps should read the constant. The INI load path should apply the same clamp. The GUI test should type `kSqueezeWindowMs + 100` and expect `kSqueezeWindowMs`.

Neither value has a recorded user decision. `kSqueezeWindowMs` dates to 3b69958 (2026-08-15). Commit 7dafadd (2026-09-24) only moved its line. ADR 0011 mentions 500 ms only in passing. One of the two source findings also checked the neighbouring default, `kDefaultHitWindowMs` = 85 (line 69), which `config.h` uses as its hit-window setting default. That 85 was set in 11305b3 (2026-08-19), whose message says "the 70 figure was outdated". It became a named constant in ba0885a (2026-08-22). CONTEXT.md defines the hit window but gives no default. No user decision was found for it either. Unlike the 500, it is read from the constant everywhere, so it has no copy problem.

*Owner proposed: kSqueezeWindowMs (src/core/model.h). Candidates: squeeze-13, sweep-tests4-c10, tempo-33.*

#### 139. The smallest SP cap of 1 is checked in five places with no shared constant; replay's message also types Clone Hero's 4 by hand

The question is: what is the smallest SP cap Hydra allows? A side question is how text names Clone Hero's own cap.

Five places answer the first question, and none of them shares a constant. The INI loader, `Settings::load_file` in `src/app/config.cpp` (line 91), keeps a value only if it is at least 1. The settings box, `render_sp_cap` in `src/ui/settings_bar.cpp` (line 86), floors the typed value to 1. The Preview's meter builder, `build_sp_meter_curve` in `src/app/preview_view.cpp` (line 75), floors to 1. The Preview panel, `render_preview_panel` in `src/ui/preview_tab.cpp` (line 538), floors to 1 a second time as a divide-by-zero guard. hydra_replay's settings parse, `settings_from` in `tools/replay.cpp` (lines 147-150), throws below 1.

One finding counted four places and missed the second Preview floor. The code has it, so five is right.

Clone Hero's cap lives once, as `kCloneHeroSpCap = 4` in `src/core/model.h`. The GUI help text is built from that constant. hydra_replay's error message is not: it types "(4 is Clone Hero's rule)" as a literal.

| Cap given | INI loader | Settings box | Preview (both floors) | hydra_replay --cap |
|---|---|---|---|---|
| 0 or junk | keeps the default 4 | becomes 1 | uses 1 | throws an error |
| 1 | accepted | accepted | accepted | accepted |

All five use the same threshold of 1 today, shown by reading the code. They differ only in what a too-small value turns into. That is a policy difference, not drift. The literal 4 in replay's message matches the constant today too. If `kCloneHeroSpCap` ever changed, the `--cap` error text would go stale while the GUI help text stayed right.

What the user sees: a cap of 1 is accepted everywhere, but it can never produce an activation. The engine clamps the meter to the cap (`src/search/engine.cpp` line 513), and activation needs 2 (line 516). So the user can analyze at cap 1 and get only paths with no activations, with nothing on screen saying why.

A `kMinSpCap` next to `kCloneHeroSpCap` in `src/core/model.h` should own the rule. That header already owns the cap's default, and all five readers already include it. Replay's message should print the constant instead of a typed 4. Replay's own default cap of "4" is a separate item, covered in the hydra_replay defaults finding.

The copies came in over five weeks. One finding said they all arrived in eaf3b73; history shows otherwise. The INI check first appeared in ba0885a (2026-08-22, "SP cap as a runtime setting"), and eaf3b73 only rewrote that line. Both Preview floors came from aab5ff1 (2026-08-31). The panel guard moved to `preview_tab.cpp` in the 2facc04 file split. The settings-box floor is from 99cbda2 (2026-09-27). The replay check and its literal 4 are from eaf3b73 (2026-09-27, "Remove Auto").

The minimum of 1 is an assumption, not a user decision. The two findings disagreed here, and the plan text settles it. In the 2026-09-27 UI-redesign plan, decision #6 quotes the user: drop Auto and "leave the default as 4." It says nothing about a minimum. The "at least 1" wording appears only in the agent-written plan body: the T6 goal line and the "T6 provides" note. CONTEXT.md's "SP cap" entry and ADR 0003 say nothing about a minimum. Whether cap 1 should be allowed at all, given it can never activate, is a question for the user.

*Owner proposed: A kMinSpCap constant (or one validate helper) beside kCloneHeroSpCap in src/core/model.h, read by the INI loader, the settings box, the Preview and hydra_replay. Candidates: spwin-13, sweep-core1-c7.*

#### 140. Hit window default is turned into an int in Settings, ReportOptions and ReportJob, and the INI loader drops fractions

The question is what the hit window is, in milliseconds, and what type holds it. The hit window is how early or late a hit can land and still count.

The value lives in one place: `kDefaultHitWindowMs = 85.0` in `src/core/model.h` (around line 69). It is a decimal number. What repeats is the step that turns it into a whole number.

Three places write `static_cast<int>(kDefaultHitWindowMs)`. They are `Settings` in `src/app/config.h` (line 81), `ReportOptions` in `src/app/report.h` (line 114), and the `ReportJob` constructor in `src/ui/library_jobs.h` (line 234). `Settings::load` in `src/app/config.cpp` (lines 84-87) parses the INI value with `atoi`, which drops any fraction. Readers in `paths_tab.cpp` and `report.cpp` then turn it back into a decimal. Meanwhile the function defaults in `report.h` (lines 65, 86, 94) and `squeeze_rating.h` (lines 80, 89, 142, 153) take a `double` straight from the constant.

The probe tools hold the game's 85 ms separately, twice: `EXPECT_NORMAL_BACK_S = 0.085` and `EXPECT_NORMAL_BACK_MS = 85.0` in `tools/ch_probe/constants.py`.

| Source of the value | 85 | 85.5 |
|---|---|---|
| Settings, ReportOptions, ReportJob defaults | 85 | 85 |
| `hydra_settings.ini` read by `atoi` | 85 | 85 |
| double defaults in report.h and squeeze_rating.h | 85 | 85.5 |

All copies agree at 85 today. This was shown by reading.

Production always passes `settings.hit_window_ms`. The `ReportJob` default is never used, and only tests in `tests/test_report.cpp` rely on the `ReportOptions` default. So nothing on screen depends on those two copies now.

The INI path is the live one. If someone writes 85.5 in `hydra_settings.ini`, squeeze budgets, ratings and report tiers silently use 85. There is no UI control for this setting.

The value 85 is decided: d49c624 (2026-08-19), which added the setting, says "default 85". The whole-number type is not decided anywhere. CONTEXT.md "Hit window" defines it as a setting that feeds squeeze budgets, ratings and report tiers, which makes `Settings` the user-facing owner, but it gives no type. The three int casts came together in ba0885a (2026-08-22), with the constant. The probe constants came in with 283e028 (2026-09-26).

`Settings::hit_window_ms` should own the default, as a decimal that starts from the constant and is parsed without dropping the fraction. `ReportJob` can drop its default argument, because its only caller passes the setting. The probe's two spellings should become one constant.

*Owner proposed: Settings::hit_window_ms in src/app/config.h, as a double seeded from kDefaultHitWindowMs; one expected-ms constant in tools/ch_probe/constants.py. Candidates: sweep-ui1-c14, tempo-29.*

#### 141. .chart solo and disco events match raw text; section names strip quotes

The question is how the text of a .chart `E` event is read. An `E` event carries a word such as `solo` or a section name.

Two paths answer it inside `ChartDataEntry::ChartDataEntry` in `src/parse/song.cpp`. Solo, solo end and disco events (around lines 711-718) compare the raw second word, and only when there are exactly two words. The generic path (around lines 719-725) trims the text and strips one pair of surrounding quotes, for practice section names.

| Event line | Read as |
|---|---|
| `E solo` | solo starts |
| `E "solo"` | plain event named solo, no solo |
| `E [mix_3_drums0d]` | disco flip |
| `E "[mix_3_drums0d]"` | no disco flip |

They agree on every chart in testdata, shown by reading and a corpus grep. All 29 `E solo`, 29 `E soloend` and the disco events are unquoted. Only section, lyric and phrase events use quotes. How Clone Hero reads a quoted `E "solo"` is not verified.

A chart that quoted its gameplay events would lose its solos or disco flips in Hydra.

The raw matching came from 3b69958 (2026-08-15). The quote stripping came from aab5ff1 (2026-08-31).

`ChartDataEntry` should own one normaliser that every event match uses.

*Owner proposed: ChartDataEntry (src/parse/song.cpp): one payload normaliser feeding every E-event match. Candidates: parse-25.*

#### 142. Batch analyzed and skipped counts are rebuilt by each caller, not reported

Two questions share one root: how many charts did a batch store, and how many did it skip because they already had a result. `run_batch` in `src/app/analysis.cpp` knows both, but `BatchProgress` reports only `completed`, `failed` and the to-do `total`. So every caller rebuilds the numbers.

For "analyzed", the GUI subtracts twice in `src/ui/library_dialogs.cpp`. `batch_counts` (around line 55) prints `completed - failed`. `render_batch_strip` (around line 422) repeats it for the Stop tooltip. The CLI in `src/cli/batch.cpp` counts `on_result` calls instead (around line 246). For "skipped", `BatchJob::run` in `src/ui/library_jobs.cpp` (around line 266) and the CLI (around line 233) both compute items given minus the first progress total.

They agree on every reachable path, shown by reading. The "-1 analyzed" case the finder raised needs a load-failure branch that only an unused constructor reaches, so that drift was refuted. One small split remains. After a chart fails, the GUI bumps `failed` before `completed`, so a snapshot taken between the two shows "analyzed" one too low for about a frame. The CLI never shows this.

The numbers appear as "N analyzed", "N skipped (already had a result)" and "Keeps the N results already finished" in the GUI, and as "Analyzed N, skipped ..." in the CLI. No user decision was found in CONTEXT.md or `docs/adr/`.

`run_batch` should own both counts and put them in `BatchProgress`.

*Owner proposed: run_batch in src/app/analysis.cpp (report analyzed and skipped in BatchProgress). Candidates: sweep-ui1-c1, sweep-ui1-c2.*

#### 143. Which library row is the selected one is decided in three places

The question is which library row is the one the song panel shows. The answer is "same notespath", because the same chart can sit in two folders.

Three places decide it with their own compare. `AppState::relative_row` in `src/ui/app_state.cpp` (around line 139) uses it for previous/next. `src/ui/library_table.cpp` uses it for the row highlight (around line 374) and the scroll-to-selected loop (around line 351). They agree today.

Two nearby checks answer different questions. `AppState::update_song_length` (around line 287) compares md5. It decides which md5-keyed record lookups get a new song length, and md5 is the store key, so it is not a copy. `AppState::analyze_job_shown` (around line 262) compares notespath. That is a separate identity choice, and it has a visible edge, shown by reading. Put the same chart in folders A and B, analyze A, then open B. B's panel shows no progress box and says "Wait for <title> to finish analyzing." When A finishes, B shows the new stored result, and a failure goes to the status line.

No user decision covers either choice. The "two folders" line is a code comment from an agent-written plan (f1c6a3d, a3799cc, 2026-09-27). CONTEXT.md and `docs/adr/` say nothing on row identity.

One `AppState` helper should own "is this row the selected one", used by `relative_row` and both table sites. `analyze_job_shown` should name its identity choice next to it.

*Owner proposed: one AppState helper for row identity by notespath. Candidates: sweep-ui1-c4.*

### Visibility 3: visible only if the copies drift later

#### 144. Whether a SqIn/SqOut is still to earn or already free is decided twice, with flipped signs

The question is: does the player still have to earn this SqIn or SqOut, or is it already free?

Two places answer it. They use opposite signs on different numbers.

The first is `rate_activation` in `src/core/squeeze_rating.cpp`, around line 156. It checks `sq.difficulty() >= 0`. If true, the squeeze is still to earn, and it judges the hit that achieves it. If false, the squeeze is free, and it judges the opposite direction instead.

The second is `squeeze_sentences` in `src/app/path_view.cpp`, around lines 66-99. It tests `sq.timing()` instead. For a SqOut (lines 72-74), `t >= 0` gives "more than N late", the earn wording; otherwise it says "no more than N early". For a SqIn (lines 97-99), `t <= 0` gives "more than N early"; otherwise it says "no more than N late".

The two numbers are related through `SPSqueeze` in `src/core/model.h` (lines 190-191). `timing()` is the offset negated. `difficulty()` is the offset negated for a SqOut, and the plain offset for a SqIn. So for a SqOut timing equals difficulty, and for a SqIn timing is minus difficulty.

| Squeeze | rate_activation says "earn" when | sentence says "earn" when | same rule? |
|---|---|---|---|
| SqOut | difficulty >= 0 | timing >= 0 | yes, timing = difficulty |
| SqIn | difficulty >= 0 | timing <= 0 | yes, timing = -difficulty (same as offset >= 0) |

They agree today on every input. That includes exactly 0, -0.0 versus +0.0, and NaN. I re-checked this against the code. At a SqIn offset of -0.0, timing becomes +0.0 (so `<= 0` holds) and difficulty is -0.0 (so `>= 0` holds). Both pick "earn". With NaN every comparison is false, so both pick "free". Both verifiers confirmed agreement by reading, not by running.

Two other suspects are not copies. `SPSqueeze::description` in `src/core/model.cpp` never branches on the sign, so it is not a third answer. A finder also counted `BackendSqueeze::summarystr`, but its "Free SqOut" means slack of a whole hit window. That belongs to the separate ladder in the footer finding, so that part was refuted.

Nothing is wrong on screen today. The risk is if one copy moves its edge at zero, or flips a sign, and the other does not. Then the squeeze sentence would call a squeeze "already free" while the rating judged it as one still to earn. The rating's direction feeds two visible things. One is whether the "Frontend timing scales ..." warning line turns orange (`path_view.cpp` around lines 256-259, via `early_note_warns` and `late_note_warns`). The other is the "(eff. N ms)" figure printed on a SqIn's sentence. The two findings called this "the scale line" and "the transfer warning". They are the same line, so there is no factual disagreement.

`SPSqueeze` in `core/model.h` should own one predicate, for example `is_free()` or "still to be earned" (`difficulty() >= 0`). Both callers would use it. It belongs there because `SPSqueeze` already owns `timing()` and `difficulty()`, the two numbers each copy derives the answer from. The rating's direction rule came in 257745a (2026-08-29). The sentence tests came in 577ddc2 (2026-09-27). No user decision on it was found in CONTEXT.md or the ADRs. (Merged from V01-sqout-rating#7, source screenA-25, and V06-difficulty#10, source squeeze-3.)

*Owner proposed: core/model.h SPSqueeze: one 'still to be earned' predicate (difficulty() >= 0), called by both rate_activation and squeeze_sentences. Candidates: screenA-25, squeeze-3.*

#### 145. hydra_replay re-derives which phrase chord the engine squeezes out

The question is which phrase chord the engine squeezes out at a given SP end. Call that end D.

The graph answers it in two halves in `src/search/graph.cpp`. `ScoreGraph::add_deact_edge` (around line 360) takes the first SP chord at or before D that is still within 500 ms. If none is found, `ScoreGraph::store_new_backend` (around line 241) takes the first SP chord after D within 500 ms. `hydra_replay` answers it again for itself. `sqout_candidates` in `src/core/replay.cpp` (lines 29-37) lists every SP chord less than 500 ms from D, in chart order. `resolve_sqout_note` (line 236) and `ambiguous_window_warnings` (line 267) both take the first one. A comment at replay.cpp lines 25-28 says it mirrors the graph.

| Chord position | Graph | Replay |
|---|---|---|
| Before D, under 500 ms | candidate, checked first | candidate |
| Exactly 500 ms away | excluded | excluded |
| After D, under 500 ms | candidate only if none before | candidate, later in chart order |

They agree today, shown by reading. Every chord before D comes before any after D in chart order, so both pick the same first chord. A chord the graph already pruned is always 500 ms or more away, so the replay drops it too. The replay tests pass, but they never compare against the graph.

If the graph's rule drifted, `hydra_replay` would accept or refuse a typed SqOut, and name a warning chord, differently from the engine.

The graph should own it, since it makes the real choice. The replay copy came in 798026c (2026-09-24). Plan decision 20 asks for the refusal, not a second copy of the rule. `src/core/replay.h` (line 66) mis-cites it as "user decision 23".

*Owner proposed: search/graph: one exposed predicate or result next to kSqueezeWindowMs, called by the replay. Candidates: backend-12, screenB-20, spwin-2, squeeze-21.*

#### 146. The "before, on or past the squeezed-out chord" tick test is retyped outside sqout_position

The question is where a backend row sits against the activation's squeezed-out chord: before it, on it, or past it. A backend row is a note just after Star Power ends that might still count under SP. The squeezed-out chord is the note hit right after SP has ended, so nothing past it can be a backend.

One function already answers this. `core::sqout_position` in `src/core/backend_value.h` (lines 20-26) compares the row's tick with the stored squeeze-out tick. It returns NoSqOut when there was no squeeze-out, Before when the row is earlier, Exact when the ticks are equal, and After when the row is later. The engine's `create_deactivated_path` calls it (`src/search/engine.cpp` line 630), and so does the replay's `replay_path`.

Six other places write the comparison out themselves. In production, the trim in `rebuild` (`src/search/engine.cpp` lines 1233-1239) removes rows whose tick is greater than the squeeze-out tick, using its own lambda. That is the same file where `create_deactivated_path` does call `sqout_position`. `Activation::display_backends` in `src/core/model.cpp` has an `is_beyond_sqout` lambda (lines 584-586) that also drops rows with tick greater than the squeeze-out tick. `Activation::is_sqout_backend` in the same file (line 576) tests equality on its own. `Engine::trim_cols` (`src/search/engine.cpp` line 365, called at line 614 with the squeeze-out tick) drops collected SP phrases at or after that tick. `build_activations` in `src/app/path_view.cpp` (lines 316-320) does not compare ticks at all: it passes Exact or NoSqOut depending on the row's squeezed-out flag. In the tests, the case "no activation keeps backends past its squeezed-out note" in `tests/test_search.cpp` (lines 300-306) retypes the `>` and `==` checks.

| Row tick vs stored squeeze-out tick S | `sqout_position` | `rebuild` trim and `is_beyond_sqout` | `is_sqout_backend` / test | `trim_cols` (phrases) | `path_view` |
|---|---|---|---|---|---|
| no S stored | NoSqOut | not applied | false / activation skipped | not applied | NoSqOut |
| below S | Before | kept | not the sqout row / kept | kept | NoSqOut |
| equal to S | Exact | kept | the sqout row / counted once | dropped | Exact |
| above S | After | dropped | not reached / "past" failure | dropped | never sees it |

They agree today. All three findings checked this by reading, and I re-read the code and confirm it. Every row copy uses strict "greater than" for past and "equal" for on. Before and NoSqOut look different in the table, but `backend_row_value` (`backend_value.h` lines 40-45) prices them the same: only After and Exact change the value. `trim_cols` uses "at or after", but it trims collected phrases, not backend rows, and a phrase on the squeezed-out chord was never collected, so it is consistent. The corpus test reported passing on 100 squeeze-out activations; that count is printed, not enforced as a floor. The `display_backends` guard can no longer fire, because `rebuild` already trimmed those rows. Its own comment says it is there only "for any list that still holds one". So there is no input where the copies disagree today.

If one copy drifted, say the `rebuild` trim moved to "greater or equal", the engine would drop the squeezed-out chord's row while `display_backends` and the details table still expect exactly one. On screen, the Backends table's Points column and its "(-N)" mark could then disagree with the stored score. A row could also be kept by one path and dropped by another. The test copy changes nothing on screen, but it would keep passing against its own retyped rule rather than the real one.

Three nearby comments have gone stale. The `rebuild` comment (`engine.cpp` lines 1224-1226) says it mirrors an "is_after_sqout arm" of `create_deactivated_path`, which no longer exists; that function now calls `sqout_position`. `src/core/model.h` (lines 211-213) still calls the 3 ms edge absolute, though it is now the `leeway_ms` parameter. The `rate_activation` comment in `src/core/squeeze_rating.cpp` (lines 89-92) says nothing about offset 0, which the code sends to the early scale.

A second backend rule has the same shape. A backend's offset is the row's ms minus the SP-end ms. No function owns it. It is written inline in `store_new_backend` (`graph.cpp` lines 235-236), in `add_deact_edge` (`graph.cpp` lines 355-356) and in `rebuild` (`engine.cpp` line 1204). The test "SP past the last note: backends measured from the tracked SP end" (`test_search.cpp` lines 460-461) restates it a fourth time. All copies agree today, by reading. The production copies decide the real Backends rows and their offsets, so drift would show a different offset on screen.

`sqout_position` should own the tick test, because it already exists for exactly this question and the engine and replay use it. The `rebuild` trim, `is_beyond_sqout`, `is_sqout_backend` and the test should call it, and the stale comments should point there. The offset rule needs one new function in `core` to replace its three inline copies, and the test should call that too.

The copies came in over four commits. 8668c04 (2026-08-29, "No backend rows past the squeezed-out note") added the `rebuild` trim and the display guard. 2a5691c, 706782b and 798026c (all 2026-09-24) stored the squeezed-out phrase, routed pricing through one function, and switched to finding the chord by its stored tick. 706782b created the one pricing function, yet the tick test stayed retyped around it. No user decision for keeping separate copies was found in docs/adr or CONTEXT.md.

*Owner proposed: core::sqout_position in src/core/backend_value.h for the tick test; a new single function for the backend offset rule (row ms minus SP-end ms). Candidates: backend-14, backend-17, backend-2, screenA-8, spwin-4, squeeze-23, squeeze-24.*

#### 147. Preview SP gauge counts banked bars itself, including squeezed-out phrases by set difference

The question is how many SP bars are banked at the playhead. That covers two moments: the stretch between activations, and the instant an SP window ends.

The engine answers it in `src/search/engine.cpp`. `Engine::advance` (around line 511) adds one bar per phrase, capped at the SP cap. `Engine::create_deactivated_path` (line 602) sets the bank to 1 after a squeeze-out and to 0 otherwise. A squeeze-out (SqOut) is a phrase collected just as SP ends. The record keeps the bank only at each activation, as `sp_meter`. Line 1232 also stores `sqout_tick`, the tick of that squeezed-out phrase (ADR 0014).

The Preview answers it again in `build_sp_meter_curve` in `src/app/preview_view.cpp`. Between activations it adds one bar per phrase, capped (lines 88-97). At each activation it snaps back to the stored `sp_meter` (line 105). Inside a window it gathers the phrases that end before the SP end (lines 118-123). It then counts the ones missing from `collected_phrase_ticks` (lines 136-141). That count becomes the bank when SP ends (line 183). Nothing under `src/app` reads `sqout_tick`.

| Case | Engine bank after the window | Preview bank after the window |
|---|---|---|
| One squeezed-out phrase inside the window | 1 | 1 |
| Squeezed-out phrase exactly at the SP end | 1 | 0, then 1 a moment later from the flat walk; the drawn meter looks the same |
| Two uncollected phrases inside the window | 1 | 2 |

They agree today. Reading the code shows it. So does a corpus probe: 1484 of 1486 activations match. The other two agree at the same instant once the activation chord is hit. The last table row is where they would differ. The verifier could not build an engine path that produces it.

If that row ever happened, the gauge would jump higher at the SP end than the engine's own bank. It would then visibly drop at the next activation, when it snaps to `sp_meter`. The same jump would appear between activations if the engine's banking rule ever changed and the gauge's copy did not.

The engine should own the count, and the gauge should only read it. The engine already decided the answer. The record could carry the bank after each window. Or the gauge could read `sqout_tick` instead of taking a set difference over the chart. One finder named `ScoreGraph::max_sp_bars` as the owner. That was wrong: it answers a different question (the most bars a path could hold, not what is banked now).

The two source findings read CONTEXT.md differently, and both are right. The "SP meter gauge" entry (lines 206-214) does write the set-difference rule down: a window phrase the record does not list was squeezed out and banks when SP ends. The same entry also says the gauge is "never re-derived". The code re-derives the bank, so the entry contradicts itself. This is a written-down copy, not a hidden one. The between-activation stepping came in aab5ff1 (2026-08-31). The squeezed-out count came in 906881d (2026-09-24).

*Owner proposed: The engine (Engine::advance and Engine::create_deactivated_path); the record should carry the bank after each window, or the gauge should read the stored sqout_tick. Candidates: display-27, screenB-2, spwin-7, squeeze-26.*

#### 148. The graph writes its squeeze-note claim test twice

The question is which SP phrase chord claims an SP end as its squeeze note, and at what offset.

The graph writes the claim twice in `src/search/graph.cpp`. `ScoreGraph::add_deact_edge` (lines 360-365) handles chords at or before the SP end. `ScoreGraph::store_new_backend` (lines 241-247) handles chords after it. Both use the same test: the chord is an SP chord and the edge is not yet claimed. Both write the offset as the chord time minus the SP end time (lines 235-236 and 355-356). `engine.cpp` (around line 1204) writes that offset formula a third time for tail rows.

They agree today, shown by reading. For any one chord only one site runs. The after-end site only runs if the before-end site left the edge unclaimed. The different side effects look deliberate: only the before-end site extends `sqout_time`, and only the after-end site counts a late SqIn.

If one copy's test or offset changed, SqIns before the end and late SqIns would be found by different rules. That would change which squeezes the Paths tab shows.

One helper in `graph.cpp` should own the claim, because both sites live in the same class and do the same job. The code dates from the 3b69958 C++ port (2026-08-15). No user decision was found in docs/adr, CONTEXT.md or the 2026-09-24 plan.

*Owner proposed: src/search/graph.cpp: one claim_squeeze_note(edge, note, offset) helper. Candidates: squeeze-18.*

#### 149. A row's distance from the SP end is computed in five places

The question is how many milliseconds a note sits from the SP end (the node where Star Power runs out, called D). That number is the offset shown and priced for each backend row.

The same subtraction, note ms minus D ms, is written in several places. In `src/search/graph.cpp`, `ScoreGraph::add_deact_edge` (around line 355) does it for rows up to D, and `ScoreGraph::store_new_backend` (around line 235) does it for rows after D. `rebuild` in `src/search/engine.cpp` (lines 1201-1204) does it again for tail rows, where SP outlasts the chart. Its own comment calls this "the same offset rule add_deact_edge uses". `replay_path` in `src/core/replay.cpp` (lines 121-123) recomputes it instead of reading the stored `offset_ms`. It also adds a clamp to zero for rows at or before D, which the engine does not have. `resolve_sqout_note` in the same file (lines 232 and 247) does the subtraction twice more.

They agree today, as shown by reading. Every copy reads ms through the same `MsIndex::at`. Replay's D is the stored `Activation::deact_tick`, the same node the engine measured against. The clamp never fires while time rises with tick. Only a negative tempo in a .chart could make time run backwards, and no such chart was built or run.

Nothing differs on screen today. If one copy changed its anchor or clamp, the Preview score box and the Paths tab Points column would price the same row differently. The replay self-check would then mark the path unfaithful.

One helper in `core/backend_value.h` should own this, because graph, rebuild and replay all need it. Replay still has to compute offsets for windows typed into `hydra_replay`, which have no stored rows. No decision on where the rule lives was found in docs/adr or CONTEXT.md.

*Owner proposed: One backend_offset_ms(row_tc, end_tc) helper in core/backend_value.h, called by graph, rebuild and replay. Candidates: backend-4, backend-5, spwin-10, squeeze-19, tempo-8.*

#### 150. Squeeze-out cost is worked out twice on the Paths tab, by undoing core's subtraction

The question: how many points does squeezing out this chord cost?

Core owns the answer. `category_scores` in `src/core/scoring.cpp` (lines 75-76) builds `sqout_reduction`, and `CategoryScores::sqout_sp` in `src/core/scoring.h` (line 28) subtracts it. Only `points` and `sqout_points` are stored, so the cost itself is never kept.

`src/app/path_view.cpp` rebuilds it twice by running that subtraction backwards. `squeeze_sentences` (lines 81-88) computes `lost = row.points - backend_row_value(..., Exact)` and says "scores N fewer points" when `lost > 0`, else "costs no points". `build_activations` (lines 314-337) prints "squeezed out (-N)" with N = `bsq.points - value` in warning colour, with no `lost > 0` check.

| Counted row | Sentence | Table |
|---|---|---|
| cost above 0 | scores N fewer | (-N), orange |
| cost exactly 0 | costs no points | (-0), orange |
| not counted | never counted | (uncounted) |

They agree today, shown by reading. The middle row cannot happen: the cost is base score (at least 50) times a multiplier (at least 1). A one-note chord under FirstNote stores `sqout_points` = 0, so it costs everything, not nothing. The `build_activations*` tests pass (7/7).

The same function also passes `Exact` from `br.squeezed_out`, which comes from `Activation::is_sqout_backend` (tick == `sqout_tick`). That is a small second copy of the `Exact` test in `sqout_position` in `backend_value.h`. It agrees today.

If one copy's gate changed, the sentence and the table row could quote different costs for the same note. One cost helper in `src/core/backend_value.h` should feed both.

*Owner proposed: core/backend_value.h (one squeeze-out cost helper, plus sqout_position for the row's position). Candidates: backend-3, display-8, score-21, screenA-7, squeeze-25.*

#### 151. A path's hardest timing is computed three times; the stored one re-types Path::difficulty's loop

The question is: what is a path's hardest timing in ms? It is the largest squeeze gap among the path's activations. Three places compute it.

`Path::difficulty` in `src/core/model.cpp` (lines 558-566) is the owner. It loops over `walk_activations()` and keeps the largest `act.difficulty()`. It drives the path buttons on the Paths tab and `Path::is_difficult`.

`summarize_path` in `src/store/record_store.cpp` writes the same loop again instead of calling it. The `hardest` variable is declared at line 475 and the loop runs at lines 477-481. The result is stored as `hardest_ms` at line 488. That stored number drives the library's `squeeze<=` filter, the hardest-timing sort column, and the report's Hardest ms column and tier. One of the source notes names the owner as "summarize_path sets s.hardest_ms = path.difficulty()". That describes the fix, not today's code. Today it does not call `Path::difficulty`.

`Engine::search_difficulty` in `src/search/engine.cpp` (line 704) keeps a running max during search. `close_last_activation` just above it (line 697) folds each finished activation into `diff_prefix`. `search_difficulty` then adds the open last activation. It feeds the "Limit timings" filter in `passes_ms_filter`.

The engine copy is worded differently, so here is how the three treat the same cases:

| Case | Path::difficulty | summarize_path | Engine::search_difficulty |
|---|---|---|---|
| No activation has a gap | empty optional | empty optional | "no value" sentinel |
| Gaps 12 and 20 | 20 | 20 | 20 |
| Two equal gaps | first kept (strict >) | first kept (strict >) | first kept (strict >) |
| Negative gaps -12 and -5 | -5 | -5 | -5 |

They agree today for every input. All three take the max over the same activations with the same strict comparison. "Nothing" means the same thing in all three. The verifiers checked this by reading the code, and I re-read all three loops to confirm it. No test catches drift between them. The filter test in `tests/test_library_query.cpp` (lines 194-209) uses hand-made rows, so it never runs `summarize_path`.

Nothing is wrong on screen now. If one copy drifted, the library filter, sort and report would name a different hardest timing than the path buttons on the song panel for the same path.

`Path::difficulty` should own it, and `summarize_path` should just call it. The engine's copy is shaped by search, since it updates a max step by step on its own data. It should share the per-activation difficulty helper from the next finding rather than call `Path::difficulty` directly. ADR 0001 asks for one raw-gap difficulty across filter, path list and report, which supports one owner. CONTEXT.md defines "Difficulty" as the path's hardest raw gap but says nothing about a second computation. Nothing in docs/adr/ or the commit messages endorses one. All three copies arrived with the C++ port in 3b69958 (2026-08-15).

*Owner proposed: Path::difficulty in src/core/model.cpp (summarize_path should call it; the engine's running max should share the per-activation helper). Candidates: display-12, fills-9, screenA-10, screenB-11, squeeze-28, sweep-tests2-c7.*

#### 152. Activation hardest timing written in both the model and the search engine

The question is: how hard is one activation? The answer is the largest of its squeeze timings and its required early fill.

`Activation::difficulty` in `src/core/model.cpp` (around lines 424-430) computes it for display. `Engine::act_difficulty` in `src/search/engine.cpp` (around lines 681-695) computes it again for search. The engine's number feeds `passes_ms_filter`, which is the Path limit (ms filter). Both lean on the same per-term helpers in `src/core/model.h`. But the "max over squeezes plus required fill" step is written twice.

They agree today. Both take the max over the same set from the same helpers. The engine walks the newest squeeze first. That could only change which of -0.0 and +0.0 wins a tie, and those compare equal. The verifier checked this by reading.

If one copy changed, the Path limit would keep or drop paths by a different number than the Paths tab and report show.

A single free function in `core/model.h`, over the squeeze list, the early-fill offset and the skip count, should own it. Both `Activation::difficulty` and `Engine::act_difficulty` would call it. The engine copy first appeared in native search (1f38f07, 2026-08-13); both took their current form in the port, 3b69958 (2026-08-15). The `activation_badge` copy one finder listed is not a third copy; it calls the owner and adds a separate rule.

*Owner proposed: a core free function in core/model.h over (squeezes, e_offset, skips). Candidates: squeeze-27, fills-8.*

#### 153. Activation::is_difficult re-words the path's 'hardest above 2 ms' rule

The question is: is this activation's timing difficult enough for the warning colour? Two functions in `src/core/model.cpp` phrase it differently.

`Path::is_difficult` (around line 568) asks whether `difficulty()` is above `kDifficultMs`. `Activation::is_difficult` (around lines 432-437) writes its own loop. It asks whether the required early fill is above `kDifficultMs`, or whether any squeeze is difficult.

| Wording | Empty activation | Hardest exactly 2.0 | Hardest 2.1 |
|---|---|---|---|
| any item above 2.0 | false | false | true |
| max above 2.0 | false | false | true |

They agree on every input today. "Some item above the line" means the same as "the largest item above the line". One finder ran an 84-case grid with zero mismatches, and the verifier confirmed by reading.

If one wording changed, the badge colour and the path-button colour could split for the same squeeze.

`Activation::is_difficult` should read `difficulty()` the way `Path` does, or use the shared difficult-floor predicate from the 2.0 ms finding. Both came in with the port, 3b69958 (2026-08-15).

*Owner proposed: Activation::is_difficult in src/core/model.cpp, as difficulty() > kDifficultMs. Candidates: squeeze-29, screenA-9.*

#### 154. Activation badge re-works out which part is hardest by matching numbers

The question is: which part of an activation is its hardest, a squeeze in, a squeeze out, or the early fill? `Activation::difficulty` in `src/core/model.cpp` (around lines 424-430) finds the largest value but throws away which part it came from. So `activation_badge` in `src/app/path_view.cpp` (around lines 44-51) works it out again. It loops over the squeezes and looks for one whose value equals the hardest. If none matches, it says "early fill".

They agree today. The hardest value is the very number one of the parts produced, so the equality test finds it. The verifier checked this by reading.

Two choices live only in the badge. A tie between a squeeze and an equal early fill names the squeeze. The number prints as whole ms with `%.0f`. Elsewhere the same activation prints with one decimal on the path button, and `notationstr_verbose` in `model.cpp` truncates instead. By reading, 162.7 ms shows as 163 in the badge and 162 in `notationstr_verbose`.

If `difficulty()` ever gained or changed a term, the badge could name the wrong part.

`Activation` should return the hardest part's kind with its value, with rounding through a shared display helper. This came in 577ddc2 (2026-09-27), drafted in plan commit f1c6a3d. No user decision on the tie rule or rounding was found in docs/adr, CONTEXT.md, or the 2026-09-27 UI-redesign plan and spec.

*Owner proposed: core/model Activation: return the hardest part's kind with its value. Candidates: display-11, screenA-6, screenA-18.*

#### 155. Report page recomputes where Beyond starts and who is past it

The questions: at what ms does the Beyond tier start, and which rows are in it?

The owner of the edge is `beyond_edge_ms` in `src/core/squeeze_rating.cpp` (around line 207). It takes the largest finite cutoff of `timing_tiers`. The owner of membership is `tier_for` in `src/app/report.cpp` (around line 227). The report page script repeats both. `kPageJs` in `report.cpp` (line 70) recomputes `BEYOND` with the same max over the same table. The "Past N ms" tile (line 146) then counts `r.ms >= BEYOND` instead of rows whose tier is Beyond. The Timing column text (line 106) restates the edge in words: "at least twice the hit window". The footer is not a copy, because it calls `beyond_edge_ms`.

| ms at W=85 | tier_for | page tile |
|---|---|---|
| 169.9 | Insane+ | not counted |
| 170.0 | Beyond | counted |
| null | None | not counted |

They agree for every input today. The verifier showed this by reading, plus a node run printing 170. A non-integer window cannot happen, because `hit_window_ms` is an int.

Nothing looks wrong today. If the copies split, the "Past N ms" count would differ from the Beyond chips and the Timing filter. The column text could also name the wrong edge.

`beyond_edge_ms` and `tier_for` should own this. The payload should carry the edge as a number. The tile should count `r.tier === 'Beyond'`, and the column text should print the number.

*Owner proposed: beyond_edge_ms (edge) and tier_for (membership). Candidates: squeeze-30, tempo-14, display-14, screenB-9.*

#### 156. The unextended SP end is recomputed by hydra_replay and by test fixtures

The question is where Star Power ends when no phrase is collected during it. The answer is the activation tick plus two measures per banked SP bar. That end point is the "unextended" or "nominal" deactivation node.

The engine answers it in `ScoreGraph::add_act_edge` in `src/search/graph.cpp` (lines 337-340). It fills `activation_initial_end_times` with `plusmeasure(activation, sp_bars_to_measures(sp))`. `sp_bars_to_measures` is `kMeasuresPerSpBar * bars` in `src/core/timing.h`, so two measures per bar. The engine computes this value but never stores it on the record.

Two tools recompute it with the same formula. `paths_json` in `tools/replay_json.cpp` (lines 86-93) writes it as `nominal_deact_tick` in the dump JSON, next to the real `deact_tick`. `check_chart` in `tools/replay.cpp` (lines 767-775) prints it as "nominal" on selfcheck FAIL lines.

Test fixtures rebuild it too. `sp_act_at` in `tests/test_preview_view.cpp` (lines 146-153) chains the same two calls the engine uses and writes the result as the fixture's `deact_tick`. That fixture stands in for an engine record. The fixtures in `tests/test_squeeze_rating.cpp` go one step further. They pass a literal measure count straight to `plusmeasure`, skipping even `sp_bars_to_measures`. This happens at lines 34, 56, 66, 97, 135, 160, 174 and 201.

The copies are worded differently, so here is how each one writes the end.

| Place | How it writes the end |
|---|---|
| engine, `add_act_edge` | `plusmeasure(act, sp_bars_to_measures(sp))` |
| `paths_json` and `check_chart` (hydra_replay) | the same formula, from the stored `sp_meter` |
| Preview fixture, `sp_act_at` | the same two calls |
| squeeze-rating fixtures | `plusmeasure(act, 16)` for a full 8-bar meter, 18 (2B+2) for a SqIn, and 6 or 4 elsewhere |

They all agree today. A run showed it for the tools: an activation at tick 1920 with meter 2 printed nominal 2688, matching the graph. The tools match because the stored `sp_meter` is already the capped meter, and every copy uses the same `plusmeasure`. One finder claimed the JSON ignores the SP cap; the verifier refuted that. The fixtures were checked by reading. A few squeeze lines also pin the resulting tick: 21840 at line 37, 12960 at line 137 and 69120 at line 202.

Nothing wrong appears on screen today. If the engine's SP-length rule changed, the JSON field and the selfcheck line would go stale. The fixtures would keep the old rule and stop matching real records, so the Preview and squeeze-rating tests could not catch the change.

The two findings disagreed on the owner. One said the engine's stamped `deact_tick` (ADR 0011) should own this. The other said `deact_tick` is a different fact and cannot replace it. The code supports the second view. `deact_tick` is the real end, which moves later when a phrase is collected mid-SP. It equals the nominal end only when nothing is collected. That is true in `sp_act_at`, whose comment says a fixture that collects a phrase overwrites `deact_tick` itself. So for the tools, the graph should own the nominal end. A shared helper, or a field stamped onto `Activation`, would let them read it instead of redoing the math. For the fixtures, the cleaner source is literal ticks or ticks taken from a search run, not a rebuilt formula.

The tool copies came in with 30c4008 (2026-09-03). The `sp_act_at` fixture came in with 711ba62 (2026-09-26). The squeeze-rating line 34 copy also appears in the squeeze-32 finding.

*Owner proposed: ScoreGraph::add_act_edge in src/search/graph.cpp (or a shared initial-end helper it calls); fixtures take literal ticks or engine output. Candidates: display-28, screenB-19, spwin-15, squeeze-31, tempo-12.*

#### 157. Drain box and highway floor each decide whether SP is running

The question is whether Star Power is running at the playhead.

Two Preview modules answer it from the same activation fields. `build_drain_box` in `src/app/preview_view.cpp` (around line 535) says yes when `has_sp_end` and `ms <= now < sp_end_ms`. That picks the "SP drain" header and gold colour. `build_track_state` in `src/render/track_state.cpp` (lines 88-90) builds the interval `[ms, sp_end_ms]` for the tinted floor. `toggle_at` (lines 52-66) turns it on at the start and off at the end instant.

| Playhead | Drain box | Floor tint |
|---|---|---|
| now == ms | running | on |
| ms < now < sp_end_ms | running | on |
| now == sp_end_ms | idle | off |

They agree today, by reading, including at both edges. The only theoretical gap is that track_state compares seconds with exact float equality while the box compares milliseconds. One finder claimed a split for a record with `has_sp_end` and `sp_meter == 0`. The engine cannot write that (`branch_activate` refuses fewer than 2 bars). The gauge's `sp_meter > 0` guard and the half-tick span edge in track_state answer different questions, so those parts were refuted.

If the two drifted, the gold "SP drain" header and the tinted floor would disagree on the frame at the deact node.

One helper on the Preview scene should own the predicate, so both read one answer. The drain box copy came in with 893e107 (2026-09-27); the track_state interval is older.

*Owner proposed: One PreviewScene / PreviewActivation helper, e.g. sp_running_at(ms), read by both. Candidates: display-26, screenB-5, tempo-16.*

#### 158. Preview SP gauge applies the at-least-one-bar cap floor twice

The question is what SP cap the Preview gauge uses, and what happens to a cap below 1. (The SP cap is the most bars of Star Power the meter can hold.)

Two places answer the "below 1 becomes 1" part. `build_sp_meter_curve` in `src/app/preview_view.cpp` (line 75) sets the curve's cap to 1 when the cap it is handed is below 1. The gauge draw in `render_preview_panel`, in `src/ui/preview_tab.cpp` (lines 538-540), floors it again with `std::max(1, pc->sp_meter_cap())`. That same spot also re-clamps the fill fraction to 0..1, even though the curve already keeps the bank between 0 and the cap.

The finder first suspected the cap itself was stored twice: `sp_cap_` in `PreviewController` and `scene_.sp_meter.cap`. The verifier refuted that, and the code agrees. `sp_cap_` is the cap the controller asked for. The scene's cap is the one the scene was actually built with, computed from that request. The gauge reads the built cap on purpose, so it matches the curve on screen while a rebuild is in flight. The one place that picks the cap is still `render_preview_panel` (line 194), which uses the viewed record's cap, or the Clone Hero cap when there is no ready record.

| Raw cap | preview_view.cpp | preview_tab.cpp |
|---|---|---|
| 4 | 4 | 4 |
| 1 | 1 | 1 |
| 0 or -3 | 1 | 1 (already 1 when it arrives) |

They agree today, shown by reading. They cannot split on any input. The second floor only ever sees the first floor's output, because `sp_meter_cap()` returns `scene_.sp_meter.cap`. Before any scene is built, that field defaults to `kCloneHeroSpCap` (`preview_view.h` line 166), so it is never below 1 there either. A cap below 1 looks unreachable anyway. `config.cpp` (line 91) only accepts an `sp_cap` of 1 or more from the INI file, and `settings_bar.cpp` (line 86) stores `std::max(1, cap)`.

The user sees nothing different, now or for any real cap. The second copy is redundant code. The risk is only that someone later changes one floor and forgets the other.

`build_sp_meter_curve` should own the floor, since it builds the curve the gauge draws. The panel should divide by `sp_meter_cap()` directly and draw what it gets.

Both clamps came in with aab5ff1 (2026-08-31, "Preview: SP meter gauge anchored to the record"). 2facc04 (2026-09-27) moved the tab-side copy when details_view.cpp was split into one file per tab. No user decision about it was found in `docs/adr/`, CONTEXT.md or the plans.

*Owner proposed: build_sp_meter_curve in src/app/preview_view.cpp. Candidates: screenB-21, sweep-ui2-c9.*

#### 159. Gauge decides 'before or inside the window' in ms here, ticks there

The question is whether a phrase ends before an activation, inside its SP window, or after it.

One function, `build_sp_meter_curve` in `src/app/preview_view.cpp`, answers it three ways. The flat walk (line 89) compares milliseconds: phrase end before `act.ms`. The window loop (lines 120-121) stops on milliseconds against `sp_end_ms` but keeps phrases by tick. The splits (lines 133-149) compare ticks only.

They agree today, by reading. Every millisecond value here comes from the same `MsIndex::at` lookup. With positive tempos, milliseconds rise strictly with ticks, so the ms test and the tick test give the same answer. A split would need a zero or negative tempo, which was not found.

If one of the ms sources changed, a phrase on the window edge could bank at the wrong moment, and the gauge would step a frame early or late.

The function should compare ticks throughout. Its own comment at line 117 already says ticks compare exactly. Introduced by aab5ff1 (2026-08-31) and 906881d (2026-09-24).

*Owner proposed: build_sp_meter_curve, using tick comparisons throughout. Candidates: screenB-22.*

#### 160. One note's 1x value is built by two different formulas

The question is: what is one note worth at 1x? The rule is 50, plus 15 for a cymbal, doubled for a ghost or accent note. `ChordNote::basescore` in `src/core/model.cpp` (lines 81-85) computes it in one line. `category_scores` in `src/core/scoring.cpp` (lines 47-69 and 103-111) builds the same value differently. It splits it into a base part (50 + cymbal + dynamic cymbal share) and an accent or ghost part (another 50). Its per-note `dynamics_bonus` (lines 96-97) spells the combination a third way. The same function also calls `basescore()` for the squeeze-out cut, so both formulas sit side by side. The two constants have one home in `model.h`; only the formula that combines them is repeated.

| Note | `basescore` | `category_scores` split |
|---|---|---|
| plain tom | 50 | 50 |
| cymbal | 65 | 50 + 15 |
| ghost tom | 100 | 50 + ghost 50 |
| accent cymbal | 130 | 50 + 15 + 15 + accent 50 |

They agree today on every note type. A test in `tests/test_stars.cpp` (lines 86-121) ties them on the chart "87": 137,950 both ways, run today. That test is a cross-check, not a second production copy. `Path::chart_base_score` (around line 606) reads the `category_scores` side for the Stars tab.

If one changed, the Stars tab base score and cutoffs and the Song Details average multiplier would split from the Paths tab's squeezed-out costs and multiplier-squeeze points.

`ChordNote::basescore` should own it, with `category_scores` reading it. This came from 3b69958 (2026-08-15). 7dafadd (2026-09-24) gave the constants one home but left the formula in both places. ADR 0012 decides the ghost/accent doubling. No user decision was found for the 50 base or the +15 cymbal in `docs/adr/` or CONTEXT.md.

*Owner proposed: core/model ChordNote::basescore (plus one per-note dynamic-share helper). Candidates: score-4, screenA-24.*

#### 161. A note's Star Power value is computed twice inside category_scores

The question is: what is a note's full value while Star Power is on? That is also what squeezing the note out of SP costs. `category_scores` in `src/core/scoring.cpp` answers it twice. Lines 84-87 add up eight separate SP terms per note. Line 75 computes the squeeze-out cut as `note.basescore() * combo_multiplier`. Lines 107-109 sum the same eight terms again for the chord total. A test in `tests/test_model.cpp` (around line 44) recomputes the cut as basescore times `to_multiplier(combo+1+i)`.

| Form | Formula |
|---|---|
| eight-term sum | 50 + cymb + 50·extra + cymb·extra + (dyn ? 50) + dyn_cymb + (dyn ? 50·extra) + dyn_cymb·extra |
| squeeze-out cut | basescore × multiplier |

They agree today. The verifier checked the algebra for tom, cymbal, ghost or accent tom, ghost or accent cymbal, and kick: both come to basescore × multiplier. The test passes.

If one term list changed, the Paths tab "squeezed out (-N)" cost would stop matching the SP points the note really adds.

`category_scores` should compute the value once and take the cut from it. The eight-term sum came from 3b69958 (2026-08-15). The basescore form of the cut came in 7dafadd (2026-09-24). This sits next to the two-formula note value in the finding above.

*Owner proposed: category_scores in src/core/scoring.cpp. Candidates: score-5.*

#### 162. Star Power doubling lives in the scorer and again as a display constant

The question is: how much does Star Power multiply a chord? `category_scores` in `src/core/scoring.cpp` (lines 57-60, 66-69, 107-109) pays SP as exactly one extra copy of the chord's value. So SP is x2 by construction, and the scorer never reads a constant. `kStarPowerMultiplier = 2` in `src/core/timing.h` (line 27) is a separate copy. `shown_multiplier` in `src/core/timing.cpp` (lines 16-18) uses it to draw the multiplier disc, and `replay_path` in `src/core/replay.cpp` (line 135) calls that.

They agree today, because 2 matches one extra copy. Changing the constant would change only the disc, not the points paid.

If they split, the Preview score box would show a multiplier disc (say x8) that does not match the points being added.

`category_scores` should own it by paying `kStarPowerMultiplier - 1` extra copies. The other way works too: the display could take its factor from the scorer. The constant came in with 60a1f5e (2026-09-25). No user decision was found in `docs/adr/`, CONTEXT.md, or that agent-written commit message.

*Owner proposed: core/scoring (category_scores should read kStarPowerMultiplier). Candidates: score-6.*

#### 163. Per-chord solo bonus formula written in engine and replay

The question is: how many solo-bonus points does a chord earn? The rule is 100 per note when the chord is in a solo, else 0. `ScoreGraph::build` in `src/search/graph.cpp` (lines 109-110) writes it as `kSoloBonusPerNote * chord.count()` when `flag_solo` is set. `replay_path` in `src/core/replay.cpp` (lines 140-141) writes the same expression again. Both read the one constant in `src/core/model.h` (line 92), so only the small rule around it is repeated.

They agree today, by reading.

If one changed, the Preview running score and the replay JSON would disagree with the Score breakdown "Solo Bonus" line.

A `solo_bonus(chord, flag_solo)` helper in core, next to the constant, should own it, and both places should call it. The replay copy came in 30c4008 (2026-09-03). The shared constant came in 7dafadd (2026-09-24). No user decision for 100 per note was found in `docs/adr/` or CONTEXT.md. The 2026-09-27 stars-tab plan states it as a game fact in its own prose.

*Owner proposed: core (a solo_bonus(chord, flag_solo) helper next to kSoloBonusPerNote). Candidates: score-8.*

#### 164. Running combo walked separately by the graph, the replay, the scorer and a test

The question is: what is the combo before and after each chord? The rule is simple. The combo starts at 0. Each chord adds its note count. It never resets, because every path is scored as a full combo.

Four places walk that count. `ScoreGraph::build` in `src/search/graph.cpp` keeps its own counter, `combo_`. It reads the value before the chord at line 114 to spot multiplier squeezes, then adds `chord.count()` at line 123. `replay_path` in `src/core/replay.cpp` keeps a second counter, `combo`, starting at line 68. It stores the value before the chord as `combo_before` at line 92, adds the count at line 145, and stores the result as `combo_after`. `category_scores` in `src/core/scoring.cpp` steps the same count one note at a time at line 41 to price each note, but throws the end value away, so both callers add the count again themselves. The test "the graph finds the chart's multiplier squeezes once, in chart order" in `tests/test_search.cpp` (lines 114 to 123) walks it a fourth time. Its comment calls this "an independent spelling".

| Place | Starts at | Steps by | Reads it |
|---|---|---|---|
| ScoreGraph::build | 0 | `chord.count()` after the chord | before the chord |
| replay_path | 0 | `chord.count()` after the chord | before and after |
| category_scores | caller's combo | 1 per note, inside the chord | per note, end value dropped |
| test_search.cpp | 0 | `chord.count()` after the chord | before the chord |

They all agree today. There is no edge input where they differ. This was shown by reading the code and by a hydra_tests run over the chart corpus. The two source findings did not disagree on any fact; one cited only the add lines and the other also the read lines, and both sets of line numbers check out.

Nothing is wrong on screen today. The replay's `combo_after` feeds the Preview's "x? · combo ?" readout (`src/app/preview_view.cpp` line 213). If the graph and the replay ever counted differently, the Preview's combo and multiplier would drift from the engine's score. The multiplier squeezes the search lists and the score the replay shows could then disagree for the same path.

`category_scores` should own it. It already walks the count note by note, so it can return the combo after the chord. The graph and the replay would then read that value instead of adding the count a second time. The test can keep its hand-written walk only if the user decides test oracles are exempt from the derive-once rule. A code comment calling it "independent" is not that decision.

The graph and scorer copies date from 3b69958 (2026-08-15, the C++ port checkpoint). The replay copy came in 30c4008 (2026-09-03, hydra_replay). The test copy came in 711ba62 (2026-09-26, record format v7).

*Owner proposed: category_scores in src/core/scoring.cpp (return the combo after the chord), read by ScoreGraph::build and replay_path. Candidates: score-9, sweep-tests3-c3.*

#### 165. A path's total score is summed in three places

The question is: what is a path's total score? It is the sum of six stored categories. `Path::totalscore` in `src/core/model.cpp` (lines 479-482) adds them. `ReplayScore::total` in `src/core/replay.h` (line 80) adds the same six on its own. The engine in `src/search/engine.cpp` keeps a third copy: a running `p.score` beside its six counters `p.sc[]`. Three functions add to both, `advance` (lines 472-479), `branch_activate` (571-572) and `create_deactivated_path` (640-641). The engine ranks paths and picks the optimum by `p.score`, but it stores `sc[]` as the six shown fields (lines 1016-1021).

They agree today, because every adder updates both.

If one adder missed one of the two, the engine would rank paths by a total that differs from the one shown. It could even pick a path as optimal that is not the highest shown score.

`Path::totalscore` should own the rule. The engine could compute `p.score` from `sc[]`, and `ReplayScore` could share one six-field sum. `Path::totalscore` and the engine's `p.score` came from 3b69958 (2026-08-15), going back to 1f38f07 (2026-08-13). `ReplayScore` came in 30c4008 (2026-09-03).

*Owner proposed: core/model Path::totalscore. Candidates: score-10.*

#### 166. Stars tab and its GUI test add the solo bonus to each cutoff themselves; core/stars only ever subtracts it

The question is how the solo bonus relates to star cutoffs. Clone Hero counts stars before it adds the solo bonus. So a star counts when the score without the solo bonus reaches the cutoff. The flip side is the score you see on screen at that cutoff if you collect every solo bonus: the cutoff plus the solo bonus.

Four places write this rule, and one more repeats it. `path_stars` in `src/core/stars.cpp` (line 32) passes `totalscore() - score_solo` to `stars_for_score`. `Path::avg_mult` in `src/core/model.cpp` (line 611) writes the same subtraction for the average multiplier. `render_stars_panel` in `src/ui/stars_tab.cpp` (line 50) works out the inverse in UI code: it prints `cutoff + sc.solo_bonus` under "With full solo bonus". It has to, because `star_cutoffs` in core fills the base, the solo bonus and the seven cutoffs but no with-solo figure. `StarCutoffs` in `src/core/stars.h` (lines 30-34) has no field for it. Finally, the GUI test `test_stars` in `tests/ui/uitest_details.cpp` builds its expected text with the same sum. One finder cited lines 302-305 for that; the code shows the loop at lines 299-302, with the sum on line 301.

| Place | Form of the rule |
|---|---|
| `path_stars`, `avg_mult` | total minus solo bonus (compared to cutoff, or divided by base) |
| Stars tab cell | cutoff plus solo bonus |
| GUI test expectation | cutoff plus solo bonus, copied from the tab |

They all agree today. For any total, solo bonus and cutoff, "total minus solo reaches cutoff" and "total reaches cutoff plus solo" are the same inequality, edges included. The verifiers showed this by reading the code. No input makes them disagree now.

The weak spots are what happens after a change. If solo handling in `path_stars` changed, the "With full solo bonus" column would keep the old addition. It would then show the wrong target score, and the headline star count, the average multiplier and that column would stop matching. The GUI test would not catch it. It takes its expected value from the same sum, so a wrong sum makes the test wrong the same way and it still passes. Today the user sees nothing wrong on screen.

The finder for one copy also listed a line in `replay_path` (`src/core/replay.cpp`, line 157). The verifier showed it answers a different question. It holds back only an unfinished solo run's bonus for the on-screen counter, so it is not a copy of this rule. That rule belongs to the solo-run finding.

`core/stars` should own the with-solo figure. It already owns the star rule, so the sum and the subtraction would sit in one module. The fix is a with-solo array in `StarCutoffs`, plus a `score_without_solo` helper beside `path_stars` that `avg_mult` can call. Then the tab only draws, and the test reads the core value instead of redoing the sum.

History, from the commits. The `avg_mult` subtraction came from 3b69958 (2026-08-15). `path_stars` came in bc01fa6 and da26438, and the Stars tab column in ce6c9e1, all on 2026-09-27. The user asked for and approved the column. In `docs/superpowers/plans/2026-09-27-stars-tab.md` they picked "Extra column (Recommended)" (lines 49-51), and `docs/UserGuide.md` line 180 describes it. The same plan says the UI must not recompute the base, solo bonus or cutoffs (line 29). Yet its own spec says "The last cell is cutoff + solo bonus" (line 37), and its test step asks the GUI test to check "7-star cutoff + solo bonus" (line 316). So the user's approval covers the column, not where the sum lives. Putting the sum in the UI and the test was the plan author's choice, and it cuts against the plan's own no-recompute rule.

*Owner proposed: core/stars: StarCutoffs carries a with-solo array (and a score_without_solo helper beside path_stars), so the tab and its test only read it. Candidates: display-34, score-11, score-12.*

#### 167. Solo section boundaries decided in replay and in Preview scene

The question is: which back-to-back chords form one solo section? `replay_path` in `src/core/replay.cpp` (lines 151-156) ends a solo run when the next chord is not solo-flagged or the song ends. It releases the held-back solo bonus onto the on-screen score at that chord. `build_preview_scene` in `src/app/preview_view.cpp` (lines 229-247 and 263-264) joins consecutive solo-flagged timestamps into one span to draw the solo band. It closes the span at the first non-solo chord or at the end of the song.

They agree today, by reading. Both use runs of adjacent `flag_solo` chords in `song.sequence`, and both close a run that reaches the last chord.

If one changed, the Preview's solo band could end on a different chord from where the score box jumps by the solo bonus.

`parse/song` should own it by storing solo sections as data that both read. The replay version came in 30c4008 (2026-09-03). The Preview spans came in ba0885a (2026-08-22). Holding the bonus back until the run ends has no user decision either. The verifier looked in `docs/adr/`, CONTEXT.md and the plan headers. Only a comment in `replay.h` (lines 161-163) explains it.

*Owner proposed: parse/song (solo sections stored as data both read). Candidates: score-22.*

#### 168. Fill and DM report pages decide 'which side is higher' twice

The question is which side of a comparison is higher. On the fill page that means CH 1.0 versus 1.1. On the DM page it means a posted score versus Hydra's optimal.

C++ answers it first and stores the answer as the row's status. `collect_fill_rows` in `src/app/fill_report.cpp` (line 198) sets the status from the delta's sign. `collect_dm_rows` in `src/app/dm_report.cpp` (line 204) sets 'above optimal' when score is greater than optimal. Then each page's script (the `kPageJs` string, `cells` around lines 98-102 in both files) decides again from the delta's sign to pick the cell colour. The fill script's stats (lines 119-120) also split gains and losses by sign.

| Input | Fill status | Fill colour | DM status | DM cell |
|---|---|---|---|---|
| delta > 0 | '1.1 higher' | pos, '+' | 'matched' | plain |
| delta < 0 | '1.0 higher' | neg | 'above optimal' | neg, '+N over' |
| delta = 0 | 'same' | dim | 'matched' | plain |

They agree on every input today. The DM delta is optimal minus score, so 'delta below zero' and 'score above optimal' are the same test. The verifier checked this by reading. The tally counts in JS and in `tally_dm_rows` both key on the C++ status string, so they do not re-decide the rule.

If the copies drifted, a row's status chip and its coloured delta cell would disagree.

The DM copies came in 21ddf53 (2026-08-18), the fill copies in 406d473 (2026-08-31). The C++ status should own it, because it already exists; the script should read `r.status` or a flag.

*Owner proposed: C++ collect_fill_rows / collect_dm_rows; the page script keys on r.status or a shipped flag. Candidates: display-40, screenB-13, score-17.*

#### 169. Report sorts paths by score again after the engine already did

The question is what order the paths are ranked in: highest score first.

The engine sets that order. `src/search/engine.cpp` (lines 1137-1144) sorts the root paths by score, highest first, before saving them. Each variant copies its parent's score (`prepare_variants`, around line 1310). Then `collect_rows` in `src/app/report.cpp` (lines 267-269) sorts `all_paths()` again with the same comparison. `build_path_list` in `src/app/path_view.cpp` groups neighbouring equal scores, but it only relies on the order and does not sort, so it is not a copy.

They agree today. Paths arrive already sorted and every variant ties its parent, so the report's stable sort changes nothing. The verifier showed this by reading. A claimed drift where traversal order breaks has no reachable input.

If the engine's order ever changed, the report would keep sorting by score while the Paths tab would show split or misordered groups.

The report sort came in 3b69958 (2026-08-15). The engine, through `HydraRecord::all_paths`, should own the order, because it is the one place that decides it. The report can drop its own sort.

*Owner proposed: engine / HydraRecord::all_paths guarantees the order; the report drops its re-sort. Candidates: score-16.*

#### 170. Leaderboard page prints 'SP cap 4' as text, and its Clone Hero rules gate is split between the button and the report

The question is when a library may be compared with the dmleaderboards site, and at which Hydra result. The site plays by Clone Hero's rules: Expert, a 4-bar Star Power cap, and the CH 1.1 fill rule. ADR 0003 (lines 57 to 59) pins the comparison to 4 bars for that reason. ADR 0010's 2026-09-29 note says the comparison is disabled while "1.0 fills" is on, but not where that rule lives.

Two parts of the answer are written more than once.

The first part is the cap number. `collect_dm_rows` in `src/app/dm_report.cpp` (line 157) queries results at `kCloneHeroSpCap`. The page text then types the cap as plain "SP cap 4" in three places in the same file: the Hydra optimal column note in `kPageJs` (line 78), the Status column note (line 86), and the footer in `generate_dm_report` (lines 286-287). Other places already show the right pattern. The toolbar tooltip in `src/ui/library_toolbar.cpp` (lines 114-116) and the settings bar (`src/ui/settings_bar.cpp`, line 76) format the constant into their text.

The second part is the gate. `render_actions_row` in `src/ui/library_toolbar.cpp` (lines 94-121) disables the button unless all three rules hold, and its tooltip names the missing one. `collect_dm_rows` (lines 149-157) forces only the 4-bar cap. It passes the caller's chart mode (which carries the difficulty) and the lens (which carries the fill rule) through unchanged. I checked both by reading the code.

| Rule | Toolbar button checks | collect_dm_rows checks |
|---|---|---|
| Expert | yes | no |
| 4-bar cap | yes | yes (forces it) |
| 1.1 fills | yes | no |

They agree today, shown by reading. The only caller is `src/ui/dm_jobs.cpp`, through `generate_dm_report`, and it sits behind that button. The picker is a modal popup, so the setting cannot change while it is open. So a 1.0-fill comparison cannot happen through the GUI today.

Two inputs would split them. If `kCloneHeroSpCap` ever changed, the page would compare scores at the new cap while still saying "SP cap 4". If a new caller, such as a CLI command or a test, skipped the button with 1.0 fills on, the page would rate CH 1.0 scores as above or below optimal against 1.1 leaderboard runs. Today the user sees nothing wrong on screen.

`collect_dm_rows` should own all three rules, because it is the code that produces the comparison. The button can ask it whether the comparison is allowed and why not. `dm_report` should format `kCloneHeroSpCap` into the page text the way the settings bar does. The literal text came in with 4bdada0 (2026-09-27) and the three-rule toolbar gate with a591bba (2026-09-27).

*Owner proposed: collect_dm_rows in src/app/dm_report.cpp: it prints kCloneHeroSpCap and owns all three Clone Hero rules; the toolbar button asks it. Candidates: display-22, fills-20, screenB-14.*

#### 171. Stars filter error hard-codes '0 to 7' beside kMaxStars

The question is the most stars a chart can have, as the library's `stars:` search filter reports it.

The owner is `kMaxStars = 7` in `src/core/stars.h` (line 19). `parse_stars` in `src/app/library_query.cpp` (lines 250-260) checks the typed number against it at line 256. The error text right beside it does not use it. `kStarsError` at library_query.cpp line 19 types the literal "stars: needs a number from 0 to 7". The same literal shows up again in three places. `tests/test_library_query.cpp` pins it at lines 215 and 221. A header comment in `src/app/library_query.h` (line 46) repeats it. `docs/UserGuide.md` line 81 says "0 to 7" too.

| Filter value | Range check | Message |
|---|---|---|
| 0 to 7 | accepted | none |
| -1, 8, empty, x | rejected | "from 0 to 7" |

They agree today, so no input makes them disagree. Both finders showed this by reading. An earlier drift claim on this spot does not hold; the copy does.

Nothing wrong is visible now. If `kMaxStars` ever changed, the search box would accept one range while its message named another.

`kMaxStars` should own the number, and the message should be built from it. `CONTEXT.md` "Star cutoff" (lines 105-108) records 1 to 7 stars, which covers the value but not the copied text. The 2026-09-27 UI redesign plan (`docs/superpowers/plans/2026-09-27-ui-redesign.md`, lines 1390-1392) supplies the wording and itself names `kMaxStars` as the source of the range. That is plan text, not a user quote. Nothing in docs/adr/ says more. This came in with b89beed (2026-09-27).

*Owner proposed: kMaxStars in src/core/stars.h, formatted into the error message. Candidates: score-24, sweep-tests2-c8.*

#### 172. 'Paths kept' and the path-count sort column count paths two ways

The question is how many paths a record kept.

Two places count them in different ways. `build_record_status` in `src/app/path_view.cpp` (lines 133-134) shows "Paths kept:" as `record.all_paths().size()`. That flattens the path tree, roots plus nested variants (`flatten_paths` in `model.cpp`). `summarize_record` in `src/store/record_store.cpp` (lines 497-504) fills the stored sort column by summing each root's `tied_pathcount()`. That is a running count of 1 plus each variant's count (`recount_tied_paths` in `model.cpp`).

They agree today. The tied count is recounted on every decode (`path_codec.cpp`) and when the engine finishes (`engine.cpp`), so it always equals the subtree size. They could differ only if code changed a record in memory without recounting.

If that happened, the details panel's "Paths kept" and the library's path-count sort would disagree for the same song. One `HydraRecord` method should own the count, and both places should call it. The sum came with the port (3b69958, 2026-08-15). The flatten came with 09ad9fa (2026-08-22).

*Owner proposed: one HydraRecord method (a path count) used by both. Candidates: display-31.*

#### 173. Search keeps a shortcut copy of the 'within N scores' rule

The question is whether a path is inside the score range the user picked ("Within N scores").

`Engine::reduce_group` in `src/search/engine.cpp` answers it twice. A shortcut (lines 719-742) keeps every path when the group has at most N+1 paths, none are filtered, and all scores differ. The general rule (lines 846-866) removes a path that more than N others outscore, with extra handling for filtered paths and for Points mode.

| Group | Shortcut | General rule |
|---|---|---|
| N+1 distinct, unfiltered scores | keep all | lowest is outscored by N, not more than N: keep all |
| Any tie or filtered path | skipped | decides |
| One path | keep | returns early: keep |

They agree on every input today, shown by reading. The shortcut restates a result of the general rule. If the general rule changed, small groups would keep the old behavior silently.

On screen, the paths kept for small groups would then differ from what "Within N scores" promises. `reduce_group` should hold one band test that the shortcut calls or checks against. The shortcut came in with 1f38f07 (2026-08-13). `docs/UserGuide.md` line 49 describes the band; there is no ADR.

*Owner proposed: Engine::reduce_group in src/search/engine.cpp (one band predicate). Candidates: score-23.*

#### 174. Probe experiment scripts and tests hard-code engine field offsets instead of reading constants.py

The question is where each field of Clone Hero's drums engine object sits in memory: total window, back window, score, flags and the rest. An offset is the distance in bytes from the start of that object to the field.

`tools/ch_probe/constants.py` (lines 66-92) owns the answer. It holds the `OFF_*` constants, such as `OFF_SCORE = 0x94` (line 80) and the total window at `0x20` (line 66), plus `PRECISION_MODE_BIT`. `EngineModel` in `engine.py` (lines 102-144) reads them, and so does `experiments/live.py`. The constants.py docstring says nothing else hard-codes an address. That claim is false today.

Three experiment scripts type raw offsets in their `main`. `find_engine.py` uses `h + 0x20` (lines 74, 91, 109), then 0x30, 0x38, 0x8C and 0x198 (lines 92-95). `poll_windows.py` uses 0x20 (lines 66, 88) and 0x30 (line 67). `find_clock3.py` uses 0x20 (lines 61, 63).

`hit_detect.py` keeps its own module constants (lines 32-35), even though it already imports `constants` for other values. Two are private copies: `OFF_TOTAL_WINDOW = 0x20` (line 32) and `OFF_SCORE = 0x94` (line 35, read at line 98). Two more are offsets constants.py does not have at all: `0x28` ("time A") and `0xb0` (a hits counter).

Two tests repeat literals too. `test_engine_finder.py` (line 58) subtracts 0x30 instead of `C.OFF_BACK_WINDOW`. `test_hit_window_scripts.py` (line 44) packs the flags as 0x1000 instead of `C.PRECISION_MODE_BIT`.

Every literal matches its constants.py value today. This was checked by reading. Three newer scripts (`engine_finder`, `watch_window`, `walk_edges`) skip `EngineModel` but still spell their offsets through constants, so they are not number copies.

If a Clone Hero update moved a field and only constants.py were fixed, these four scripts would read the wrong bytes. hit_detect would log a wrong score or window, the others would print wrong values, and the two tests would keep checking the old layout. This is developer console output only. The Hydra GUI is untouched.

The repo already guards against this elsewhere. `tests/test_play_chart.py` (line 89) forbids private copies in `play_chart`, and `tests/test_hit_window_scripts.py` (line 54) does the same for `live.py`. hit_detect and the three older scripts have no such test.

The `OFF_*` constants, read through `EngineModel`, should own the layout, because the layout is one game fact. The literals came in with 283e028 (2026-09-26). Commit 288b07a (same day) set the goal of one name per offset, but did not remove these.

The two members read the plan differently, and the plan text settles it. Task 20 of `docs/superpowers/plans/2026-09-26-codebase-audit-fixes.md` (around line 15219) keeps hit_detect, find_clock3 and poll_windows on purpose, because the newer scripts had not yet run at the game. But it only repoints their imports, renames a private key call, and replaces hit_detect's `0x100` constant. It never decides to keep the other literals. So keeping the scripts was a decision; keeping their raw offsets was not. Task 21 Step 10 asks the user whether to delete three of the old scripts, and that question looks unanswered.

*Owner proposed: tools/ch_probe/constants.py (the OFF_* constants and PRECISION_MODE_BIT), read through engine.EngineModel. Candidates: score-25, sweep-probe1-c1, sweep-probe2-c2.*

#### 175. Preview finds the taken fill twice, once in the scene and again for lane colour

The question is which fill an activation took. The rule is "the fill whose end tick equals the activation tick".

It is written twice. `build_preview_scene` in `src/app/preview_view.cpp` (lines 340-344) marks the first matching fill as Taken. `build_track_state` in `src/render/track_state.cpp` (lines 94-97) searches again for a fill that is Taken and ends on the activation tick, to colour its lane.

| Case | Scene builder | Track state |
|---|---|---|
| No fill ends on the activation | none | none |
| One fill ends on it | that fill | that fill |
| Two fills end on the same tick | first one, marked Taken | first Taken one, so the same |

They agree today, shown by reading. The second copy only stays right because it filters on the Taken state the first copy set.

If one copy changed, the lane colour could land on a different fill than the one drawn as taken. Nobody sees that today.

`build_preview_scene` should own it. It already makes the decision, so it should store the taken fill's index on `PreviewActivation`, and `build_track_state` should read that. This sits next to the offered-fill finding, which concerns the same scene code.

*Owner proposed: build_preview_scene in src/app/preview_view.cpp (store the taken fill index on PreviewActivation). Candidates: display-25.*

#### 176. Early-fill window test written twice, in is_e0 and is_e_critical

The question is whether an activation sits inside the early-fill window, meaning its fill offset (`e_offset`) is under 60 ms.

It is written twice. `Activation::is_e_critical` in `src/core/model.cpp` (around line 414) tests `e_offset < kEarlyFillWindowMs`. `is_e0` in `src/core/model.h` (around line 177) restates the same test and adds `skips == 0`. The constant is 60.0 ms.

| e_offset, skips | is_e_critical | is_e0 |
|---|---|---|
| 59.9, 0 | true | true |
| 60.0, 0 | false | false |
| 59.9, 1 | true | false (by design) |

They agree today, shown by reading. Both use a strict "less than".

If one became "less than or equal", an activation at exactly 60 ms would show the "E" badge but not count as E0, or the reverse.

The engine's legality test in `Engine::branch_activate` (around line 540) uses the same constant at -60 ms. That answers a different question, so it is not a copy.

Both model lines came in at 1eb8063 (2026-09-28). One window test in `src/core/model.h` should own it, and `is_e0` should call it instead of restating the comparison.

*Owner proposed: a shared window test in src/core/model.h. Candidates: fills-7, tempo-15.*

#### 177. Legacy flag to fill rule mapping written twice in pather.cpp

The question is which fill deadline rule a run uses: Clone Hero 1.0 or 1.1.

It is answered twice in `src/search/pather.cpp`. `search_target` (lines 101-104) maps `settings.legacy_fill_deadline` to Ch10 or Ch11. `analyze_at_cap` (lines 157-160) maps `legacy_fills`, which is fed from the same setting, the same way. No other place in `src` makes this mapping.

They agree today, shown by reading: true gives Ch10 and false gives Ch11 in both.

If they drifted, `hydra_replay target` would price a named path under a different fill rule than the analysis that found it. The user would see a path's replayed score differ from the score Hydra showed for it.

The `search_target` copy came in at 30c4008 (2026-09-03). The `analyze_at_cap` copy dates from 406d473 (2026-08-31) and was rewritten in eaf3b73 (2026-09-27).

One helper on `SearchSettings`, such as `fill_rule()` in `src/search/pather.h`, should own it. Both search entry points already take those settings, so each can ask the helper.

*Owner proposed: a fill_rule() helper on SearchSettings in src/search/pather.h. Candidates: fills-5.*

#### 178. Fill-attach check copied between the MIDI and .chart parsers

The question is whether a chord has reached the current fill's end, so the fill must be attached now.

The check is copied in two parsers in `src/parse/song.cpp`. `MidiParser::push_timestamp` (lines 554-565) and `ChartParser::push_timestamp` (lines 955-964) both test "the chord has notes, a fill end is pending, and this tick is at or past it". Both then reset the fill start and end. The placement test `fill_lands_on_chord` and the attach action `apply_fill_end` are already shared; only this check is copied.

They agree today, shown by reading. In both: an empty chord, no pending fill, or a tick before the end means no attach; a tick at or past the end means attach.

If they drifted, the same chart in .mid and .chart form would place fills on different chords. The user would see different activation points and different paths for what is the same song.

Both copies came in at 3b69958 (2026-08-15). A helper beside `fill_lands_on_chord` should own the check, so both parsers ask one place when to attach.

*Owner proposed: a shared helper beside fill_lands_on_chord in src/parse/song.cpp. Candidates: fills-11.*

#### 179. Tied path count kept by the engine and recounted from the tree

The question is how many paths a leader stands for. A leader is the one path kept when several paths tie on score.

It is answered twice. `Engine::reduce_group` in `src/search/engine.cpp` (lines 760-773) keeps a running sum, `leader.tied_count += p.tied_count`, to enforce the tie limit. `Path::recount_tied_paths` in `src/core/model.cpp` (lines 531-538) recomputes "one plus the children's counts" from the finished tree. That second value is stored and shown.

They agree today by construction, shown by reading. Every merged path keeps its own branch in the tree, so one plus the children's counts equals the running sum.

If the two counting rules drifted, the "Paths kept" count the user sees would no longer match the limit the engine enforced.

The engine's running sum traces to 1f38f07 (2026-08-13) and 2c2eee4 (2026-08-18). The recount was not traced.

`Path::recount_tied_paths` should own it, because its value is the one stored and shown. The engine's sum is internal bookkeeping for the limit.

*Owner proposed: Path::recount_tied_paths in src/core/model.cpp. Candidates: fills-22.*

#### 180. The all-0 path's 0 ms limit is written twice, plus a test copy

The question: is a path inside the all-0 path's 0 ms timing limit? The all-0 path is the best path with no skipped fills, found under a 0 ms limit (CONTEXT.md "All-0 path").

Two places in `src/search/pather.cpp` answer it. `search_allzero` (around lines 75-78) sets the engine's `ms_filter = 0.0` with a hard filter. `attach_allzero` (around lines 46-48) skips that whole pass when the best path is already all-0 and `difficulty().value_or(0.0) <= 0.0`, a second literal for the same limit. The test in `tests/test_search.cpp` (line 352) restates it a third time.

The difficulty they compare is also written twice. `Engine::act_difficulty` in `src/search/engine.cpp` (around lines 681-695) and `Activation::difficulty` plus `Path::difficulty` in `src/core/model.cpp` each take the max of squeeze and early-fill difficulty.

| path difficulty | engine keeps it? | attach_allzero skips the pass? |
|---|---|---|
| unset | yes | yes |
| -5 | yes | yes |
| 0 | yes | yes |
| 0.1 | no | no |

They agree today on every input the verifier checked, by reading.

If they drifted, the "Best all-0 path" row could vanish, or show a copy of the optimal path. A named constant in `pather.cpp` should own the limit, and one rule should own path difficulty.

*Owner proposed: A named constant or predicate in src/search/pather.cpp, and one shared path-difficulty rule. Candidates: fills-19.*

#### 181. The default search depth of 4 is written in three places

The question is the default search depth, meaning how many scores or points below the best the search keeps.

Three places answer it. `Settings` in `src/app/config.h` (line 62) has `depth_value = 4`. `SearchSettings` in `src/search/pather.h` (line 25) has `depth_value = 4`. The argument struct in `tools/replay.cpp` (line 75) gives hydra_replay's `--depth` a default of 4. The zero defaults in `store::Lens` and `engine.h` answer different questions (an unset key and engine options), so they are not copies.

They agree today; all three say 4, shown by reading.

Nothing shows on screen today. If one changed, tests and tools that build `SearchSettings` directly, or hydra_replay run without `--depth`, would search a different depth from the app. Their results would not match what the GUI shows for the same chart.

The copies came in 3b69958 (2026-08-15), 515bf37 (2026-08-22) and 30c4008 (2026-09-03). No user decision was found in `docs/adr/`, `CONTEXT.md` or the commit subjects. The search module should own one constant, because the search is what the number controls.

*Owner proposed: search (one constant in src/search/pather.h used by app::Settings and tools/replay.cpp). Candidates: fills-14.*

#### 182. Audio ms-to-frames conversion written out by hand four times

How many audio frames is a span of milliseconds, and back? The audio code writes the conversion out by hand in four places.

Milliseconds to frames appears in `Playhead::seek_ms` in `src/audio/player.cpp` (around line 25). It appears again in `pad_front_ms` in `src/audio/mixer.cpp` (around line 107). Both round `ms * sample_rate / 1000`. Frames to milliseconds appears in `Playhead::position_ms` and `Playhead::length_ms` in `player.cpp` (around lines 29 and 33). Both compute `frames * 1000 / sample_rate`. A grep of src/ found no other copies.

The formulas are identical, so they agree today, shown by reading. Nothing is wrong on screen now. If one copy changed, the seek and the front pad (the silence added for a negative chart offset) could differ by a frame. The Preview audio would then sit that far off the highway.

One helper pair in `src/audio`, for example `frames_of_ms` and `ms_of_frames`, should own it. The sample rate is an audio fact, and every caller already lives in that folder. ADRs 0004 and 0006 cover the audio libraries, not this conversion.

*Owner proposed: One helper pair in src/audio (frames_of_ms / ms_of_frames). Candidates: tempo-10.*

#### 183. Rounding milliseconds to a chart tick written twice in the app

Which whole tick does a millisecond value round to? The app writes this twice.

`tick_at` in `src/app/preview_view.cpp` (around lines 427-430) rounds `MsIndex::tick_at_ms` with `llround` and clamps at zero. `build_activations` in `src/app/path_view.cpp` (around line 361) writes the same rounding inline for the timeline end, without the clamp. The missing clamp never matters there, because that code only runs when the song length is above zero.

They agree today, shown by reading. Two other places the finder named are not copies. The check in `tests/test_search.cpp` is an independent test of the stored deact tick. `tools/ch_probe/probe_chart.py` writes its own single-tempo charts.

Right next to it, `path_view.cpp` (around line 363) adds 1 to the measure by hand. It does not call its own `format_measure` (line 28). That also agrees today.

If the rounding or the 1-based measure changed in one place only, the Paths timeline end and the Preview time box would name different measures for the same moment.

A rounded ms-to-tick next to `MsIndex::tick_at_ms` should own it, since the unrounded version already lives there. `path_view` should call it and `format_measure`.

*Owner proposed: A rounded ms-to-tick on MsIndex/SongTiming (or promote preview_view's tick_at), plus format_measure for the 1-based label. Candidates: tempo-7.*

#### 184. Paths timeline left label is a hard-coded "m1"

The question is: how is a measure number written as a label? Hydra writes "m" plus the zero-based measure plus one.

Three places answer it. `format_measure` in `src/app/path_view.cpp` (around lines 25-31) is the rule. `build_activations` (around lines 360-363) rebuilds it inline for `timeline_end`, as "m" + (measure + 1). And `render_timeline` in `src/ui/paths_tab.cpp` (line 227) prints a literal "m1" for the left end.

They agree today, shown by reading. The timeline's left edge is 0 ms, which is tick 0, which `format_measure` writes as m1.

Nothing is wrong on screen now. If the numbering rule changed, for example to start at measure 0, the right label would follow only if someone edited the inline copy, and the left label would stay "m1" regardless. The two ends of the timeline would then disagree.

`build_activations` should own both labels. It should produce a `timeline_start` and a `timeline_end` from `format_measure`'s rule, and `render_timeline` should only print them.

*Owner proposed: build_activations in src/app/path_view.cpp (timeline_start and timeline_end both from format_measure's numbering). Candidates: screenA-23.*

#### 185. Drain box restates half of the Preview playhead clamp

The question is: which moment do the Preview's overlay boxes read? The playhead can in principle sit before 0 or past the song length.

`shown_ms` in `src/app/preview_view.cpp` (around lines 438-441) holds the playhead inside 0 and the length. `build_time_box` and `step_tick_ms` use it. `build_drain_box` (line 520) inlines only the lower half: below 0 becomes 0, with no upper limit. `build_score_box` (line 502) and `build_next_act_box` (line 596) read the raw playhead.

| Playhead | shown_ms | build_drain_box |
|---|---|---|
| below 0 | 0 | 0 |
| 0 to length | playhead | playhead |
| past length | length | playhead |

In the app they agree. `PreviewTransport::seek_ms` keeps the playhead inside 0 and the length, and `tick()` stops at the length (`preview_transport.cpp` around lines 61 and 79-82). The length already covers the audio tail. The finder's claim of a visible mismatch past the last note was wrong. A difference shows only when the pure functions are called directly with a time past the length, and only if a tempo change lies there. That was shown by reading.

If the transport ever let the playhead run past the length, the drain rate could come from a later tempo section than the time box shows. It came in with 893e107 (2026-09-27). `shown_ms` should own the clamp, and every box should call it.

*Owner proposed: shown_ms in src/app/preview_view.cpp, called by every box. Candidates: screenB-7.*

#### 186. "notes.mid beats notes.chart" is written separately for folders and .sng

The question is which notes file wins when a song has both notes.mid and notes.chart.

Two places answer it. `discover_charts` in `src/app/analysis.cpp` (line 373) takes `found_mid ? found_mid : found_chart` for a folder. `load_songpath_sng` in `src/parse/song.cpp` (around lines 1095-1111) breaks on the first notes.mid inside a .sng, otherwise keeps the last notes.chart. Both use `notes_file_format` to recognise the names, but each writes the preference on its own.

They agree today, shown by reading. Both pick notes.mid when both exist. Two notes.chart entries can only happen inside a .sng, where the last wins; a Windows folder cannot hold two.

Nothing is wrong on screen today. If one copy changed, folder charts and .sng charts would silently analyze different files for the same layout.

A single helper in `chart_files.cpp` should own the choice, since that file already owns the name test. The copy in `play_chart.py`, which prefers notes.chart, is part of the play_chart finding.

*Owner proposed: One pick-notes-file helper in src/parse/chart_files.cpp. Candidates: parse-11.*

#### 187. Preview checks .sng/.srb by suffix instead of asking chart_format_of

The question is which chart format a file is.

`chart_format_of` in `src/parse/chart_files.cpp` (around lines 9-15) is the owner. Its header says the scan and the loaders ask it "so the two can never disagree". `resolve_preview_source` in `src/app/preview_source.cpp` tests the suffix itself, with `ends_with_ci` on `.sng` (line 341) and `.srb` (line 346), to choose the audio and delay route. Everything else is treated as a loose folder.

They agree today, shown by reading. The suffixes and case folding are the same, so `x.SNG` and `x.srb` land the same way in both.

Nothing is wrong on screen today. If a new container type were added to `chart_format_of`, the Preview would treat it as a folder and look for loose audio and song.ini, so it would play silent.

`chart_format_of` should stay the owner, and the Preview should call it. The finder dates the Preview copy to ba0885a (2026-08-22); the verifier did not re-check that.

*Owner proposed: chart_format_of in src/parse/chart_files.cpp. Candidates: parse-10.*

#### 188. The .srb stream layout is walked separately by three readers

The question is where an .srb file's metadata and notes streams are. An .srb holds a metadata stream at offset 16, then the notes stream right after it.

All readers share the constants and inflate function in `src/parse/srb.h`. What is written more than once is the step order. `load_songpath_srb` in `src/parse/song.cpp` (around lines 1127-1135) reads the metadata, then inflates stream 2 at stream 1's end. `extract_srb_audio` in `src/app/preview_source.cpp` (around lines 239-244) walks stream 1 and stream 2 again to reach the audio. `parse_srb_metadata` in `src/app/analysis.cpp` (around lines 229-237) composes "inflate at offset 16, then parse metadata" a second time, but stops there. So two copies walk to stream 2, and two compose the metadata read.

They agree today, shown by reading: same constants, same order.

Nothing is wrong on screen today. If one walk were changed and the others not, the Preview could play no audio, or the library could lose names for .srb charts.

`srb.h` should own one function that returns the metadata and the stream offsets. The finder dates this to 21ddf53 (2026-08-18).

*Owner proposed: src/parse/srb.h (one function returning the metadata and stream offsets). Candidates: parse-13.*

#### 189. Scan and Preview each walk the folder to find song.ini

The question is which file is a song folder's song.ini.

Two folder walks answer it. `discover_charts` in `src/app/analysis.cpp` (line 365) keeps the last match. `find_song_ini` in `src/app/preview_source.cpp` (around lines 122-138) runs its own `FindFirstFileW` walk and keeps the first match. Both use `is_song_ini` in `src/parse/chart_files.cpp` for the name test, so only the walk and the first-or-last choice are doubled.

They agree today on normal Windows folders, shown by reading. NTFS is case-insensitive, so a folder holds at most one song.ini. They could only differ in a case-sensitive directory holding both `song.ini` and `Song.ini`; the verifier did not build one.

Nothing is wrong on screen today. In that edge case, the library name and the Preview's audio delay could come from different files.

The scan's walk came with 21ddf53 (2026-08-18); `find_song_ini` came with 1b7eeb3 (2026-09-25). One picker should own the choice, or the Preview should use the ini path the scan already found.

*Owner proposed: is_song_ini plus one folder-entry picker shared with discover_charts (or the Preview reuses the scan's ini path). Candidates: parse-14.*

#### 190. The kick-means-2x, pad-means-cymbal lane rule is respelled in several places with no owner

The question is which drum lane may carry which flag. Only the kick may carry 2x. Only yellow, blue and green may carry a cymbal. Red carries neither.

The cymbal half has an owner. `allows_cymbals` in `src/core/model.cpp` (lines 14-17) is exported from `model.h`. The 2x half has no owner, and the "kick's flag means 2x, a pad's flag means cymbal" split is written again and again inside `model.cpp`. `lane_flag` (line 93) picks `is2x` for the kick and the cymbal flag otherwise. `lane_allows_flag` (line 97) is hidden in an anonymous namespace. It is not a plain wrapper on `allows_cymbals`: it adds "or the kick", so it carries its own copy of the 2x half. The stray-flag check in `Chord::code` (lines 111-114) spells the split a third time. `Chord::from_code` (lines 144-148) spells it a fourth time when it turns an upper-case letter back into 2x or cymbal. `Chord::add_2x` (line 257) hard-wires the kick. The test helper `lane_shapes` in `tests/test_chord_code.cpp` (around line 30) decides the 2x half itself instead of asking production.

The Preview has its own version too. `gem_of` in `src/render/track_state.cpp` (line 44) writes `pro && n.cymbal && lane != Red`. That repeats "red is never a cymbal" as its own lane test. It also re-applies the Pro Drums gate the parser already applied.

| Lane | 2x allowed | Cymbal allowed |
|---|---|---|
| Kick | yes | no |
| Red | no | no |
| Yellow / Blue / Green | no | yes |

Every copy gives this same table today, shown by reading. The inputs are enum values, so there are no edge cases. The parsers never produce a red cymbal, and non-pro parses carry no cymbals. The Preview's pro flag is the same one the song was parsed with, so `gem_of`'s extra checks never change an answer.

Nothing is wrong on screen today. If one copy changed, chord codes would be spelled or parsed wrongly, and the stored chord codes on screen would stop matching the chart. If the lane rule in `allows_cymbals` changed, the Preview would still draw the old way and the chord code would still reject it.

`allows_cymbals` should stay the cymbal owner. One exported lane-flag predicate beside it in `core/model.h` should own the 2x half and the split. `lane_flag`, `code`, `from_code`, `add_2x` and the test would all call it. `gem_of` should take `n.cymbal` as parsed.

The model copies came in with 823ed7c (2026-09-25). `gem_of`'s lane check came with ba0885a (2026-08-22). ADR 0015 records the user-level spelling rule for chord codes, but not where the lane rule lives.

*Owner proposed: one exported lane-flag predicate in src/core/model.h beside allows_cymbals; gem_of takes n.cymbal as parsed. Candidates: parse-9, sweep-tests1-c9.*

#### 191. The Preview works out song length itself instead of asking the store

The question is how long a song is, measured as the time of its last note.

Two places answer it. `song_length_ms` in `src/store/record_store.cpp` (lines 462-465) reads the last timestamp of the parsed song. Its own comment calls it the one definition. `build_preview_scene` in `src/app/preview_view.cpp` (line 267) answers it again as `scene.notes.back().ms`, the last note of the Preview's own note list. Two tests pin the copies instead of the owner. The test "build_preview_scene: an analyzed chart's overlay matches its path" in `tests/test_preview_view.cpp` (line 560) checks the scene's own rule. `tests/test_store.cpp` (line 1838) re-types the store's formula as its expected value. The test at `test_preview_view.cpp` line 268 checks a literal 750 ms, which is a proper fixed answer.

They agree today, shown by reading, and the tests pass. The parser only makes a timestamp for a chord that has notes (`src/parse/song.cpp`, around lines 569 and 968). The Preview copies every note and drops none. So its last note always sits on the song's last timestamp. For an empty song, the store returns no length, and `build_preview_scene` returns early (line 227), leaving the scene's length at its default 0. Both mean "no length".

Nothing on screen changes today. If the store's rule changed, for example to include a trailing section, the Preview would not follow, because the transport takes the scene's number. The Preview scrubber and the Paths timeline would then end at different times.

One comment is loose. `preview_view.h` (line 221) and the test comment call this number "the scrubber's right edge". In fact the transport length is the larger of the last note and the audio end (`preview_transport.cpp` line 17).

`store::song_length_ms` should own the answer, because it already claims to be the one definition. `build_preview_scene` should call it, and the tests should compare against it.

The Preview line came in with ba0885a (2026-08-22). The store function came in later, with bbf9345 (2026-09-27). No user decision covers the second copy.

*Owner proposed: store::song_length_ms in src/store/record_store.cpp, called by build_preview_scene. Candidates: display-38, parse-23, store-7, sweep-tests3-c14.*

#### 192. Chart hash lowercasing written in four places

The question is how a chart hash is spelled for matching. The hash is an md5 fingerprint written as hex text, so letter case matters.

Four places answer it. `stream_md5` in `src/app/analysis.cpp` (around lines 153-158) writes lowercase hex, and it is the only producer of chart hashes. `records_by_hash` in `src/app/report.cpp` (around line 246) lowers hashes again. `collect_dm_rows` in `src/app/dm_report.cpp` (around line 164) lowers them again too. `parse_score` in `src/net/dmbot_client.cpp` (around line 264) lowers the leaderboard's ids, which is needed because they come from outside. The store's SQL joins compare hashes as-is.

They agree today, shown by reading. Lowercasing text that is already lowercase changes nothing.

If a hash ever arrived in another case, dmleaderboards rows would read "not in library" or "not analyzed" for charts that are analyzed.

The report and DMBot lowering came in with 283e028 (2026-09-26). The dm_report lowering came in with f1c6a3d and 4bdada0 (2026-09-27).

One `normalize_chart_hash` next to `stream_md5` should own it. The DMBot boundary would call it, and the internal re-lowering could go.

*Owner proposed: A normalize_chart_hash next to stream_md5 / hash_chart_file in src/app/analysis.cpp, used at the DMBot boundary. Candidates: parse-15.*

#### 193. Error messages become plain text by matching each thrower's exact wording

The question is which plain sentence the user sees when something fails.

`plain_error_text` in `src/app/user_messages.cpp` (from line 81) answers it by matching the raw exception text against copied strings, by exact text or by prefix. `is_no_notes_message` (line 75) copies the shape of the "No ... notes in this chart." message from `parse/song.cpp`. Other matched strings include 'cancelled' (around line 83), '(error 12002)' (around line 87), and 'PreviewRenderer: missing' and '3d-config.json:' (around lines 139-143).

So every matched message lives twice: once where it is thrown, and once in the matcher. The throwers are spread across net, store, parse, audio, search and app. Examples: `fail` in `src/net/dmbot_client.cpp` (around line 38) appends ' (error N)'. 'cancelled' is thrown at several lines in `dmbot_client.cpp` and returned by `JobCancelled::what()` in `src/ui/job_base.h`. `store/serialize.cpp` throws 'truncated blob', `audio/decode.cpp` throws 'decode_audio:', and `record_store.cpp` throws others.

They agree today. A verifier checked 44 matched literals by reading and found each one at a throw site. Nobody checked whether every thrower is covered. Some are not: damaged-asset errors from `src/render/preview_renderer.cpp` and `src/render/obj_loader.cpp` ('undecodable texture', 'shader', 'obj: no faces') have no specific mapping.

If someone rewords a thrown message, nothing breaks at compile time. The user quietly gets the generic fallback instead of the specific remedy.

The Preview branch of the matcher is dead outside tests. I confirmed this by reading. `PreviewController` stores the raw `e.what()`, and `preview_tab.cpp` (line 214) prints "Preview failed: <raw text>". Nothing on that path calls `plain_error`; only `app_state.cpp` and `library_jobs.cpp` do. So a missing or damaged Preview asset shows its raw exception text on screen, never "Reinstall Hydra". Only `tests/test_user_messages.cpp` reaches the Preview-assets sentence.

The throw site should own the cause. It should throw a typed error kind, and one place in `app::plain_error` should map kinds to sentences. That ties the text to the cause instead of to spelling. The Preview tab would then need to show that sentence.

The matcher came in e6d8159 (2026-09-27) from the ui-redesign plan f1c6a3d (task T4). The render throws came in ba0885a (2026-08-22). That plan's audit item 11 asked that raw exception text not reach users. No user decision choosing string matching over typed errors was found.

*Owner proposed: Typed error kinds thrown at the source, mapped to sentences once in app::plain_error (src/app/user_messages.cpp). Candidates: display-29, sweep-media1-c12.*

#### 194. The structure blob's 12-byte header layout is respelled by hand in the record store

The question is where, inside a stored structure blob, the path format number and the rules fingerprint sit. The format takes the first 4 bytes. The fingerprint takes the next 8.

The owner is `src/store/path_codec.cpp`. `flatten_record` writes the format and then the fingerprint (lines 310 and 313). `rebuild_record` reads them back (lines 343 and 347).

`src/store/record_store.cpp` keeps its own copy of that layout. It defines `kStructureHeadBytes = 12` (line 263). In C++, `layout_is_current` (line 277) reads offset 0 at line 279, and `structure_is_current` (line 285) reads offset 4 at line 288. In SQL, `row_ready_sql` (line 349) uses `substr(structure,1,4)` and `substr(structure,5,8)` at lines 352-353. `delete_auto_results` uses `substr(structure,5,8)` three times (lines 1047, 1055, 1056). `get_summaries` (line 1102), `fill_missing_stars` (line 1481) and `list_records` (line 1540) each select `substr(structure,1,12)`. The tests in `tests/test_store.cpp` and `tests/test_path_codec.cpp` poke the bytes by hand too.

They agree today, shown by reading. Only comments tie the copies together.

Nothing is wrong on screen now. But if someone reordered the header in `flatten_record`, everything would still compile. Then every row in the library would read Stale, or rows made under other rules would read Ready.

`path_codec` should own the layout and export the offsets, because it is the code that writes the bytes. The store should build both its C++ reads and its SQL offsets from those constants.

The same review noted three other store choices that also have no recorded decision, all in record_store.cpp. `kHashesPerQuery = 10000` (line 235) is tied by comment to SQLite's variable limit. WAL with `synchronous=NORMAL` (lines 557-566) sets how writes reach disk; the 2026-09-26 plan (line 95) lists WAL under "Calls I made myself (not yours)". `outranks` (lines 370-374) prefers the current stamp, then the current format, then the higher result id; that ordering only matters if a chart has two candidates, which the code says cannot happen since Auto was removed.

The header copy came in 2a5691c (2026-09-24), and df192dc (2026-09-27) touched the same lines. The other choices trace to bc01fa6, 81d7724 and bc1b00e (2026-08-31 to 2026-09-27). No user decision was found in ADR 0014, ADR 0018, the 2a5691c commit message, CONTEXT.md or the plan headers.

*Owner proposed: store/path_codec: export the header offsets (and a read-header helper); record_store builds its C++ reads and SQL substr offsets from them. Candidates: store-24, store-3.*

#### 195. Three separate little-endian byte codecs serve the stored blobs

The question: how is a whole number written into, and read out of, a stored blob byte by byte? Little-endian means the lowest byte comes first.

Three places answer it. `BinaryWriter`/`BinaryReader` in `src/store/serialize.cpp` (lines 9-14 and 48-58) is the declared shared codec; `serialize.h` says it is shared by the record blob and the tempo-map blob. `read_le`/`write_le` in `src/store/record_store.cpp` (265-274) is a second loop, used for the structure header and the format and fingerprint values bound into SQL. `write_u32_le`/`read_u32_le` in `src/app/dynamics_breakdown.cpp` (129-141) is a third, used for the dynamics blob.

They agree today for every value of every width, shown by reading. All three put byte i at `v >> 8*i`.

The structure header is written by `BinaryWriter` inside `flatten_record` but read back by the store's own `read_le`. So a change to one codec's width or byte order would make the store read the wrong bytes. The whole library could then read Stale.

`store/serialize` should own the byte order, because it is already the declared shared codec.

`write_u32_le` came in ec3ade9 (2026-09-21), `read_le` in 2a5691c (2026-09-24), and the current `write_le` in df192dc (2026-09-27).

*Owner proposed: store/serialize (BinaryWriter/BinaryReader or one small helper exported from it). Candidates: store-4.*

#### 196. Why a row is Stale is worked out separately from whether it is Stale

The question: is a stored row Ready, and if not, was the cause the build or the rules?

`rank_row` in `src/store/record_store.cpp` (line 307) decides Ready. It needs the results stamp current, the path format current, and the fingerprint equal to the current rules. `row_ready_sql` (349) spells the same rule in SQL from the same stamp lists; ADR 0018 sanctions that copy. `stale_reasons` (322) combines the same helpers a second time: build = results stale or layout stale; rules = layout current but fingerprint wrong.

They agree today, shown by reading. The verifier worked out that the two reasons together always equal "not Ready". The C++ and SQL checks also agree on short blobs. One odd case remains. A format-6 blob 4 to 11 bytes long would be labelled "rules" though it has no fingerprint at all. No writer produces that; only a corrupt row would.

Only `hydra_replay dump` prints the reasons. If the two compositions drifted, that command would give the wrong cause.

`rank_row` should return the reasons along with the verdict, so both come from one evaluation. The SQL copy stays, per ADR 0018.

`stale_reasons` came in 2a5691c (2026-09-24). User decision 27 (2026-09-24 plan) asks the dump to name the actual reason. No decision covers working the reasons out a second time.

*Owner proposed: rank_row in src/store/record_store.cpp (return the reasons with the verdict). Candidates: store-8.*

#### 197. reload_row checks row identity without the lens

The question: during a report walk, is the row at this result_id still the same result?

`RecordStore::reload_row` in `src/store/record_store.cpp` (line 822) answers it by comparing four columns: hyhash, chartmode, hyversion and sp_cap (833-836). The real identity is wider. `RecordKey::operator==` and `Lens::operator==` in `src/store/record_store.h` (131 and 116) include the five lens fields, the analysis settings a result is filed under. The unique key in `create_result_tables` (217-218) includes them too. So `reload_row` keeps a partial identity check of its own.

They agree today, shown by reading. A wrong match would need a Ready lens-A row deleted and a lens-B row taking its id on the same connection. Within one connection that cannot happen, because purge 2 re-inserts lens A in the same transaction. Writes from another process skip `reload_row` entirely, since it only runs when this connection's change counter moves.

If the copies drifted, the path report could list another lens's paths for one chart.

`RecordKey` should own identity, because it is the one definition the unique key mirrors. `reload_row` could select the lens and compare it.

Introduced in f5320dd (2026-09-03), before the lens was part of the key.

*Owner proposed: RecordKey and the lens columns (reload_row should compare the full key). Candidates: store-14.*

#### 198. hydra_replay's Args struct holds its own copy of every GUI default setting

The question is what analysis settings `hydra_replay` uses when no flag is given: SP cap, Path limit, depth and the drum options.

The app owns the answer in `Settings` in `src/app/config.h` (lines 58-69 and 91). The defaults are Expert, Pro Drums on, 2x Bass on, depth 4 in scores mode, timing limit on at 10 ms, and `kCloneHeroSpCap`.

`struct Args` in `tools/replay.cpp` (lines 67-88) types its own copy as literals. It says cap "4", ms "10", depth_mode "scores", depth 4, prodrums true, bass2x true, difficulty "expert".

`settings_from` (lines 140-160) starts from `app::Settings`, then overwrites every field with the `Args` values, whether a flag was given or not. Its comment says "struct defaults are the GUI defaults", yet it overwrites the real GUI defaults with them. It also falls back to Expert for an unknown `--difficulty` word, which is a third copy of that default.

In the same file, `check_chart` (lines 689-694) does the opposite and reads `app::Settings` directly.

The values match today. This was shown by reading. Nothing shows wrong now.

If an app default changed, hydra_replay would keep the old one. `hydra_replay dump` would then build a different record key than the app. It would print NotAnalyzed and analyze fresh, for a chart the app has already analyzed. Its dump and target output would be priced under different settings from the app.

This came in with 30c4008 (2026-09-03). No ADR, CONTEXT.md entry or commit message decides it.

`app::Settings` should own the defaults, because they are the GUI's settings. `Args` fields should become optional. `settings_from` should change a Settings field only when its flag was given.

*Owner proposed: app::Settings in src/app/config.h; Args holds optionals and settings_from changes only what was passed. Candidates: display-36, screenB-18, sweep-core1-c8.*

#### 199. Rules field names typed twice: fingerprint and INI reader

The question is which rules fields exist and how each is spelled.

The `Rules` struct in `src/core/rules.h` (line 26) owns the field list. Two places type the names by hand. `fixed_cap_text` in `src/core/rules.cpp` (lines 31-42) writes each name into the fingerprint text. A fingerprint is a short code that changes whenever the rules change. `load_rules_file` in `src/app/rules_file.cpp` (lines 57-73) matches the same names as keys in hydra_rules.ini. Both also spell the two `sqout_rule` words, "first_note" and "whole_chord".

They match today. This was shown by reading.

If someone added a field to the reader and forgot the fingerprint, nothing would fail. The test "rules: every key in the file is read" in `tests/test_rules.cpp` (line 83) checks only that the whole fingerprint changes. Old results would then show Ready under different rules.

core/rules should hold one table that both walk, keeping the fingerprint text byte-for-byte. The copies came in with 7e14401 on 2026-09-24. The user decided the key names (2026-09-24 plan, line 40, decisions 13 and 16), not keeping two lists.

*Owner proposed: core/rules: one (name, member) table walked by both the fingerprint and the reader. Candidates: store-18.*

#### 200. Settings INI key names typed twice, in load and save

The question is which keys hydra_settings.ini holds.

`Settings::load_file` in `src/app/config.cpp` (lines 68-95) reads 17 keys. `Settings::save_file` (lines 109-125) writes the same 17 keys, each name typed again.

They match today. This was shown by reading. The test "settings round-trip through an INI file" in `tests/test_config.cpp` (line 34) checks all 17 fields, so a key saved but not loaded would be caught.

The user sees nothing today. A misspelled key would quietly reset that setting to its default on the next launch.

A reader and writer pair always names each key, so the risk is small. A single key table in `Settings` would remove it. The pair came in with 3b69958 on 2026-08-15.

*Owner proposed: app::Settings: one key table used by load and save. Candidates: store-20.*

#### 201. ASCII case and prefix helpers copied outside core/strutil after it became their owner

The question is how text is compared by prefix or suffix, ignoring ASCII case, and which bytes count as whitespace.

The owner is `src/core/strutil.cpp`. It has `lower_ascii`, `to_lower_ascii`, `trim` with its six-byte whitespace set `kSpace`, and `ends_with_ci` with its own case-blind loop. Its header says every caller uses it rather than its own copy. Commit 682e924 (2026-09-26, audit-fixes plan Task 16) gave these one home.

Copies appeared anyway. One day later, b89beed (2026-09-27, library search) added private copies in `src/app/library_query.cpp`. `is_ascii_space` (around line 31) tests the same six bytes as `kSpace`. `ascii_lower` (around line 43) is the same A-Z map as `lower_ascii`. `iequals_ascii` (line 47) and `starts_with_ci` (line 56) are case-blind equality and prefix checks that strutil does not offer. `src/app/user_messages.cpp` has a private `ends_with` (line 64) with the same body as strutil's, plus a `starts_with` (line 61) that strutil lacks. `difficulty_from_name` in `src/parse/song.cpp` (around line 174) and the "color" check in `plain()` in `src/app/report.cpp` (around line 196) compare with `std::tolower` inline.

| Byte | strutil / library_query | `std::tolower` (C locale) |
|---|---|---|
| A-Z | a-z | a-z |
| a, 0, @, [ | unchanged | unchanged |
| 0x80 and up | unchanged | unchanged |

They all agree today, shown by reading. `std::tolower` would differ only under a non-C locale. No `setlocale` call exists in src, tests or tools.

Nothing shows on screen now. A later change to strutil's rules would not reach library search, difficulty names, the report's color-tag stripping or the error-message mapping.

Two look-alikes are not copies. The ini trims in `read_song_ini_keys` strip only space and tab by design, as Task 16 says. The case check in `Chord::from_code` is a chord-code flag, not text comparison.

strutil should own all of these. It needs to export its per-character tests and lowercase, plus `starts_with`, one case-blind equals and `starts_with_ci`. Then the copies can call it.

*Owner proposed: core/strutil (src/core/strutil.cpp), exporting its character tests, starts_with, a case-blind equals and starts_with_ci. Candidates: sweep-core1-c1, sweep-tests4-c1, sweep-tests4-c2.*

#### 202. The report file name is typed twice, once for the GUI and once for the CLI

The question is: what is the report page's file name, and which folder does it go in?

Two places name it. `report_html_path` in `src/app/report_files.cpp` (line 69) uses `L"hydra_paths.html"` inside `reports_dir()`. The hydra_report CLI in `src/cli/report.cpp` (line 42) sets its default `--out` to `"hydra_paths.html"`, resolved against the current folder.

The file name agrees today, by reading. The folder does not. With no `--out`, the CLI writes into the current folder. The GUI writes into Documents\Hydra, or the db folder as a fallback. The `report_files.h` header comment says the CLI takes page locations from that module, but its default does not. Nothing says whether the folder difference is intended, so it is not called wrong here.

Nothing shows wrong today. If the GUI page were renamed, hydra_report would keep writing the old name.

The CLI literal dates from 3b69958 (2026-08-15); the GUI one from 21ddf53 (2026-08-18). No ADR or CONTEXT.md entry covers it. A file-name constant in `src/app/report_files.h` should feed both. Whether the CLI should also use `reports_dir()` is a separate question for the user.

*Owner proposed: a file-name constant in src/app/report_files.h. Candidates: sweep-core1-c11.*

#### 203. Squeeze kind is written by type_name but read back with a typed "SqOut"

The question is: how is a squeeze kind spelled in text?

The owner is `SPSqueeze::type_name` in `src/core/model.h` (around line 196). It returns "SqIn" or "SqOut". `paths_json` in `tools/replay_json.cpp` (around line 81) writes the kind with it. But `windows_from_json` in the same file (line 53) reads it back by comparing with a hand-typed `"SqOut"`. `SPSqueeze::description` in `src/core/model.cpp` (around lines 292-301) also types both names inline instead of calling `type_name`.

| Kind | Written as | Reader does |
|---|---|---|
| SqIn | "SqIn" | skips it |
| SqOut | "SqOut" | takes its offset |
| missing or unknown | n/a | skips it |

Writer and reader agree today, by reading. Nothing shows wrong now. The JSON only feeds developer tools (hydra_replay dump, target, score --path). If `type_name` changed its spelling, the reader would silently skip every squeeze-out and leave `sqout_offset_ms` unset.

The reader literal came in 2045a21 (2026-09-09). No ADR or CONTEXT.md entry covers it. `type_name` should own the spelling, with a parse twin (name to `SqueezeKind`) for the reader. `description` could build on `type_name` too.

*Owner proposed: SPSqueeze::type_name in src/core/model.h, plus a matching parse function. Candidates: sweep-core1-c12.*

#### 204. Library search decides twice whether a search term applies to a column

The question is: does a library search term apply to a given field, such as Title or Artist?

Two functions in `src/app/library_query.cpp` decide it. `term_matches` (around line 281) uses a switch. A Title, Artist, Charter or Folder term tests its own field, and an Any term tests all four. `match_spans` (around line 406), which picks the text to highlight, skips a term unless its field is Any or equals the column.

They agree today for every column the UI passes, by reading. `src/ui/library_table.cpp` only ever passes Title, Artist, Charter or Folder. Nothing shows wrong now.

The risk is one-sided. `term_matches` holds the list of fields Any covers. `match_spans` has no list; it trusts Any to cover every column. If Any stopped covering a field in `term_matches`, the Library would still highlight Any-term hits in that column, even though they no longer made the row match.

Both came in b89beed (2026-09-27). No ADR, CONTEXT.md entry or commit message covers where this rule lives. One `term_applies_to(term_field, field)` helper should own it, used by both functions.

*Owner proposed: one term_applies_to helper in src/app/library_query.cpp. Candidates: sweep-core1-c6.*

#### 205. Ghost plus accent count is summed in two places

The question is: how many dynamic notes (ghosts plus accents) does a count hold?

Two places add them. `DynamicsCounts::has_dynamics` in `src/app/dynamics_breakdown.h` (line 27) checks `ghost + accent > 0`. The Dynamics tab in `src/ui/dynamics_tab.cpp` (around line 203) computes `played.ghost + played.accent` again for the "Dynamic notes: N of M" line. The same tab also calls `has_dynamics` on the same counts at line 106.

They agree today, by reading. Counts are never negative, so `has_dynamics` is true exactly when the tab's sum is above 0. Normal notes count in neither. Nothing shows wrong now. If one side began counting another note type, the Totals line and the has-dynamics check could disagree.

The same tab also writes one whole-percent formula twice, at lines 153-155 and line 205.

This came in ec3ade9 (2026-09-21). The tab code moved in 2facc04 (2026-09-27). No plan, ADR or CONTEXT.md entry covers it. `DynamicsCounts` should get a `dynamic()` accessor. `has_dynamics` and the tab would both call it.

*Owner proposed: DynamicsCounts in src/app/dynamics_breakdown.h. Candidates: sweep-core1-c4.*

#### 206. Three settings-to-text formatters describe the same analysis settings

The question is: how are the analysis settings described in a line of text?

Three places write it. `folder_breakdown` in `tools/bench.cpp` (around lines 55-56) prints "SP cap, score range, ms limit". It checks neither whether the timing limit is on nor which depth mode is set. The batch header in `src/cli/batch.cpp` (around lines 188-205) names the depth unit with its own switch and prints "Timing cap : none" when the limit is off. `within_label` in `src/app/path_view.cpp` (around lines 436-442) picks "score(s)" or "point(s)" for the depth.

| Settings | bench | batch |
|---|---|---|
| GUI default | "score range 4, 10ms limit" | "Depth : scores 4", "Timing cap : 10 ms" |
| Limit off | (cannot happen in bench) | "Timing cap : none" |

Nothing prints anything wrong today, by reading. Bench always builds default Settings and never loads the ini. So it always sees the limit on and depth by scores. The same facts appear in different words.

The visible risk is in developer-tool headers only. If bench ever ran non-default settings, its header would still say "score range" and show a limit that might be off. One description helper next to `app::Settings`, reusing `within_label` for the depth part, should serve both headers.

*Owner proposed: one settings-description helper next to app::Settings (reusing within_label for depth). Candidates: sweep-core1-c10.*

#### 207. Two 2-second file re-check constants and their caching code, typed twice with no recorded decision

The question is how long the UI trusts a cached "this file exists" answer before asking the disk again.

Two constants answer it, both 2.0 seconds and both in `src/ui/app_state.h`. `kFileCheckSeconds` (line 231) serves `AppState::selected_file_ok`, for the selected chart file. `kReportCheckSeconds` (line 242) serves `AppState::report_file_shown`, for the path report file. The caching code around each one is written twice, line for line, in `src/ui/app_state.cpp` (the checks are at lines 193 and 352). A stamp of -1 means "look now", and an elapsed time of 2 seconds or more means "look again". Both comments give the same reason: avoid a disk call every frame.

| Seconds since last check | Both functions |
|---|---|
| never checked | look |
| 1.0 | use cache |
| 2.0 | look |
| 2.5 | look |

They agree today on every input, including the exact 2.0 edge, shown by reading. Nothing on screen differs. If one interval changed alone, the Analyze button's "file missing" state and the report's "file shown" state would refresh at different speeds after a file is deleted. The tests in `tests/test_app_state.cpp` pin both with literal timestamps rather than the constants, so they would not notice either way.

The two values came in two separate tasks on 2026-09-26: `kFileCheckSeconds` in cb4f28a and `kReportCheckSeconds` in ddde90c. `docs/superpowers/plans/2026-09-26-codebase-audit-fixes.md` says "every two seconds" in two agent-written task bodies, outside its list of approved changes. So the user did not ask for the 2-second figure, as far as was found. Nothing in CONTEXT.md or docs/adr/ says the two must match or may differ.

`app_state` should own this. One helper that caches a file-exists check for a fixed interval, used by both functions, would remove the twin caching code. The smallest fix is one shared constant. Merging them is a design call, so it should follow only if the user wants one policy.

*Owner proposed: src/ui/app_state.h: one cached file-exists helper (or one shared constant), if the user wants one policy. Candidates: sweep-tests1-c10, sweep-ui1-c13.*

#### 208. UI font size 18 is typed three times in setup_imgui

The question is what pixel size the UI fonts load at.

`setup_imgui` in `src/ui/app_shell.cpp` types 18.0f three times. The main font loads at 18 (around line 241). The mono font loads at 18 (around line 243). The CJK fallback, which adds Japanese and Chinese glyphs to the main font, merges at 18 (around line 261).

The main and CJK values are tied together. ImGui scales merged glyphs by the merge size divided by the main font's size (`imgui_draw.cpp` around line 4746). Today that is 18/18, so 1.0. If the main font moved to 20 and CJK stayed at 18, Japanese titles would draw at 0.9 of the Latin text size. The mono value is the same choice typed a third time. Its load size has no visible effect today, because every `PushFont` of it passes either the current size or an explicit 24.

All three agree today, shown by reading. Nothing on screen differs. The three lines came in bd9cb28 (2026-08-18). No user decision on the font size was found in CONTEXT.md or `docs/adr/`.

One `kFontSize` constant in `app_shell.cpp` should own it. The main load and the CJK merge must use it, and the mono load can too.

*Owner proposed: one kFontSize constant in src/ui/app_shell.cpp. Candidates: sweep-ui1-c15.*

#### 209. Resource folder path is built twice; only the font loader takes an override

The question is where the bundled resource folder lives. It sits next to the exe and holds the fonts and icons.

Two functions build the path. `setup_imgui` in `src/ui/app_shell.cpp` (around line 238) uses `options.resource_dir` when set, else `exe_dir()` plus `\resource`. `load_icons` in `src/ui/icons.cpp` (around line 70) always uses `exe_dir()` plus `\resource\` and has no override.

| resource_dir | setup_imgui | load_icons |
|---|---|---|
| empty (the shipped app) | exe_dir\resource\ | exe_dir\resource\ |
| set (uitest harness, test_app_shell) | the override | exe_dir\resource\ |

They agree today, shown by reading. The two places that set the override, `tests/ui/uitest_harness.cpp` and `tests/test_app_shell.cpp`, never call `load_icons`. So nothing on screen differs. If a test or a future layout loaded icons with the override set, fonts and icons would come from different folders.

The override came in ba0885a (2026-08-22). Before that, `main.cpp` built the same literal that `icons.cpp` still uses, so the path was already written twice. No user decision was found in CONTEXT.md or `docs/adr/`.

One resource-folder helper beside `app::exe_dir()` should own it. `setup_imgui` keeps its override on top, and `load_icons` calls the helper.

*Owner proposed: one resource-folder helper next to app::exe_dir(). Candidates: sweep-ui1-c9.*

#### 210. DPI-to-scale is computed by Hydra and by the vendored ImGui backend

The question is what UI scale a monitor's DPI gives. DPI is dots per inch, and 96 is the Windows baseline of 100%.

Two pieces of code answer it. Hydra's `ui_scale_for_dpi` in `src/ui/app_shell.cpp` (around line 163) returns DPI / 96, or 1.0 when DPI is 0. The vendored backend's `ImGui_ImplWin32_GetDpiScaleForMonitor` in `third_party/imgui/backends/imgui_impl_win32.cpp` (around line 1088) also returns DPI / 96. `src/ui/main.cpp` uses the backend at startup (around lines 106 and 173) and Hydra's own function on a monitor change in `WndProc` (around line 424).

| DPI | ui_scale_for_dpi | backend |
|---|---|---|
| 96 / 120 / 144 / 192 | 1.0 / 1.25 / 1.5 / 2.0 | same |
| 0 | 1.0 | 0.0 |

They agree today for every positive DPI, shown by reading. The DPI 0 edge needs Windows to report 0, which was not observed. So the UI scale at startup and after moving to another monitor come out the same. If they drifted, the window would change size the first time it crossed monitors.

The monitor-change path came in 69eeb3c (2026-09-27). The startup call is older, from 3b69958 (2026-08-15). No user decision was found in CONTEXT.md or `docs/adr/`.

`ui_scale_for_dpi` should own it. `main.cpp` would read the raw DPI and pass it through. The vendored backend can stay untouched.

*Owner proposed: ui_scale_for_dpi in src/ui/app_shell.cpp. Candidates: sweep-ui1-c10.*

#### 211. Library split share range is checked twice in app_shell.cpp

The question is whether a library split share is legal. The share is the fraction of the window width the library takes, and it must lie strictly between 0 and 1.

Two functions in `src/ui/app_shell.cpp` check it with the same test. `parse_layout_line` (around line 142) rejects a bad `LibraryShare` loaded from `hydra_ui.ini`. `remember_library_share` (around line 149) rejects a bad share from a drag. The pixel clamp in `src/ui/library_view.cpp` (around line 62) is a different rule, minimum and maximum widths, not a copy.

They agree today on every input, shown by reading. Both reject 0, 1, NaN and anything outside the range, and both accept anything in between. Nothing on screen differs. If one copy changed alone, a split you drag could be saved and then refused on the next start, so the split would jump back to its default.

Both lines came in 39d61cf (2026-09-27). No user decision on the range was found in CONTEXT.md or `docs/adr/`.

One `share_is_valid` helper in `app_shell.cpp` should own the range, called by both functions.

*Owner proposed: one share_is_valid helper in src/ui/app_shell.cpp. Candidates: sweep-ui1-c8.*

#### 212. AnalyzeJob::start hand-copies the job base class's failure handling

The question is what a failed job records: the raw error text, the plain message shown to the user, ok = false and finished = true.

`ResultJobBase::run_guarded` in `src/ui/job_base.h` (around line 79) is the owner. Its catch sets all four, and skips the plain message when the job was cancelled. `AnalyzeJob::start` in `src/ui/library_jobs.cpp` (around line 326) repeats the same four stores when the worker thread fails to start, but without the cancel check. The finder also named `BatchJob::run`'s load-failure branch. The verifier ruled that out: `BatchJob` is not a `ResultJobBase`, and it writes a different surface, the batch failure list.

They agree today, shown by reading. The only split is a cancel that lands before the thread starts. That cannot happen, because `AppState::start_analyze` calls `start()` right after building the job. So the Analyze panel shows the same error lines either way. If the base class later changed what it records on failure, a failed thread spawn would show the old wording.

The copy came in e6d8159 (2026-09-27). No user decision was found in CONTEXT.md or `docs/adr/`.

`ResultJobBase` should own it with a protected `fail(e)` helper that both catches call.

*Owner proposed: ResultJobBase in src/ui/job_base.h (a protected fail helper). Candidates: sweep-ui1-c6.*

#### 213. Button width formula re-typed in three places beside its helper

The question is how wide a button will be for a given label. The rule is the label's text width (without the hidden "##" part) plus frame padding on both sides.

`button_slot_width` in `src/ui/widgets.h` (around line 101) owns it. Three places re-type it. `button_width` in `src/ui/paths_tab.cpp` (around line 88) is the same formula in its own function. `render_chips` in `src/ui/library_table.cpp` (around line 173) inlines it to measure each chip. `render_search_box` in the same file (around line 88) measures the literal "X" for its clear button, which gives the same width as hiding "##clearsearch".

The finder also named the six-digit input in `settings_bar.cpp` and the combo chrome in `preview_tab.cpp`. The verifier showed those answer different questions (an input with step buttons, a combo's arrow square). They share only the padding term, so they are not copies.

All three true copies agree today, shown by reading. If button sizing or padding changed, chip wrapping and right-aligned buttons on the Paths tab would misjudge widths until each copy was fixed.

`button_slot_width` should own this. The Paths tab helper becomes a call to it or is deleted, and the chips and clear button call it too.

*Owner proposed: button_slot_width in src/ui/widgets.h. Candidates: sweep-ui2-c3.*

#### 214. Does-the-next-item-fit test written five times

The question is whether the next item still fits on the current line, or must wrap.

It is answered at least five times. `fits_on_line` in `src/ui/paths_tab.cpp` (around line 70) compares against the window's work-rect right edge. `render_chips` in `src/ui/library_table.cpp` (around line 174) and `src/ui/settings_bar.cpp` (around lines 199 and 234) use "cursor x plus available width" taken once. `key_hints` in `src/ui/preview_tab.cpp` (around line 68) does the same rule in its own local coordinates.

The finder claimed drift: two different right edges. The verifier refuted that by reading ImGui's source. Inside tables the two edges are identical, and an indent moves neither. They could only differ in a horizontally scrolling window or one with an explicit content size, and `src/ui` has neither. So they agree today, and no runtime check was done.

Nothing differs on screen today. If one copy changed its edge or spacing, chips, setting blocks and Paths tab rows would wrap at different points for the same width.

One helper in `widgets.h` should own it. Move `fits_on_line` there and have the chips and settings bar call it; `key_hints` can use the same predicate on its local x.

*Owner proposed: one fits-on-line helper in src/ui/widgets.h. Candidates: sweep-ui2-c4.*

#### 215. Preview 5-second jump typed as bare 5000 four times

The question is how far a Preview jump moves, and what the buttons and key bar call it.

All answers sit in `render_preview_panel` in `src/ui/preview_tab.cpp`. The 5-second jump is the bare literal 5000.0 in four places: the -5s and +5s buttons (around lines 258 and 271) and the Left and Right keys (around lines 328 and 330). The "5 seconds" wording is typed again in two tooltips and the `kKeyHints` table (around line 39). The tick step does have a constant, `kTickStep = 5` (around line 248). But its four button labels, two tooltips and the key bar's "5 ticks" spell 5 by hand.

Every copy agrees today: 5000 ms, "5 s", and 5 ticks. The verifier found this by reading.

If someone changed the jump or `kTickStep`, the buttons, tooltips and key bar would still say 5 while the Preview moved a different amount.

The 5-second value is a user decision. `docs/superpowers/plans/2026-09-25-preview-score-and-stepping.md` (line 48) records the chat design accepted with 5 s jumps. That fixes the value, not writing it in seven places. The 5-tick step came from 0be22a1, which quotes no user. The jumps came from 3471d8a and the key bar from f4ade35.

A `kJumpMs` constant beside `kTickStep` should own the number. Labels, tooltips and `kKeyHints` should be built from both constants.

*Owner proposed: a kJumpMs constant beside kTickStep in src/ui/preview_tab.cpp. Candidates: sweep-ui2-c7.*

#### 216. Has-an-SP-meter-curve test asked twice, then ANDed

The question is whether the Preview scene has an SP meter curve, so the gauge and the drain box should draw.

Two places ask it. `PreviewController::sp_meter_has_curve` in `src/ui/preview_controller.cpp` (around line 318) checks that the meter's segments are not empty. `build_drain_box` in `src/app/preview_view.cpp` (around line 516) asks again, and also requires timing. Then `render_preview_panel` in `src/ui/preview_tab.cpp` (around line 378) ANDs the two, so the first half adds nothing.

| Timing set | Segments | has_curve | drain shown | drain drawn |
|---|---|---|---|---|
| no | empty | false | false | false |
| yes | empty | false | false | false |
| yes | non-empty | true | true | true |
| no | non-empty | true | false | false (unreachable) |

They agree today. `build_preview_scene` always sets timing before it builds the meter, so the last row never happens. The verifier found this by reading.

Nothing differs on screen today. If the scene could ever hold segments without timing, the gauge would draw while the drain box stayed hidden.

Introduced by aab5ff1 (2026-08-31), 893e107 and 073dfa0 (both 2026-09-27). No user decision was found.

One predicate on the scene in `src/app/preview_view.h` should own it. Both the controller and `build_drain_box` should call it.

*Owner proposed: one predicate on PreviewScene in src/app/preview_view.h. Candidates: sweep-ui2-c10.*

#### 217. ShellExecute success test typed in two files

The question is whether a Windows ShellExecuteW call succeeded. Windows reports success as a return value above 32.

Two places answer it, and they are the only ShellExecuteW calls in `src`. `show_in_folder` in `src/ui/win32_dialogs.cpp` (around line 72) and `open_in_browser` in `src/app/report_files.cpp` (around line 66) both write `reinterpret_cast<INT_PTR>(result) > 32`.

They agree today for every value: 32 and below means failure in both, 33 and above means success in both. The verifier found this by reading.

If one drifted, "Show in folder" or "Open report" could report success when Windows failed, or a failure when it worked.

`report_files.cpp` got it in 21ddf53 (2026-08-18). `win32_dialogs.cpp` got it in a591bba (2026-09-27), copied from plan text. No user decision was found in `docs/adr/` or CONTEXT.md.

One `shell_execute_ok` helper, next to the wide-string helpers in `core/winstr`, should own the rule. Both callers should use it.

*Owner proposed: one shell_execute_ok helper next to core/winstr. Candidates: sweep-ui2-c11.*

#### 218. Dimmed grey and hover teal kept as parallel values

There are two questions here: which grey is dimmed or disabled text, and which teal is the hover colour for frames and headers.

The grey is written twice in `src/ui/theme.h`. `kDimTextColor` (around line 49) and `kDisabledInputTextColor` (around line 65) are both (160,160,160). A disabled label already uses `kDimTextColor`, so the two constants answer the same question. The teal is written twice in `apply_theme` in `src/ui/theme.cpp`. `FrameBgHovered` (around line 36) and `HeaderHovered` (around line 38) each spell out (0,100,100) as a literal.

All copies agree today, shown by reading. There is also a third teal, `kButtonHoveredColor` (0,104,104), in `theme.h` (around line 35). The verifier treats it as a separate choice for white-text contrast on buttons, not proof of drift. Whether frame and header hover should match it is not established.

If one copy changed, disabled inputs and dimmed labels would show different greys, or frame and header hover would differ.

The greys came from ae2a4a6 (2026-09-27), following an agent-written plan. The teal literals came from 3b69958 (2026-08-15). No user decision was found.

`theme.h` should own both. Alias `kDisabledInputTextColor` to `kDimTextColor`, and add one hover-teal constant for both slots.

*Owner proposed: src/ui/theme.h. Candidates: sweep-ui2-c12.*

#### 219. Status-line fade is a bare, unchosen 6.0 seconds that the GUI test mirrors as 400 frames

The question is how long a neutral news line stays in the status bar before it fades. Problems never fade; they stay until replaced.

One production place answers it. `render_status_line` in `src/ui/library_toolbar.cpp` (line 27) hides news once more than 6.0 seconds have passed. The 6.0 is a bare literal with no name. The other timer in the UI, the copied badge in `paths_tab.cpp`, is a different fact.

The GUI test `test_status_line` in `tests/ui/uitest_batch_reports.cpp` (line 405) waits 400 frames at 1/60 s and expects the line gone. Its comment says "over 6 s". One finding called that pinning the behaviour; the other called it a copy. Both are partly right: it is a bound that depends on knowing the number, not the same number.

| Seconds since shown | App | Test expects |
|---|---|---|
| 6.0 or less | visible | nothing checked |
| over 6.0 | hidden | nothing checked |
| about 6.67 (400 frames) | hidden | hidden |

They agree today, and the status-line test passes. If the fade were raised past about 6.67 seconds, the test would fail. If it were shortened, the test would still pass and stop checking anything close to the real value.

The user sees a 6-second fade now. `docs/UserGuide.md` only says a notice "fades by itself".

No user decision fixes 6 seconds. The 6.0 came in 21ddf53 (2026-08-18), a bulk commit of earlier untracked files. It moved to `library_toolbar.cpp` in 92e27d3 (2026-09-27), and the exemption for problems came with the UI redesign. The test came in 842fd76 (2026-09-27). The verifiers checked `git log -S`, `docs/adr/`, CONTEXT.md and the ui-redesign plan, which mentions "fades after 6 s" only as existing behaviour.

A named constant beside `render_status_line` should own the time. The test should turn that constant into frames.

*Owner proposed: A named constant (e.g. kStatusFadeSeconds) beside render_status_line in src/ui/library_toolbar.cpp, with the test deriving its wait from it. Candidates: sweep-tests4-c9, sweep-ui2-c16.*

#### 220. Preview's Onyx numbers kept in both 3d-config.json and the PreviewConfig header defaults, plus test copies

The question is what the Preview's camera, highway, colour and timing numbers are. Onyx is the drum previewer Hydra's Preview is ported from, and these are its numbers.

Two production places hold every value. The shipped `assets/preview/3d-config.json` is what the renderer loads (`src/render/preview_renderer.cpp` lines 289-292). The member defaults of `PreviewConfig` in `src/render/preview_config.h` (roughly lines 25-82) are a full second copy as C++ initializers. `load_preview_config` keeps the struct's default for any key the json lacks (preview_config.h line 87).

Tests add more copies. `check_is_onyx` in `tests/test_preview_config.cpp` (around lines 37-75) types every number a third time as literals, and pins both the defaults and the shipped file to them. `is_background` in `tests/test_preview_renderer.cpp` types `#1c1d2b` a fourth time. `tests/test_overlay_layout.cpp` and `tests/test_highway_draw.cpp` build a default `PreviewConfig`, so they check the header numbers, not the shipped ones.

All copies agree today. One verifier compared every value, from background `#1c1d2b` to `fill_offered_alpha` 0.35. The other ran the five preview-config tests and they pass.

The two finders disagreed on whether the struct copy can reach the screen. The code says it does not, today. `PreviewController::preview_config` in `src/ui/preview_controller.cpp` (lines 306-308) returns a default-built `kOnyxDefaults` when no renderer exists. But its only caller, in `src/ui/preview_tab.cpp` (line 355), runs only after `pc->render` returned a texture (lines 342-343), which needs the renderer that loaded the json. So the overlay always gets the json's numbers.

The real risks are elsewhere. ADR 0008 says to change numbers in 3d-config.json. If someone did that, the 'shipped 3d-config.json loads to the Onyx values' test would fail, so the change cannot drift silently. But the overlay-layout and highway-draw tests would keep checking the old numbers. And a key removed from the json would silently take the old header value.

The json should own the numbers. `docs/adr/0008-preview-is-a-faithful-port-of-onyx.md` says 'Keep the assets verbatim and the numbers in 3d-config.json'. Only a header comment (`src/ui/preview_controller.h` lines 187-189) justifies the struct copy, and a code comment is not a decision. The header defaults came in ba0885a (2026-08-22), `fill_offered_alpha` in 133b52e (2026-08-28), and `kOnyxDefaults` in 9946927 (2026-09-24). No decision approves a second copy.

*Owner proposed: assets/preview/3d-config.json (ADR 0008); PreviewConfig should not restate the values, and tests that need real values should load the json. Candidates: sweep-media1-c6, sweep-tests2-c2.*

#### 221. Track rectangle size floor, aspect and bottom edge worked out in several places

The question: where does the highway's rectangle sit in the Preview image, and at what size?

The track height has one home, `track_height` in `src/render/highway_draw.cpp` (around line 63). Three related rules are written more than once.

The size floor ('below 1 counts as 1') appears four times: in `track_height`, in `project_to_image` and `bottom_left_room` in `src/render/overlay_layout.cpp` (around lines 18 and 68), and in `PreviewRenderer::resize` in `src/render/preview_renderer.cpp` (around line 385). `project_to_image` even clamps before calling `track_height`, which clamps again.

The camera aspect (width over track height) and the bottom anchor (track top at row h minus track height) appear twice. `project_to_image` has one copy (around lines 21 and 27). `PreviewRenderer::render` has the other: the camera at line 407 and the fade rectangle at line 458, in screen coordinates from -1 to 1.

| Image | Aspect (both) | Track top (both) |
|---|---|---|
| 1920x1080 | 1.778 | row 0 |
| 600x1080 | 0.857 | row 380 |
| width 0 | clamped to 1 | clamped to 1 |

All copies agree today, checked by reading. The clamps never fire in the app, because `PreviewController::render` and `preview_tab.cpp` already skip sizes of 0 or less. If one copy changed, the text boxes would be fitted against a highway that is not drawn where the fit assumes.

Origins: ba0885a (2026-08-22, renderer), 76c5530 (2026-09-27, `project_to_image`), 085555c (2026-09-27, `bottom_left_room`). No user decision was found. One `track_rect` helper beside `track_height` should return all of it.

*Owner proposed: a track_rect(cfg, w, h) helper next to render::track_height in src/render/highway_draw. Candidates: sweep-media1-c5, sweep-media1-c17.*

#### 222. '100 means base speed' typed four times, and the status line ignores it

The question: which leaderboard scores count as played at normal speed?

The number 100 answers it as a bare literal in four places. `DmScore::speed` defaults to 100 in `src/net/dmbot_client.h` (around line 53). `DmReportRow::speed` does the same in `src/app/dm_report.h` (around line 36). `parse_score` in `src/net/dmbot_client.cpp` (around line 271) reads a missing speed as 100. `collect_dm_rows` in `src/app/dm_report.cpp` (around line 202) shows a percent only when speed equals 100. The page's own column text in `dm_report.cpp` (around lines 80 and 83) repeats '100% is normal speed'.

They agree today. No single predicate owns them.

The status and delta lines in `collect_dm_rows` (around lines 201 and 204) never ask about speed. So a 150% score above optimal gets no percent but is still marked 'above optimal'. That is not drift between copies. It is an open question: nothing in the repo says whether playback speed changes a Clone Hero score.

No wrong number is proven on screen today. If speed does change score, off-speed runs could add to the 'above optimal' count on the dmleaderboards page while their percent is hidden.

Both literals trace to 21ddf53 (2026-08-18). No user decision was found in CONTEXT.md, docs/adr/, docs/*.md or README.md. One base-speed constant in `dmbot_client.h` should own the literal. Whether the status should use it too is a question for the user.

*Owner proposed: one base-speed constant or predicate in src/net/dmbot_client.h. Candidates: sweep-media1-c10.*

#### 223. Toggle 'on after this instant' rule written twice in the renderer

The question: given an instant's Toggle value, is the span on just after it? (Toggle is a five-value enum: Empty, Start, End, Restart, On.)

Two places answer it. `TrackState::make_toggle_bounds` in `src/render/track_state.cpp` (around line 208) lists the on values. `toggle_on` in `src/render/highway_draw.cpp` (around line 146) lists the off values instead. It decides whether a gem draws in the SP-phrase texture (around line 334).

| Toggle | make_toggle_bounds | toggle_on |
|---|---|---|
| Empty | off | off |
| Start | on | on |
| End | off | off |
| Restart | on | on |
| On | on | on |

They agree today on all five values, read by hand. Nothing differs on screen. A sixth Toggle value would split them silently: one list would include it by default and the other would not. Gems could then draw in the phrase texture where the highway span is off, or the reverse.

Both lines came in ba0885a (2026-08-22). No user decision was found; ADR 0008 covers the Onyx port in general. One helper beside the Toggle enum in `src/render/track_state.h` should own the rule.

*Owner proposed: one helper next to the Toggle enum in src/render/track_state.h. Candidates: sweep-media1-c3.*

#### 224. Highway far-end time typed three times in highway_draw.cpp

The question: what song time sits at the far end of the highway?

The answer is now plus speed times `secs_future`. It is typed three times in `src/render/highway_draw.cpp`: in `time_to_z` (around line 71), in `z_to_time` (around line 77), and in `build_highway_draws` (around line 207). `build_highway_draws` gets the near edge by calling `z_to_time`, but types the far edge inline, even though `z_to_time` at `z_future` gives the same value.

The three are identical today, by reading. Nothing differs on screen.

If one changed alone, for example how speed scales the window, the visible window edge would fall out of step with where gems are placed. Gems could pop in early or late at the far end.

All three came in ba0885a (2026-08-22). No user decision was found; ADR 0008 says the highway ports Onyx's timeToZ maths but not where the expression lives. One `far_time` helper in `highway_draw.cpp` should own it, called by all three.

*Owner proposed: a far_time(cfg, now_s, speed) helper in src/render/highway_draw.cpp. Candidates: sweep-media1-c4.*

#### 225. Lowest output gain clamp written in both the playhead and the transport

The question: what is the lowest legal output gain?

Two places answer it with the same line. `Playhead::set_gain` in `src/audio/player.h` (around line 45) clamps anything below 0 to 0. `PreviewTransport::set_gain` in `src/ui/preview_transport.cpp` (around line 89) types the same clamp, keeps its own clamped copy, then passes it to the playhead, which clamps again. The transport also re-applies its stored copy to a newly loaded playhead (around line 21).

| Gain in | Both copies give |
|---|---|
| -1 | 0 |
| 0 | 0 |
| 0.5 | 0.5 |
| 1.0 | 1.0 |
| NaN | NaN |

They agree today, read from both lines. Nothing differs on screen. If one floor changed alone, the transport's remembered gain and the playhead's real gain could differ after a reload.

The playhead copy came in ba0885a (2026-08-22). The transport copy came in 1da4638 (2026-08-22). No user decision was found. `Playhead::set_gain` should own the clamp; the transport would store the raw value or read the clamped one back.

*Owner proposed: Playhead::set_gain in src/audio/player.h. Candidates: sweep-media1-c8.*

#### 226. Polygon-to-triangle order written twice in obj_loader.cpp

The question: in what order is a polygon split into triangles?

The rule is a fan from the first corner. (A fan joins corner 0 to each pair of later neighbours.) `load_obj` in `src/render/obj_loader.cpp` (around lines 111-118) loops over the corners to build it. `push_quad` in the same file (around lines 159-168) spells out the two triangles for a quad by hand, and its comment says 'matching load_obj's order'. `push_quad` builds the flat quad and box meshes that `preview_renderer.cpp` uploads.

| Shape | load_obj | push_quad |
|---|---|---|
| quad 0,1,2,3 | (0,1,2), (0,2,3) | (a,b,c), (a,c,d) |
| triangle | (0,1,2) | not handled |
| 5+ corners | fan | not handled |

They agree today for quads, by reading. Nothing differs on screen. If the winding in one changed, the hand-built meshes could face the other way and vanish under back-face culling.

Both came in ba0885a (2026-08-22). No user decision was found; ADR 0008 asks for a faithful Onyx port but does not mention hand-built meshes. One fan helper that both functions call should own it.

*Owner proposed: one fan-triangulation helper in src/render/obj_loader.cpp. Candidates: sweep-media1-c15.*

#### 227. Opus decode rate 48000 typed twice and its largest packet worked out by hand

The question: at what rate does Opus decode, and how big is its largest packet?

`decode_ogg_opus` in `src/audio/decode.cpp` answers it with three literals. It sets the output's `sample_rate` to 48000 (around line 141). It creates the decoder at 48000 (around line 184). It sizes the packet buffer as `kMaxFrame = 5760` (around line 157), with the comment '120 ms at 48 kHz'. That number is worked out by hand, not derived from the rate. The pre-skip handling (around lines 178-179 and 199-204) also assumes the decoder runs at 48 kHz.

They agree today: 5760 is 0.120 times 48000. Nothing differs on screen.

If one 48000 changed alone, the audio would be labelled at the wrong rate and play at the wrong pitch and speed. Or the buffer would be too small for the largest packet.

The 48000 literals and 5760 came in ba0885a (2026-08-22); the named `kMaxFrame` took its form in 283e028 (2026-09-26). ADR 0006 decides that Opus decodes at a fixed 48 kHz, so the value has a user decision; only its spelling is duplicated. The 48000 in `src/ui/preview_load_job.cpp` is the mixer's output rate, a different fact. One `kOpusRate` constant should own it, with `kMaxFrame` derived from it.

*Owner proposed: one kOpusRate constant in src/audio/decode.cpp, with kMaxFrame derived from it. Candidates: sweep-media1-c11.*

#### 228. The probe's constants.py holds the 85 ms normal back window twice, as 0.085 s and 85.0 ms

The question is how wide Clone Hero's normal back hit window is, meaning the time allowed after a note. It is 85 ms.

`tools/ch_probe/constants.py` holds that one fact twice, as two separate literals. `EXPECT_NORMAL_BACK_S = 0.085` (line 146) is what `check_normal_constants` in `process.py` and `milestone1.py` compare the live game against. `EXPECT_NORMAL_BACK_MS = 85.0` (line 150) is the clamp cap that `run_passive_probe` in `passive_probe.py` (line 165) and `tests/test_analysis.py` use. Neither is computed from the other.

The tests in `test_process.py` (lines 141-190) use 0.085 and 0.0375 as pinned fake game memory. They would fail loudly on a change, so they are not a drift risk.

The members disagreed on whether `kDefaultHitWindowMs = 85.0` in `src/core/model.h` (line 69) is a third copy. It holds the same number, but it answers a different question. CONTEXT.md (line 125) defines Hit window as a per-side ms user setting, and this constant is only its default. The hit-window plan header says Hydra keeps the fixed 85 ms until step 5. It is also C++, so it cannot import the Python value. So it is related, not a copy; a cross-reference comment is the most it needs.

The two probe constants agree today, checked by reading: 0.085 × 1000 equals 85.0 exactly in Python.

If someone corrected only the seconds value, the build-drift check and the clamp cap would use different back windows. The probe would pass its constant check against the game but judge clamps against the stale 85. Nothing would show in the Hydra GUI.

The seconds constant should own it, because the game stores seconds. The ms value should be `EXPECT_NORMAL_BACK_S * 1000`. The ms copy came in with 396082a (2026-09-21) and the seconds copy with 283e028 (2026-09-26). The C++ default dates from ba0885a (2026-08-22). No user decision on keeping both units was found.

*Owner proposed: tools/ch_probe/constants.py EXPECT_NORMAL_BACK_S, with EXPECT_NORMAL_BACK_MS = EXPECT_NORMAL_BACK_S * 1000. Candidates: sweep-probe1-c13, sweep-probe2-c10.*

#### 229. Old diagnostics rebuild the engine search pattern and module span inline

The question is: how does a probe scan find engine objects on the heap? Two facts answer it. One is the byte pattern to search for. The other is the address range to skip, because GameAssembly's own copies of those bytes sit inside it.

`tools/ch_probe/engine_finder.py` owns both. `normal_pattern` (line 121) reads the normal-mode back/front pair, and `all_patterns` adds the precision pair. `MODULE_SPAN = 0x4000000` (line 27) marks the range to skip. Four older scripts rebuild both inline: `find_all_engines` in `find_clock3.py`, `first_engine_with_window` in `hit_detect.py`, and `main` in `poll_windows.py` and `find_engine.py`. Each reads the same two constants and repeats `0x4000000` with the same half-open range test. `play_chart.py` calls the owner, but passes only the normal pattern where the newer runners pass `all_patterns`.

They agree today in normal mode. The verifier checked by reading. In precision mode these scripts might find nothing. That is unproven, because the repo says the precision constants are unconfirmed live.

If `MODULE_SPAN` or the pattern changes, the four scripts won't follow. They could print "no engine" or wait forever. The Hydra GUI is untouched.

The owner should be `engine_finder`, ideally through one heap-hits helper. The literals came in 283e028. 288b07a named `MODULE_SPAN` but left the copies. The same plan note as the offsets applies: three of these scripts await a user yes to delete.

*Owner proposed: engine_finder.normal_pattern / all_patterns and engine_finder.MODULE_SPAN. Candidates: sweep-probe1-c4, sweep-probe1-c5.*

#### 230. Engine bytes, fields and the precision bit are decoded by hand outside process.py and EngineModel

Three linked questions in the Clone Hero probe tools are answered more than once. How do raw little-endian bytes become a double, a 32-bit or a 64-bit number? Where is each engine field and what type is it? Is the precision-mode bit set? A smaller fourth one rides along: how do seconds become milliseconds?

`tools/ch_probe/process.py` owns the byte decoding with `decode_double`, `decode_u32` and `decode_u64` (lines 38-53). `EngineModel` in `tools/ch_probe/engine.py` owns the field offsets and types (readers at lines 102-144). Its `precision_mode` (defined at line 130) returns `(flags & C.PRECISION_MODE_BIT) != 0` at line 138. `engine.py` claims to be the only module that knows the engine layout.

Copies sit around both owners. `decode_snapshot` in `experiments/live.py` (lines 46-60) reads one block and unpacks window, clock, score, hit time and flags with its own `struct` format codes, re-reading five engine fields. `Snapshot.precision` in the same file (line 43) repeats the precision test as `bool(self.flags & C.PRECISION_MODE_BIT)`. `experiments/find_engine.py` (lines 95-96) writes the test a third time, after reading the flags at the raw offset 0x198 instead of `C.OFF_FLAGS` (same value today). `decode_xmm0_double` in `debugger.py` (lines 49-58) repeats the double decode for a 16-byte register buffer. `PassiveCollector.on_formula_entry` in `passive_probe.py` (line 97) hand-decodes a u64 beside `process.decode_u64`. `find_clock3.py` calls `struct` directly too. `watch_window.py` (lines 258-259) reads the back and front windows by raw offset, bypassing `EngineModel`. `walk_edges.py`, `engine_finder.py`, `find_engine.py` and `poll_windows.py` also read fields directly. The x1000 seconds-to-ms factor appears across many scripts with no helper.

They give identical numbers today. One verifier ran many values through all the decoders, including 0.0, -0.0, 0.085, 1.0, infinity, and a synthetic snapshot reading 171.43 ms and score 1234. Another ran six flag values through the precision tests, including 0x0FFF, 0x1000 and 0xFFFFFFFF; every form gave the same answer. The only difference is which error a short read raises.

Nothing shows in the app. If a field's type or offset changed in `EngineModel` only, `watch_window` and `walk_edges` (which use `live.py`) would decode it wrong. That would show only in probe console output. If precision mode ever depended on more than this one bit, only the copy someone remembered would learn it.

The `process.decode_*` functions should own bytes. `EngineModel` should own fields, given a one-read snapshot method so `live.py` only wraps it. A pure helper such as `engine.is_precision(flags)` should own the bit test, called by all three places. One `s_to_ms` helper should own the factor.

The copies came in with 283e028 (2026-09-26). No user decision for a second decoder was found in `CONTEXT.md`, `docs/adr/` or the 2026-09-25 and 2026-09-26 plans; only a code comment explains it.

*Owner proposed: tools/ch_probe/process.py decode_double / decode_u32 / decode_u64 for bytes; EngineModel in engine.py for field offsets and types (with a one-read snapshot method and a pure is_precision(flags) helper); one s_to_ms helper. Candidates: sweep-probe1-c2, sweep-probe1-c20, sweep-probe2-c7, sweep-probe2-c8.*

#### 231. Process memory read/write is bound twice, in process.py and debugger.py

The question is: how does the probe read and write bytes in Clone Hero's memory?

Two modules each answer it with their own Windows bindings. In `tools/ch_probe/process.py`, `_make_reader` (line 341) and `_make_writer` (line 368) wrap `ReadProcessMemory` and `WriteProcessMemory`. In `debugger.py`, `_Win32.__init__` loads kernel32 again, and `Debugger._raw_read` (line 567) and `Debugger._raw_write` (line 578) wrap the same two calls. `Debugger.attach` also opens a second full-access process handle, even when the caller already holds one from `process.open_process`.

They agree today. The verifier checked by reading. A full read returns the same bytes, a partial read returns the same short bytes, and both raise an OS error on failure. The write side differs on purpose: the debugger also flushes the instruction cache. That is a real extra job, needed for breakpoint patches.

If one binding were fixed, for example how partial reads are handled, the other would keep the old behaviour. Probe tools would then read memory two different ways. Nothing shows in the Hydra GUI.

The owner should be `process.py`, with the debugger adding only its cache flush on top. Both bindings came in 396082a (2026-09-21). The only reason given for the split is the debugger.py module docstring, which is a code comment, not a user decision.

*Owner proposed: tools/ch_probe/process.py (reader/writer); debugger keeps only the instruction-cache flush. Candidates: sweep-probe1-c19.*

#### 232. The 'did the press hit' check is written three times, settle time twice

The question is: after the probe presses a key, did the game count it as a hit?

Three runners write "hit means the score rose" inline. `main` in `walk_edges.py` (line 261) and `drive_inputs` in `active_probe.py` (line 175) read the score 250 ms past the note. Each keeps its own `SETTLE_MS = 250` (walk_edges line 55, active_probe line 66). The active_probe comment even says "as walk_edges.py". `main` in `play_chart.py` (line 442) reads the score about 8 ms after key-down instead. `main` in `hit_detect.py` (line 137) watches the `+0xb0` hits counter rather than the score. `hit_time_report` in `watch_window.py` counts score rises only as a cross-check.

No input where they disagree has been shown. The one recorded run, `results/hit_detect.csv`, has 60 rows. The counter and the score rose together on every one. That run had no misses, so it can't prove agreement on misses either. The drift part of the original claim is unproven.

If one settle time is changed, walk_edges and active_probe would stop matching. play_chart could also mislabel a hit as a MISS if the game's frame lands after its quick read. That has not been observed. It would show only in probe console output.

The owner should be one helper next to `EngineModel.score` in `engine.py`, with a single settle constant. The copies came in with 283e028 and aa3ad89. The audit-fixes plan body chose to reuse walk_edges' rule by writing it again. That is the plan writer's call, not a user decision.

*Owner proposed: one 'pressed input hit?' helper beside EngineModel.score (engine.py), with one settle constant. Candidates: sweep-probe1-c6.*

#### 233. probe_chart.py and probe_songs.py each write probe .chart text, with their own kick constant, tempo rounding and defaults

Two questions overlap here. How is a probe song's .chart text written? And what resolution, tempo and kick note does a probe chart use?

Two modules write the chart from scratch. In `tools/ch_probe/probe_chart.py`, the section builders `_song_section` (lines 64-78), `_sync_track_section` (line 87) and `_expert_drums_section` (line 105) build it. Its kick comes from `constants.PROBE_CHART_NOTE_KICK = 0` (`constants.py` line 176). In `probe_songs.py`, `chart_text` (lines 121-142) writes its own [Song] header, SyncTrack and note lines. It never imports constants and keeps its own `KICK = 0` (line 38).

The tempo line is worded differently. probe_chart rounds bpm x 1000 (line 87). probe_songs truncates it (line 135).

| BPM | probe_chart `int(round(bpm*1000))` | probe_songs `int(BPM*1000)` |
|---|---|---|
| 125.0 (used today) | 125000 | 125000 |
| 119.9999 | 120000 | 119999 |
| 120.0009 | 120001 | 120000 |

The defaults repeat too. `probe_note_ticks`, `build_probe_chart_text` and `generate_probe_chart` in probe_chart.py, plus the `generate_probe_chart` stub in `interfaces.py`, all default to 192 ticks per beat at 120 BPM. The stub uses a bare 0 for the note. But the live probe passes `probe_songs.RESOLUTION` and `BPM`, which are 480 and 125.

They agree today. This was checked by reading and a small Python run. The kick is 0 in both. A tempo split needs a fractional milli-BPM, and `probe_songs.BPM` is fixed at 125.0, so someone would have to edit that constant.

If the copies split, the Window Map and Edge Walk charts from probe_songs would get a different tempo or kick from the active-probe chart. Probe notes would land at different times. Nothing reaches the Hydra GUI.

probe_chart's builders and `PROBE_CHART_NOTE_KICK` should own the format, with `probe_songs.chart_text` calling them. One constant pair for resolution and tempo should feed both, either `probe_songs.RESOLUTION`/`BPM` or new constants in constants.py. probe_songs came in with 283e028 (2026-09-26). probe_chart's rounding dates from 396082a. No user decision was found.

*Owner proposed: tools/ch_probe/probe_chart.py section builders and constants.PROBE_CHART_NOTE_KICK, called by probe_songs.chart_text, with one resolution/BPM constant pair. Candidates: sweep-probe1-c15, sweep-probe1-c26, sweep-probe2-c4, sweep-probe2-c5.*

#### 234. Probe song folder, sub-folder names, name prefix and folder writer are spelled out twice

Two questions overlap. Where are the probe songs installed, and what are they called? And how is a probe song folder written, including how long the song is?

The install location is typed twice with the same literal, `C:\Clone Hero\songs\Hydra Probe`. One copy is `DEFAULT_OUT` in `tools/ch_probe/probe_songs.py` (line 40). The other is `PROBE_ROOT` in `experiments/live.py` (line 30). `watch_window.resolve_song_dir` and `walk_edges.py` read `PROBE_ROOT` to find the songs. `active_probe.py` already imports `probe_songs.DEFAULT_OUT`, so the owner exists and that file is not a folder copy.

The sub-folder names are written twice too. "Window Map" and "Edge Walk" appear in `probe_songs.main` (line 215) and again in `watch_window.py` (line 239) and `walk_edges.py` (line 184).

Two functions write a probe song folder. `write_song` in probe_songs.py (line 196) writes the chart, song.ini, manifest.json and audio. `write_probe_song` in `active_probe.py` (line 93) rebuilds its own folder and name assembly and writes no manifest. Both build the "Hydra Probe - " title themselves (active_probe line 101, parallel to write_song line 200). The song-length rule, last note plus `SILENCE_MS`, appears three times: in both writers and again in `probe_songs.main` for its printout.

All copies agree today. A Python run found `DEFAULT_OUT == PROBE_ROOT`, and the rest was checked by reading. The missing manifest is a real output difference between the two writers, not a disagreement about one fact.

If the install path or a folder name changed in probe_songs alone, watch_window and walk_edges would look for manifest.json in the old folder, find no song, and fail to load it. Nothing reaches the Hydra GUI.

`probe_songs` should own the root, the sub-folder names, the name prefix and the folder writer, because it is the module that writes the files. `write_song`, or a shared helper there that returns folder and length, should replace `write_probe_song`'s copy. The copies came in with 283e028 (2026-09-26) and aa3ad89. No user decision was found.

*Owner proposed: tools/ch_probe/probe_songs.py: DEFAULT_OUT, the sub-folder names and the 'Hydra Probe - ' prefix as constants that live.py and the runners import, and write_song (or a shared helper returning folder and length). Candidates: sweep-probe1-c16, sweep-probe1-c17, sweep-probe2-c6.*

#### 235. Normal and precision window predictors repeat the same inner formula

The question is: what hit window does Clone Hero's formula predict for a given note spacing?

Two functions in `tools/ch_probe/experiments/analysis.py` answer it, one per mode. `predicted_window_normal` (line 211) and `predicted_window_precision` (line 238) both write out the same pre-scale step, `t = spacing * divisor`. Both also write the same inner term, `(t*c1 - t**exponent*c2)*c3`, character for character. Only the outer step differs, and that is by design. Normal mode subtracts `c4`. Precision mode subtracts the term from `c0`. `constants.py` (lines 109-132) describes the shared pieces the same way.

They agree today, because the text is identical. The verifier spot-checked one case: with divisor 1, exponent 2, c1 = 2, c2 = 0.01, c3 = 10 and t = 10, both inner terms give 190.

If someone corrects the inner term in one function only, the two modes would predict windows from different formulas. That would show only in probe analysis output. `summarize_active` calls only the normal predictor, and only a test calls the precision one. The Hydra GUI is untouched.

The owner should be one private helper in analysis.py that both predictors call. The duplication came in 396082a (2026-09-21). The 2026-09-24 derivation-fixes plan changed the exponent default in both functions but said nothing about sharing the term. No user decision was found.

*Owner proposed: one private helper in tools/ch_probe/experiments/analysis.py returning the scaled parabola term. Candidates: sweep-probe1-c21.*

#### 236. Finding and focusing the Clone Hero window is written four times

The question is: how does a runner find the Clone Hero window and bring it to the front before pressing keys?

Four runners each do it. `find_game_window` in `tools/ch_probe/experiments/active_probe.py` (line 190) is the only named version. `main` in `walk_edges.py` (line 209), `main` in `play_chart.py` (line 342) and `main` in `pad_flash_test.py` (line 54) each write the same lookup inline. All four search by the literal title "Clone Hero" and focus with `SetForegroundWindow`. `constants.py` already owns the process name, `PROCESS_NAME = 'Clone Hero.exe'`, but has no window-title constant.

They agree today on the lookup and the focus call. The verifier checked by reading. They differ only when no window is found: pad_flash_test stops, while the other three carry on without focusing.

If the game's window title changed, each copy would need its own fix. A missed copy would press keys into whatever window has focus. That would show only at the game. The Hydra GUI is untouched.

No owner exists yet. A small helper beside `constants.PROCESS_NAME`, or in `input_driver`, seeded from `find_game_window`, would serve all four. The active_probe copy came in aa3ad89 and the other three in 283e028 (both 2026-09-26). No user decision was found.

*Owner proposed: none yet; candidate: a small window helper in input_driver or beside constants.PROCESS_NAME, seeded from active_probe.find_game_window. Candidates: sweep-probe1-c24.*

#### 237. milestone1 re-lists the probe's constant set instead of asking EngineModel

The question is: which game constants make up the probe's constant set?

`EngineModel.constants` in `tools/ch_probe/engine.py` (lines 148-174) owns the list. It reads the four window constants, the divisor, the exponent, the hit-check threshold, and both formula tables. It needs only a process handle, not a live engine. A test already calls it that way. `main` in `experiments/milestone1.py` reads the same seven named constants and both tables one by one (lines 34-68), never going through `EngineModel`.

They agree today. Both read the same addresses from `constants.py`, so the values they print match for any running game. The verifier checked by reading.

Only the membership list is duplicated. If a constant is added to or dropped from `EngineModel.constants`, milestone1 won't follow. Its printout would then show a different set from what the probe actually checks. That is console output from a diagnostic script. The Hydra GUI is untouched.

The owner should be `EngineModel.constants`. milestone1 should call `EngineModel(proc, None).constants()` and print the result. The owner existed first, from 396082a (2026-09-21). milestone1.py came later in 283e028 (2026-09-26).

*Owner proposed: engine.EngineModel.constants. Candidates: sweep-probe1-c25.*

#### 238. Three Python .chart readers pick drum notes differently

The question is which lines of a .chart file are drum notes. Three Python readers answer it, each a little differently. `parse_drum_ticks` in `tools/ch_probe/tests/test_probe_chart.py` (lines 35-56) keeps every note line inside `[ExpertDrums]`, one tick per line. `chart_ticks` in `tests/test_probe_songs.py` (lines 18-19) searches the whole file and matches only the exact text `N 0 0`, a kick with no sustain. `parse_chart` in `experiments/play_chart.py` (lines 89-130) reads `[ExpertDrums]`, filters to `CHART_DRUM_NOTES`, and groups notes by tick.

| Edge chart line | parse_drum_ticks | chart_ticks | play_chart.parse_chart |
|---|---|---|---|
| `50 = N 0 0` in [ExpertSingle] | ignored | 50 | ignored |
| `100 = N 0 0` and `100 = N 1 0` | 100, 100 | 100 | (100, [RED, KICK]) |
| `200 = N 0 10` | 200 | ignored | (200, [KICK]) |

On what the probe generators write today they agree. A verifier run got 3840, 4051, 11731, 11761 from both test readers. On the edge chart above they disagree, shown by a scratch run. Nothing reaches Hydra's screen. A test could pass on a chart that play_chart reads differently. One shared Python reader should own this. The C++ `ChartParser` in `src/parse/` stays the owner for real charts, but Python cannot call it. Introduced in 396082a (2026-09-21) and 283e028 (2026-09-26). The writer side is duplicated too; see the probe .chart writer finding.

*Owner proposed: one shared Python .chart note reader for the tests and play_chart (C++ ChartParser stays owner for real charts). Candidates: sweep-probe2-c11.*

#### 239. Tick-to-ms identity assumed in four places instead of calling ms_to_ticks

The question is how a span of milliseconds converts to chart ticks. `ms_to_ticks` in `tools/ch_probe/probe_chart.py` (line 60) owns it: resolution × bpm / 60000. At the chosen 480 ticks per beat and 125 BPM, one tick is one millisecond. Other places lean on that identity instead of calling the owner. `test_one_tick_is_one_ms` in `tests/test_probe_songs.py` (line 24) restates the formula inline. `plan_inputs` in `experiments/active_probe.py` (lines 81-90) uses note ticks directly as ms. `probe_songs.py` (`Timeline` and `chart_text`, lines 71-92 and 140) writes each note's ms straight out as its tick. `walk_edges.py` and `watch_window.py` then read that manifest back as ms. `test_runners.py` (line 212) pins first_ms at 3840.

They agree today. With `probe_songs.BPM` set to 120, `plan_inputs` would report 3840 ms where the true time is 4000 ms. The finder said this would go wrong silently. The verifier refuted that part: two existing tests would fail. Nothing shows in Hydra. The plan header (lines 15-17) picks 480/125 but does not decide the duplication. `ms_to_ticks` plus an inverse `ticks_to_ms` should own it, so a tempo change lands in one place. Introduced in 396082a (2026-09-21) and 283e028 (2026-09-26).

*Owner proposed: tools/ch_probe/probe_chart.py ms_to_ticks plus an inverse ticks_to_ms. Candidates: sweep-probe2-c12.*

#### 240. Tests and the fill report hand-write key parts instead of asking Settings and Lens

The question is what key a stored result is filed under. `Settings::record_key` in `src/app/config.cpp` (around line 175) owns it, and `store::Lens::from` owns the settings part. `tests/test_app_state.cpp` writes the default chart mode "Expert Pro Drums, 2x Bass" and cap 4 as literals (around line 44). `seeded_store` and the tests around lines 270, 405 and 460 build the key by hand from those literals, while still calling `Settings{}.lens()` for the rest. The legacy-fills part of the lens is hand-coded as 1 or 0 in `key_for` in `tests/test_fill_report.cpp` (around line 33). Production does the same in `src/app/fill_report.cpp` (around lines 151 to 153) instead of calling `Lens::from`.

All copies agree today, shown by reading. `Settings{}` gives the same chart mode and cap 4, and `Lens::from` gives the same lens as `key_for` for both fill rules. The `kMode` literals in `test_dm_report.cpp` and `test_fill_report.cpp` are only labels passed to both sides of each test, so they are not copies.

If the lens encoding changed, the fill report could look up the wrong rows and show empty or wrong comparisons. The tests should call `Settings{}.record_key(md5)`, keeping the single pin in `tests/test_config.cpp`. `key_for` and `fill_report.cpp` should call `Lens::from`. The literal came in 5b95046 (2026-08-22) and the lens line in 1f3efdf (2026-09-29). No decision was found in ADRs or CONTEXT.md.

*Owner proposed: Settings::record_key and store::Lens::from. Candidates: sweep-tests1-c8.*

#### 241. A note's dynamic has two name tables in model.cpp

The question: what is a note's dynamic called in text? `src/core/model.cpp` has two name tables for one enum. `dynamic_str` (line 30) gives lowercase names. Its only caller is the `dynamic` field in hydra_replay's per-note JSON (`tools/replay.cpp` line 366). `ChordNote::str` (lines 66-70) gives capitalised modifiers inside note strings like "(Ghost, 2x)". The Dynamics tab in `src/ui/dynamics_tab.cpp` (lines 123-124 and 163-164) types "Ghost" and "Accent" a third time as column headers. `tests/test_replay.cpp` (lines 727-729) pins `dynamic_str`.

| Dynamic | `dynamic_str` | `ChordNote::str` |
|---|---|---|
| Normal | none | (nothing shown) |
| Ghost | ghost | Ghost |
| Accent | accent | Accent |

They agree apart from case and how Normal is shown. Shown by reading. Nothing is wrong on screen today: the JSON says "ghost" and the GUI says "Ghost". If one table gained or renamed a dynamic, the JSON and the GUI would name it differently.

`dynamic_str` should own the names, with display sites capitalising the word. `dynamic_str` came in c4f1369 (2026-09-24). The "Ghost" modifier dates to 3b69958 (2026-08-15, the C++ port).

*Owner proposed: dynamic_str in src/core/model.cpp (display sites capitalise). Candidates: sweep-tests3-c11.*

#### 242. Path report counts charts twice: C++ subtitle and JavaScript tile

The question: how many charts does the path report list? `generate_report` in `src/app/report.cpp` (lines 387-392) counts distinct hyhash values for the subtitle "N records across M charts". The page script's `stats()` (line 148) counts distinct chart ids `c` for the Charts tile. `build_html` (lines 333-339) assigns one `c` per hyhash, so both count the same thing. The test "path report counts charts by hash in the tile and the subtitle" in `tests/test_report.cpp` (line 703) pins only the tile side.

They agree today with no filter and with the default "Best path only" view. Shown by reading. With a tier or search filter they differ on purpose. The shared page script passes only visible rows to `stats()` (`src/app/html_page.cpp` lines 333 and 399). A "Beyond" filter matching one of three charts gives a subtitle of 3 and a tile of 1. That matches the neighbouring "Paths shown" tile, so it is a filtered-view count, not drift.

What is left is one counting rule written twice, once in C++ and once in JavaScript. If the chart key changed in only one place, the two numbers would differ even on the default view.

`generate_report` could pass the unfiltered count in the payload; the tile still needs its own count over visible rows. The subtitle count came in 728e067 (2026-09-26), the tile in 5457d7d (2026-09-27).

*Owner proposed: generate_report in src/app/report.cpp (unfiltered count); the tile keeps its visible-rows count. Candidates: sweep-tests3-c9.*

#### 243. Two different rules decide whether the all-0 section is redundant

The question is: does the best all-0 path (a path that never skips an activation) add anything beyond the main path list? Two layers each decide this, with different tests.

`attach_allzero` in `src/search/pather.cpp` (lines 43-49) skips the all-0 search when the optimal path is all-0 and needs no timing (difficulty at or below 0). `build_path_list` in `src/app/path_view.cpp` (lines 406-420) hides the section when any all-0 variant has the same score and notation as any listed path.

| Case | Search side | Display side |
|---|---|---|
| Optimal is all-0, no timing | skip, store nothing | hidden (never tests) |
| Optimal is all-0, needs timing | search at 0 ms | show, unless score+notation match |
| Optimal not all-0 | search | hide only on a match |

Whether they agree is unknown. The verifier found one plausible split by reading, never by a run. It would need a chart with more ties at the top score than `Rules::max_tied_paths`, where a zero-skip tie with different notation got folded out of the main list. The search side would call it redundant, while the display side would show it at +0. On screen, the search side always wins that case today.

Both rules arrived in commit 21ddf53 (2026-08-18), and ba0885a later touched the display wording. No user decision was found in `docs/adr`, CONTEXT.md line 81 (definition only) or commit messages. One model-layer predicate should own "redundant", since it is a fact about the record, not about search speed or drawing.

*Owner proposed: One redundancy predicate on the model layer (for example a HydraRecord function) called by build_path_list, with attach_allzero's skip kept only as a shortcut that predicate guarantees. Candidates: P2-4.*

#### 244. The 2x Bass checkbox checks 'Expert only' itself

The question is: can 2x Bass apply at the chosen difficulty? Settings owns the answer. `Settings::effective_bass2x` in `src/app/config.cpp` (lines 133-134) returns the view toggle only on Expert. Every loader uses that.

The Settings bar asks the question again on its own. `render_drum_options` in `src/ui/settings_bar.cpp` (line 61) compares the difficulty with Expert itself. That copy decides whether the checkbox is greyed out and shows the "Expert-only charting concept" tooltip. The checkbox's ticked state already comes from `effective_bass2x`, so only the enabled state and tooltip are a second copy.

Both copies agree today, shown by reading: Expert means eligible, and Hard, Medium and Easy mean not eligible. They would drift if the eligibility rule ever changed in one place only. Say a chart could mark 2x on Hard. The analysis would honour it while the checkbox stayed greyed out, or the reverse.

Settings should own this, because it already owns `effective_bass2x`. A small `bass2x_available()` beside it would let both the analysis and the Settings bar ask the same function. The Expert check in `src/ui/library_toolbar.cpp` (line 96) is for the leaderboard button, a different question.

*Owner proposed: Settings in src/app/config.cpp, via something like a bass2x_available() that effective_bass2x and settings_bar both call. Candidates: P1-3.*

#### 245. Search code restates part of the all-0 path definition

The question is: is this path an all-0 path? That means it has at least one activation and never skips one. Two places say so.

`Path::is_allzero` in `src/core/model.cpp` (line 598) states the full rule. `search_allzero` in `src/search/pather.cpp` (line 91) has its own narrower check. It drops a result only when it is a single path with no activations. It never calls `is_allzero` and never checks skips; it relies on the engine's no-skips mode for that. The `no_skips_` branch in `Engine::run` (`src/search/engine.cpp`, line 1116) is how all-0 paths get produced, not a copy of the definition. `build_path_list` only reads the stored list.

They agree today. The verifier showed this both ways. By reading, a no-skips run can only end with an activation-less path when that path is the only result. By a run, the existing test "search_allzero returns only all-0 paths inside the 0 ms limit" in `tests/test_search.cpp` (line 327) passed on 97 charts, 68 of them with an all-0 path.

If someone changed no-skips handling, the two wordings could split. The Best all-0 section could then list a path that is not really all-0. Both copies came in commit 21ddf53 (2026-08-18). `Path::is_allzero` should own the rule, and `search_allzero` should keep only paths that pass it.

*Owner proposed: Path::is_allzero in src/core/model.cpp; search_allzero should filter its result with it. Candidates: P1-4.*

#### 246. Targeted search builds its graph at the raw cap, not graph_build_cap

The question is how tall a run's score graph is built: at the SP cap, or at the cap limited to the song's phrase count. `graph_build_cap` in `src/search/pather.cpp` (around line 139) owns the answer. It takes the smaller of the cap and the phrase count, with a floor of 1. `analyze_chart` (around line 187) uses it. `search_target` (around line 101) builds at the raw cap and never asks. `search_target` is what `hydra_replay target` runs (`cmd_target` in `tools/replay.cpp`).

| Cap, phrases | graph_build_cap | search_target |
|---|---|---|
| 4, 10 | 4 | 4 |
| 32, 3 | 3 | 32 |
| 8, 0 | 1 | 8 |

The ceilings differ whenever a song has fewer phrases than the cap. The paths still agree today, shown by reading. `ScoreGraph::max_sp_bars` in `src/search/graph.cpp` (around line 251) limits activations by phrase count anyway. `Engine::advance` can't bank more bars than there are phrases. The `extend_deacts` ceiling can't bind any earlier. Runs back this only at cap 4: `tests/test_search.cpp` (around line 608) and `tests/test_replay.cpp` (around line 108). Caps above 4 rest on reasoning alone.

Nothing on screen differs now. If `graph_build_cap` or `max_sp_bars` changed, `hydra_replay target` would stop reproducing the stored paths.

`graph_build_cap` should own this, and `search_target` should call it the way `analyze_chart` does. It fits beside the known spwin-12 item about meter height.

*Owner proposed: graph_build_cap in src/search/pather.cpp. Candidates: R5-A1-1.*

#### 247. Multiplier-squeeze direction is guessed from the combo instead of from note payouts

The question: which multiplier does a multiplier-squeeze chord reach, and which end of the chord should be hit last or first? A multiplier squeeze is a chord that straddles a combo step, so some of its notes pay at the old multiplier and some at the new one.

The real answer lives in `category_scores` in `src/core/scoring.cpp` (lines 41-42). It pays note i of a chord at `to_multiplier(combo+1+i)`. Two functions in `src/core/model.cpp` answer the same question by hand. `MultSqueeze::multiplier` (line 365) returns `to_multiplier(combo_) + 1`. `MultSqueeze::direction` (lines 367-369) says "high" when `combo_ % 10 == 7` and "low" otherwise. It never looks at how many notes the chord has. `howto`, `notationstr`, `build_multsqueezes` in `src/app/path_view.cpp` and `Path::pathstring_verbose` all read these results.

| Combo, notes | Paid at | Payout says | `direction()` says |
|---|---|---|---|
| 7, 3 | 1,1,2 | high | high |
| 7, 4 | 1,1,2,2 | tie | high |
| 8, 2 | 1,2 | tie | low |
| 8, 3 | 1,2,2 | low | low |
| 6, 4 | 1,1,1,2 | high | low |
| 7, 5 | 1,1,2,2,2 | low | high |

They agree for every chord `MultSqueeze::applies` accepts today (combos 7/8, 17/18, 27/28). This was shown by reading and by a scratch check of combos 0 to 39. The last two rows split, but `validate()` throws on them first. Those are the 4- and 5-note chords that known finding score-1 says `applies()` wrongly rejects. `multiplier()` cannot disagree, because a drum chord has at most 5 notes and so crosses exactly one step.

Nothing shows on screen today. If `applies()` is widened, the Paths tab's "Hit X last/first." advice would name the wrong end for some 4- and 5-note chords. The "Nx" label would stay right.

Both literals arrived in the C++ port, commit 3b69958. No user decision on direction was found in CONTEXT.md's "Multiplier squeeze" entry, docs/adr or plan headers. Decision 22 in `docs/superpowers/plans/2026-09-24-derivation-fixes.md` covers only the fixed combo set.

`category_scores` should own this, because it already decides where each note is paid. `direction()` would compare notes paid before the step against notes paid past it. `multiplier()` would read `to_multiplier(combo_ + chord_.count())`.

*Owner proposed: src/core/scoring.cpp category_scores (per-note payout over src/core/timing.cpp to_multiplier). Candidates: R3-A4-1.*

#### 248. Copy path builds the multiplier-squeeze 'Nx' label itself

The question: how is a multiplier squeeze written as text, such as "2x"?

`MultSqueeze::notationstr` in `src/core/model.cpp` (lines 376-378) writes `std::to_string(multiplier()) + "x"`. The Paths tab uses it through `build_multsqueezes` in `src/app/path_view.cpp` (line 150). `Path::pathstring_verbose` in the same file (line 502) rebuilds the same string inline instead of calling it. That copy feeds Copy path in `src/ui/paths_tab.cpp` (line 518) and is pinned by a test in `tests/test_model.cpp` (line 174).

They agree today, shown by reading. Both read `multiplier()`, so every accepted chord gives 2x (combos 7/8), 3x (17/18) or 4x (27/28). No input splits them today.

Nothing differs on screen now. If someone changed `notationstr`'s wording, say to "x2" or to add the direction, the Paths tab would show the new label but the copied path text would keep the old one.

The inline copy came in with the C++ port, commit 3b69958. `MultSqueeze::notationstr` should own the label, because it is the one formatter the Paths tab already trusts. `pathstring_verbose` would call it. This is separate from the finding about which multiplier and direction are reached; that one asks what the value is, this one asks how it is written.

*Owner proposed: src/core/model.cpp MultSqueeze::notationstr. Candidates: R3-A4-3.*

#### 249. Two different 'same path' rules, and the Preview cache key skips SP facts

The question: are two paths of one record the same path?

Two places answer it with different rules. `build_path_list` in `src/app/path_view.cpp` (lines 411-419) treats paths as the same when the score and the short notation match. It uses this to decide whether the all-0 section adds anything. `path_overlay_key` in `src/app/preview_view.cpp` (lines 622-628) builds a cache key from the score and the verbose notation, which adds the whole-number squeeze and early-fill milliseconds. `PreviewController::open` in `src/ui/preview_controller.cpp` (lines 37-50) trusts that key. With the same chart, cap and key it returns early and keeps the old scene. It also overwrites the rules without comparing them.

| Two paths differ in | `build_path_list` | `path_overlay_key` |
|---|---|---|
| score | different | different |
| only the ms values | same | different |
| only deact tick, collected phrases or clamp | same | same |

Neither rule looks at activation ticks, deact ticks, collected phrases or clamp ticks. Whether they disagree on a real chart is unknown. The verifier confirmed the rules by reading but found no concrete input. One dedupes a list and the other caches a scene, so they may not be the same question.

If the gap were hit, the Preview's SP floor, gauge and score box could keep the previous result's SP end after a re-analysis, while the Paths tab shows the new one. That effect is unproven.

The key formula traces to 711ba62 (2026-09-26). One path-identity helper beside `Path` in `src/core/model` should own this, built from activation ticks, deact ticks and score. Both functions would call it, and the Preview key would also include the rules. Paths-tab pointer selection is state, not a third rule.

*Owner proposed: src/core/model.cpp (one path-identity helper beside Path). Candidates: R5-A4-1.*

#### 250. 'Disco flip only under Pro Drums' is written once in each parser

The question is: does a disco-flip section swap red and yellow on this chord, given the current Pro Drums setting?

Both parsers spell out the gate themselves. `MidiParser::push_timestamp` in `src/parse/song.cpp` (around line 571) and `ChartParser::push_timestamp` (around line 970) both pass `mode_pro_ && flag_disco_` to the shared `emit_chord_timestamp`. That helper (around line 151) just takes the result as a plain true/false. The flip itself has one owner, `Chord::apply_disco_flip` in `src/core/model.cpp`.

They agree today. Both flip only when Pro Drums and the disco section are both on. The flam argument beside it differs for a stated format reason (.chart has no flam marker). The disco gate has no such reason.

If one copy changed, the effect would reach scores, not just lanes. `apply_disco_flip` marks the moved note as a cymbal unconditionally, and a cymbal adds 15 points. So applying the flip with Pro Drums off in one parser would add 15 points per flipped snare for that format only. It would also change the Dynamics tab's red and yellow rows and the Preview lanes.

`emit_chord_timestamp` should take both flags and apply the gate once. The rule came from the port, 3b69958. No user decision was found: CONTEXT.md, docs/UserGuide.md and docs/adr never mention disco flip.

*Owner proposed: emit_chord_timestamp in src/parse/song.cpp (or a helper beside it). Candidates: R6-A5-2.*

#### 251. A path's parent folder is worked out by three hand-written helpers

The question is: what is the parent folder of a file path?

Three helpers each cut the path at its last slash or backslash. They are `parent_of` in `src/app/analysis.cpp` (around line 72), `dir_name` in `src/app/preview_source.cpp` (around line 36) and `exe_dir` in `src/app/config.cpp` (around line 20). A fourth way exists too: `src/app/report_files.cpp` and `src/app/report.cpp` use the standard library's `parent_path` and `filename`.

| Path | parent_of | dir_name / exe_dir |
|---|---|---|
| `C:\a\b\notes.mid` | `C:\a\b` | `C:\a\b` |
| `C:\a\b\` | `C:\a` | `C:\a\b` |
| `notes.mid` | empty | `.` |
| empty | empty | `.` |

They agree today for every real caller, by reading. `discover_charts` always builds paths with a separator, and Windows always returns an absolute exe path. So nothing on screen differs.

If a caller ever sent an edge shape, the Preview could look for song.ini and audio in the wrong folder. One helper in `src/core/winstr` or `src/core/strutil` should own this, or all three should use `std::filesystem::path` as `report_files.cpp` does. No user decision is needed for the edge behaviour.

*Owner proposed: One path helper in src/core/winstr or src/core/strutil (or std::filesystem::path everywhere). Candidates: R6-A5-4.*

#### 252. 'Rows at this cap' is written twice in the store's lookup SQL

The question: which result rows were stored under this exact SP cap and lens? The lens half is already shared through `lens_match` and `bind_lens`. The cap half is written twice.

`append_candidate_filter` and `bind_candidate_filter` in `src/store/record_store.cpp` (around line 416) serve `get_summaries`, `get_record`, `for_each_blob` and `list_records`. `analyzed_filter` and `bind_analyzed_filter` (around line 442) serve `has_record` and `analyzed_hashes`. Both now say a bare `sp_cap=?`. Both still take a `CapQuery` they never use, marked `[[maybe_unused]]`.

They agree today. `CapQuery` has only an exact cap now. Commit eaf3b73, which removed Auto, edited both copies by hand in the same way. That shows the rule has already needed two edits once.

A third place, the replace purge in `write_row` (around line 948), asks a different question. It finds the row with the new row's own unique key, bound from a stored number. It should stay exact whatever happens to `CapQuery`, so it is not a copy.

If `CapQuery` gained a second mode again, the two lookups could pick different rows. The library chip, the batch skip count and the song panel could then disagree about which row is the chart's result.

One small cap-match helper and binder in `record_store.cpp` should own it. Both filters would call it, and the dead parameters could go.

*Owner proposed: One cap-match helper and binder in src/store/record_store.cpp. Candidates: R3-A6-2.*

#### 253. Summary column list and order are hand-written in several SQL statements

The question: which result columns hold the path summary, and in what order? The named list is `kSummaryColumnList` in `src/store/record_store.cpp` (around line 228). `bind_summary` and `read_summary` (around line 132) use fixed offsets +0 to +9 that must match it.

Three statements spell it again by hand. `RecordStore::reindex` (around line 1438) has its own UPDATE list and binds the row id at a literal slot 11. `RecordStore::list_records` (around line 1537) has its own SELECT list and reads the cap at literal slot 16, then 17 to 19. `write_row`'s INSERT (around line 960) uses the named list but hard-codes ten placeholders. The table definition `kResultsColumnDefs` repeats the column set, but not an order that matters.

They all agree today, with ten columns. The verifier traced what an incomplete 11th-column change would do, by reading:

| Place | Effect of adding a column only to the named list |
|---|---|
| `reindex` | new column never written (right row still updated) |
| `list_records` | new field shows the cap number |
| `write_row` INSERT | fails at prepare, loudly |

The user sees nothing now. After such a change, listings would show a wrong value in the new field. `reindex` would leave it empty.

`kSummaryColumnList` with its bind and read helpers should own it, plus a column-count constant. The other three statements should be built from it, with their later slots computed from its size.

*Owner proposed: kSummaryColumnList with bind_summary / read_summary (src/store/record_store.cpp). Candidates: R3-A6-3.*

#### 254. 'Is a batch or analysis running?' is re-spelled next to its owner

The question: is a batch, or a single-chart analysis, running right now? The owners are `AppState::batch_running` and `AppState::analyze_running` in `src/ui/app_state.cpp` (around lines 256-258). Each asks whether the job exists and has not finished.

Three places write that test out again instead of calling them. `AppState::start_batch` (around line 437) and `AppState::start_analyze` (around line 515) use it as their guard. `render_actions_row` in `src/ui/library_toolbar.cpp` (around line 45) spells it as `batch_busy`. That flag greys out Scan library and Analyze. The GUI test harness repeats it too.

They agree today. Every copy says false with no job, true while running or paused, and false once finished but not yet cleaned up.

If one copy changed, say to treat a stopping batch as idle, the toolbar could enable Analyze while `start_batch` refuses. The user would click a live button and nothing would happen.

The inline guards date from the port (3b69958, 2026-08-15). The owners came in a3799cc and the toolbar copy in a591bba, both on 2026-09-27. The older copies were never switched over. The two `AppState` functions should own the answer, and the three copies should call them.

*Owner proposed: AppState::batch_running / AppState::analyze_running (src/ui/app_state.cpp). Candidates: R4-A6-2.*

#### 255. The chart's note total is counted three separate ways

The question is how many notes the chart has at the current difficulty and drum options.

Three places count it on their own. The engine sums it along each path. `ScoreGraph::build` in `src/search/graph.cpp` (line 108) puts each chord's note count on the graph edges. `Engine::advance` in `src/search/engine.cpp` (line 480) adds them up into `Path::notecount`. The path and fill reports show that as the Notes column (`src/app/report.cpp` line 298, `src/app/fill_report.cpp` line 191). The Dynamics tab recounts it. `count_dynamics` in `src/app/dynamics_breakdown.cpp` (around lines 104-118) walks the song and bins every note, and `played_total` gives the "of N" in "Dynamic notes: X of N" (`dynamics_tab.cpp` line 204). The Preview counts it a third time. `replay_path` in `src/core/replay.cpp` (line 145) keeps a running combo, and the Preview shows it as "combo N".

They agree today. Runs gave 1316 = 1316 on Pokemon Theme, and 905 on all five paths of How You Remind Me matching the replay sum. The Dynamics total was not run, but it counts the same song sequence. The only known split is 2x kicks sharing a tick with a normal kick, which is already parse-8 and screenA-1. `tests/test_search.cpp` (line 73) checks that all paths of one record agree. Nothing checks the engine count against Dynamics or the replay.

Nothing is wrong on screen today. If one counting rule changed alone, the Notes column, the Dynamics "of N" and the Preview's final combo could show three different totals.

A Song-level note count should own this, next to `Song::sp_phrase_count`. It is a fact about the chart, not about a path. The edge-summed count came from 3b69958 (the C++ port). The Dynamics count came from ec3ade9.

*Owner proposed: a Song-level note count, next to Song::sp_phrase_count in src/parse/song.cpp. Candidates: R3-A7-2.*

#### 256. One scanned-chart row is defined twice and converted by hand three times

The question is what one scanned chart row holds, and how a scan result becomes a library row. Two structs answer the first part. `ScanItem` in `src/app/analysis.h` (line 30) and `ChartLibraryEntry` in `src/store/record_store.h` (line 225) hold the same seven strings in the same order: md5, title, artist, charter, notespath, rootfolder, sig. The conversion is a positional brace copy (fields matched by order, not by name), typed three times. One is `ScanJob::run` in `src/ui/library_jobs.cpp` (around line 77). One is `scan_mode` in `tools/bench.cpp` (around line 137). One is the test "rescan cache reproduces the scan without reading chart files" in `tests/test_analysis.cpp` (around line 118). The rebuild in `discover_charts` (`src/app/analysis.cpp` line 429) starts from a different cache struct, so it is not a copy.

All three map each field to its same-named field, and they agree today. The verifier checked this by reading. There is no input where they differ. The risk is an edit: add a field or swap two strings in one struct, and a brace copy still compiles but leaves the field blank or swaps title and artist.

Nothing shows today. A missed copy in `ScanJob` would show blank or swapped titles in the GUI library, while bench and the test quietly differ.

One conversion in app, like `to_library_entry(const ScanItem&)`, should own it, or `ScanItem` could become an alias of `ChartLibraryEntry`. All of this dates from 3b69958 (2026-08-15).

*Owner proposed: app: one to_library_entry(const ScanItem&) conversion, or ScanItem as an alias of ChartLibraryEntry. Candidates: R3-A8-2.*

#### 257. Taskbar ID and app name are typed in both version.h and the installer

The question is what identity and name the app presents to Windows. The identity is the AppUserModelID, the tag Windows uses to group a pinned shortcut with the running window. `src/core/version.h` defines `kAppUserModelIDW = L"Hydra.Hydra"` (line 25), which `src/ui/main.cpp` (line 110) sets at start-up. `installer/hydra.iss` types `"Hydra.Hydra"` again on the Start Menu shortcut (line 66). Only comments on both sides say they must match. The name follows the same pattern. `kWindowTitleW = L"Hydra Deluxe"` in `version.h` (line 24) is the window title. `hydra.iss` types it five more times: AppName, AppVerName, the shortcut name, the Launch text and the uninstall message.

Both give the same answers today, shown by reading. There is no input where they differ. A one-sided edit would split them. The ID came in with the installer in 25704fb. The name was doubled by the rename in 74fe53b, whose message explains why the ID kept its old value. Nothing in docs/adr or CONTEXT.md decides it should be typed twice.

After a one-sided edit, a pinned Hydra would show as a second taskbar button. The Start Menu and Add/Remove name would differ from the window title.

The build should own both, next to the version in `CMakeLists.txt`. `build_installer.ps1` already passes `/DHYDRA_VERSION` to Inno this way (line 87), so the pattern exists.

*Owner proposed: CMakeLists.txt next to the version, passed to version.h and to Inno with /D like HYDRA_VERSION. Candidates: R3-A8-5.*

#### 258. Ticks-per-measure rule restated in Song's constructor and test fixtures beside apply_timesig

The question is what meter (ticks per measure) and time signature a song carries at a tick, the default 4/4 included. `apply_timesig` in `src/parse/song.cpp` (lines 118-122) owns the rule: ticks = resolution × 4 × num / den, and it writes the meter and the signature together. `Song::Song` in `src/parse/song.h` (lines 91-94) writes the opening `resolution * 4` and `{4, 4}` by hand instead. Charts with no tick-0 signature keep that default, so it matters in production. Several test fixtures also write meters by hand. `tests/test_preview_view.cpp` line 456, and `make_sp_song`'s extra meter used at line 1172, set 3/4 or 7/8 with no signature. Five fixtures in `test_preview_view.cpp`, `test_search.cpp`, `test_rules.cpp` and `test_replay.cpp` write 768 right after `Song(192)`, repeating the default.

| source | resolution 480 meter | signature |
|---|---|---|
| Song::Song | 1920 | 4/4 |
| apply_timesig(480,4,4) | 1920 | 4/4 |
| fixture at line 456, from tick 2880 | 1440 (3/4) | still 4/4 |

Shipped code agrees today: the integer math is exact for any resolution. Only test songs split meter from signature, shown by reading. That test checks only the position, so it passes.

Nothing shows in the app today. A future change to the default meter would need editing in two places.

`apply_timesig` should own it, and the constructor should call it. Introduced by 3b69958 and 34b2e51; no user decision found in either commit. The default itself is the known tempo-35 assumption.

*Owner proposed: apply_timesig in src/parse/song.cpp. Candidates: R4-A8-1.*

### Visibility 4: not visible (internal or test-only)

#### 259. 'Is this row decided by frontend timing' is answered twice in rate_activation

The question is whether a backend row is one the frontend timing decides. That means it was squeezed out, or the engine does not count it without a squeeze.

`rate_activation` in `src/core/squeeze_rating.cpp` answers it twice. The cap-clamped loop (around lines 178-191) writes it directly: `squeezed_out || (offset && !counted_without_squeeze)`. The first three arms of the scale ladder (around lines 106-132) imply the same set. Both call the owner `core::counted_without_squeeze` from `src/core/backend_value.h`.

| Row | scale ladder | cap loop |
|---|---|---|
| has offset | first three arms = cap set | same |
| squeezed out, no offset | skipped | counted |

They agree on every row the engine can produce. They differ only for a squeezed-out row with no `offset_ms`, which the engine never makes. This was shown by reading.

Nothing reachable shows on screen today. One stale comment does exist. `squeeze_rating.h` (around lines 120-124) says the cap rows are exactly the rows `rate_activation` judges. That stopped being true when 4f5fcc1 (2026-09-28) added the ladder's fourth arm for plain rows inside SP.

`squeeze_rating` should own one `is_frontend_decided(row)` predicate. The cap predicate came from 396082a (2026-09-21), reworded in 283e028 (2026-09-26).

*Owner proposed: core/squeeze_rating: one is_frontend_decided(row) predicate. Candidates: squeeze-7.*

#### 260. Meter height min(cap, phrase count) is computed twice in search

The question is how tall the SP meter can get: the cap, or the number of SP phrases if that is smaller.

Two places answer it. `graph_build_cap` in `src/search/pather.cpp` (lines 139-141) uses the smaller of the cap and the phrase count (at least 1) to decide how tall to build the graph. `ScoreGraph::max_sp_bars` in `src/search/graph.cpp` (lines 251-254) takes the smaller of the cap and the phrase count again, to decide which meter heights get an activation end. The engine clamp in `src/search/engine.cpp` (line 513) applies only the cap, so it is a different rule.

| Phrases | graph_build_cap | max_sp_bars |
|---|---|---|
| 1 or more | min(cap, phrases) | min(cap, phrases) |
| 0 | 1 | 0 |

They agree today, shown by reading. Inside the graph, the second min is redundant. With no phrases, the two differ (1 vs 0), but no activation can happen either way. The test in `tests/test_search.cpp` (line 608) pins that a 4-bar graph and one built at the phrase count store identical bytes.

Nothing shows on screen today or would if they drifted, unless activation ever became possible at 1 bar.

`max_sp_bars` should own it, since the graph already applies it. `graph_build_cap` could call it or drop its own min. `graph_build_cap` came in 64896c9 (2026-09-24, "Give each difficulty and cap rule one owner"); `max_sp_bars` dates from the port.

*Owner proposed: ScoreGraph::max_sp_bars in src/search/graph.cpp. Candidates: spwin-12.*

#### 261. All-0 path's cost against optimal is worked out twice in path_view

The question is how many points the all-0 path loses (or gains) against the optimal path.

Two functions in `src/app/path_view.cpp` answer it. `build_path_list` (around lines 422-432) subtracts the first flattened path's score and appends '(+N)' or '(-N)' to `allzero_label`. `build_path_buttons` (around lines 460-465) subtracts `record.best_path()`'s score and writes 'N below optimal' or 'N above optimal' on the button.

| Delta | Label in `build_path_list` | Button in `build_path_buttons` |
|---|---|---|
| below zero | '(-N)' | 'N below optimal' |
| above zero | '(+N)' | 'N above optimal' |
| zero | nothing | nothing |

They agree today. Both bases are the same path, `paths[0]`: `flatten_paths` in `model.cpp` puts it first, and `best_path()` in `model.h` returns `paths.at(0)`. An early claim that the two use different bases was refuted. The verifier found one structural gap by reading: with no paths but some all-0 paths, the label shows nothing while the button would say 'N above optimal'. No engine output like that was shown to exist.

Nothing on screen can show a drift. Only the button text is displayed. `allzero_label` has no reader in `src/ui`; only `tests/test_path_view.cpp` reads it.

The label copy came in ba0885a (2026-08-22) and the button copy in 577ddc2 (2026-09-27). One helper in `app/path_view` should own the sum, and the unused label delta could go.

*Owner proposed: app/path_view: one helper used by build_path_buttons; drop allzero_label's delta. Candidates: display-17, score-14, screenA-11.*

#### 262. Dynamics counts keyed by file path in memory but by md5 in the store

The question is which identity the Dynamics counts are filed under.

Two keys answer it in `src/app/dynamics_breakdown.cpp`. `dynamics_cache_key` (around line 175) uses the file path, pro setting and difficulty. `dynamics_store_key` (around line 179) uses the md5 (a fingerprint of the file), difficulty and pro setting. `AppState::update_dynamics` in `src/ui/app_state.cpp` (around lines 364-389) compares path keys for the in-memory result and loads from the store by md5.

They agree today, shown by reading. The finder's claim that they drift after an edit was refuted. The in-memory cache is cleared on every `select()` and `close_details()` (around lines 153-177). The selected entry's md5 does not change while the panel is open. So both keys always describe the same entry, and after an edit both go stale together.

The user sees nothing from this today.

Both keys came in with 4bc76ac (2026-09-24).

`dynamics_store_key` should own the identity, because md5 is the chart identity everywhere else. The in-memory cache could compare that key instead of the path.

*Owner proposed: dynamics_store_key (src/app/dynamics_breakdown.cpp); the in-memory cache could compare it instead of the path key. Candidates: parse-22.*

#### 263. Fill-deadline tests retype the deadline formula, and the 1.1 case pins nothing

The question is: by what millisecond must a drum fill spawn? That moment is the fill's spawn deadline.

The owner is `activation_fill_deadline_ms` in `src/search/graph.cpp` (lines 46-74). Three test cases in `tests/test_fill_deadline.cpp` type the same formula again instead of checking a known answer. The "CH 1.1 is unchanged 4-beats-before math" case (lines 116-119) computes fill end minus fill length minus 4 beats. That matches graph.cpp lines 52-54 word for word. The two "CH 1.0 preroll clamps" cases (lines 87-91 and 100-104) compute fill end minus fill length minus 250 ms or 10000 ms. Those match graph.cpp lines 71-73.

The copies agree today. I checked this by reading: the formulas are identical. Both 1.0 cases also pin a worked number (2675.0 and 34375.0), so a mistake shared with production would still fail there. The 1.1 case pins nothing. If the "4 beats" rule in graph.cpp were wrong, that test would repeat the mistake and still pass.

Nothing changes on screen, because only tests are involved. `activation_fill_deadline_ms` should stay the owner. The 1.1 test should check a literal expected ms, as the 1.0 cases and the 9562.5 anchor cases already do.

This came in with 406d473 (2026-08-31, "Legacy fill deadline is a CLI-only mode"). The verifiers looked for a user decision about these tests in that commit message, `docs/adr/0010` and `CONTEXT.md`, and found none.

*Owner proposed: activation_fill_deadline_ms in src/search/graph.cpp. Candidates: fills-18, tempo-11.*

#### 264. Squeeze-rating tests restate effective-ms and use a different beyond-edge rule

This finding covers two questions that `tests/test_squeeze_rating.cpp` answers again instead of asking production.

The first question is: how many effective ms does a backend gap give? `effective_backend_ms` in `src/core/squeeze_rating.cpp` (lines 63-64) answers it as |offset| x 2 / (1 + r). The Dumpweed fixture (lines 326-327) checks the result against its own `2.0*gap/(1.0+r)`. The two match for any positive gap. Line 328 also pins the literal 147.94.

The second question is: where does the timing edge sit for a given hit window? Production `beyond_edge_ms` (lines 207-212) starts at 0 and keeps the largest finite tier cutoff. The test loop at lines 803-806 keeps the last finite cutoff instead. Those are different rules.

| Hit window | Production (largest cutoff) | Test loop (last cutoff) |
|---|---|---|
| 0.5 ms | 2.0 | 1.0 |
| any whole number w >= 1 (85 is the only one tested) | 2w | 2w |

They agree on every window you can actually set. The config accepts only whole-number windows above 0 (`config.cpp` lines 84-86). The 0.5 ms case was found by reading, not by a run.

The hand-written "success" checks at lines 236-240 and 330-331 compare against numbers from real FC videos. Those are ground truth, so they are not counted as copies. The fixtures' literal deact ticks are covered in the tempo-12 finding.

Nothing changes on screen. `effective_backend_ms` and `beyond_edge_ms` should own these answers. The tests should call them or pin literal values.

*Owner proposed: effective_backend_ms and beyond_edge_ms in src/core/squeeze_rating.cpp. Candidates: squeeze-32.*

#### 265. Report test fixtures recompute DM delta, percent and status, plus fill-compare delta

The question is how a DM leaderboard row gets its delta, its percent of optimal and its status.

The owner is `collect_dm_rows` in `src/app/dm_report.cpp` (lines 198-207). It sets delta to optimal minus score. It sets percent to score / optimal x 100, but only when speed is 100 and optimal is above 0. Status is "above optimal" when the score beats optimal, and "matched" otherwise. The `add_dm` fixture in `tests/test_report.cpp` (lines 440-459) computes delta and percent itself, without the speed check, and types the status by hand. `tests/test_dm_report.cpp` (lines 113-114) recomputes the expected percent with the same formula. In that file the deltas are pinned as literals. The same report fixture has a second copy: `add_fill` (lines 466-481) sets delta to new minus old and types the status by hand. That restates `src/app/fill_report.cpp` (lines 195-198).

| Input | `collect_dm_rows` | `add_dm` rule |
|---|---|---|
| speed 150, score 90, optimal 100 | no percent | 90.0 |
| speed 100, any score | same | same |

They agree today. The differing input can't happen, because `add_dm` always uses speed 100. This was found by reading. The candidate claimed the tests would still pass if the speed check changed. That claim is wrong: `test_dm_report.cpp` lines 124-137 check that a speed-150 score gets no percent.

Nothing changes on screen. If someone edited a fixture carelessly, a sample page could show a status that contradicts its numbers. `collect_dm_rows` should own these values, and the fill-report row builder should own `add_fill`'s. The `add_dm` percent came in with 728e067 (2026-09-26). The test expectation came in with 0ad6444 (2026-08-18).

*Owner proposed: collect_dm_rows in src/app/dm_report.cpp (and fill_report's row builder for add_fill). Candidates: display-35.*

#### 266. Model and replay tests retype the per-note multiplier rule from scoring.cpp

The question is what multiplier each note in a chord scores at, and what points that gives.

The owner is `category_scores` in `src/core/scoring.cpp` (lines 41-44 and 74-75). It adds one to the combo for each note, then scores the note's basescore at `to_multiplier(combo)`. Two tests retype that rule. `tests/test_model.cpp` (line 56, and lines 69-70) computes `basescore() * to_multiplier(combo + 1 + i)` for the squeeze-out cut. `tests/test_replay.cpp` (lines 744 and 748) expects the multiplier to equal `to_multiplier(combo_before + 1)`. Production `replay.cpp` (line 93) just copies the multiplier that `category_scores` set.

| Chord | Production first-note multiplier | Test formula |
|---|---|---|
| one or more notes | `to_multiplier(combo + 1)` | `to_multiplier(combo + 1)` |
| empty | `to_multiplier(combo)` | `to_multiplier(combo + 1)` |

They agree today. The empty-chord case can't reach the test, because `test_replay.cpp` line 739 requires the chord to have notes. This was found by reading.

Two other places the finder named are not copies. `cutoffs_for` in `tests/test_stars.cpp` (lines 19-23) only loops over the owner `star_cutoff`, and every caller pins literal results. The basescore sum at lines 105-112 checks two production answers against each other. It compares owner calls with `Path::chart_base_score`.

Nothing changes on screen. `category_scores` should own the multiplier and the cut. The tests should pin literal expected values.

*Owner proposed: category_scores in src/core/scoring.cpp. Candidates: score-26.*

#### 267. A store test retypes the schema 2 column list and invents its table layout

The question is which columns the results table had in schema 2. Schema 2 is the database layout from before the "1.0 fills" column existed.

The owner is `kSchema2ResultsColumns` in `src/store/record_store.cpp` (lines 222-225). It lists 21 columns, and `add_fill_rule_column` uses it (lines 630-641). The test helper `downgrade_to_schema2` in `tests/test_store.cpp` (lines 1199-1220) types the same 21 names again, in the same order (lines 1214-1217). It also writes a full schema 2 `CREATE TABLE` (lines 1202-1213). Production keeps that table definition nowhere. The test builds it as the current table minus `legacy_fills`.

The two lists match today, name for name and in order. I checked this by reading.

Nothing changes on screen. If `kSchema2ResultsColumns` changed, the test's downgrade would still build the old shape. The migration test could then pass against a schema production never had. `record_store` should own both the column list and the schema 2 table definition, and expose them to the test. Both the constant and the test helper came in with 1f3efdf (2026-09-29, "Analysis settings: a '1.0 fills' checkbox scores fills by Clone Hero 1.0's rule").

*Owner proposed: record_store (src/store/record_store.cpp), exposing kSchema2ResultsColumns and a schema 2 DDL to tests. Candidates: store-26.*

#### 268. Path activation order is written in ActivationWalk and again in Path helpers

The question is: which activations does a path have, and in what order?

The rule is "the path's own activations first, then the variant tail". Three places write it. `ActivationWalk` in `src/core/model.h` (around lines 346-397) reads it in place. Inside it, `operator[]` and `iterator::operator*` each split the index the same way. `Path::all_activations` in `src/core/model.cpp` (around line 467) copies the two lists into a new vector. `Path::has_activations` (around line 475) checks that either list is non-empty.

They agree for every input, by reading. Both lists empty gives an empty list and false. Own [a1, a2] with tail [t1] gives [a1, a2, t1] and true. `ActivationWalk`'s own comment says it follows the order `all_activations` copies them in.

Nothing shows on screen today. If one copy changed the order, paths could list activations differently in different views.

`all_activations` and `has_activations` date from 3b69958 (2026-08-15). `ActivationWalk` came in 742d728 (2026-09-26, audit plan Task 2), which added it beside `all_activations` without rebuilding one on the other. No ADR or CONTEXT.md entry decides it. `ActivationWalk` should own the order. `all_activations` would copy from `walk_activations()`, and `has_activations` would return `!walk_activations().empty()`.

*Owner proposed: ActivationWalk in src/core/model.h. Candidates: sweep-core1-c5.*

#### 269. report_file_exists repeats winstr's file-exists test

The question is: does a file or folder exist at this path?

The owner is `file_exists_utf8` in `src/core/winstr.cpp` (around line 38). It asks Windows for the path's attributes with `GetFileAttributesW`, which returns an invalid marker when nothing is there. `report_file_exists` in `src/app/report_files.cpp` (around line 77) runs the same call directly on the report's wide path instead of calling winstr. `is_dir` in `src/app/analysis.cpp` (around line 61) repeats the same existence half before checking for a folder.

They agree today on every real path, by reading. An existing file or folder gives true. A missing, empty or blocked path gives false. Nothing shows on screen.

A separate copy sits in `tests/test_srb.cpp` (around lines 55-60). It is not an existence test. It hand-copies winstr's wide-to-UTF-8 conversion.

`report_file_exists` came in 21ddf53 (2026-08-18), after the owner already existed in 3b69958. No ADR or CONTEXT.md entry covers it. `core/winstr` should own the test, with a wide-path overload `report_file_exists` can call. The test fixture should call `hydra::wide_to_utf8`.

*Owner proposed: src/core/winstr. Candidates: sweep-core1-c14.*

#### 270. Frame clear colour copied into the GUI test harness

The question is what colour fills the frame behind the UI.

Two places answer it. `main` in `src/ui/main.cpp` (around line 180) sets (0.10, 0.11, 0.13, 1). `Harness::frame` in `tests/ui/uitest_harness.cpp` (around line 212) types the same four numbers. The app premultiplies by alpha, but alpha is 1, so the values stay identical.

The finder also claimed the 1280 window width was copied into the harness. The verifier refuted that part. The harness's 1280x800 is its own fixed test display. The app's 1280x720 is a DPI-scaled fallback window size. They are different facts, and the heights already differ.

The clear colour agrees today, shown by reading. Nothing shows in the app if they drift. In the harness the colour only shows where no window covers the target, so test screenshots could differ. The verifier did not check whether the main window always covers it.

The app copy came from 3b69958 (2026-08-15). The harness copy came from ba0885a (2026-08-22). No user decision was found.

One constant in `ui/app_shell.h` should own it. Both `main.cpp` and the harness already include that header.

*Owner proposed: one constant in src/ui/app_shell.h. Candidates: sweep-ui2-c13.*

#### 271. EngineModel.constants spells key prefixes twice; seconds stored in _ms names

The question is what names the probe uses for the engine constants it reads. `EngineModel.constants` in `tools/ch_probe/engine.py` (lines 156-173) builds its result two ways. The formula keys go through `C.CONST_KEY_PREFIX_NORMAL`, `C.CONST_KEY_PREFIX_PRECISION`, `C.CONST_KEY_DIVISOR` and `C.CONST_KEY_EXPONENT`. The per-side window keys (`'normal_back'`, `'normal_front'`, `'precision_back'`, `'precision_front'`) and `'hitcheck_threshold'` are bare strings that repeat the same prefixes. `test_returns_a_value_for_every_name` in `tests/test_engine.py` (lines 304, 306) spells the prefixes as literals too.

Separately, `Process.verify_targets` in `process.py` (lines 194-196) stores seconds in variables named `back_ms` and `front_ms`. Its docstring quotes 85.0 and 37.5. The check it calls works correctly in seconds.

They agree today; test_engine passes. Only tests read the literal keys. If someone renamed `CONST_KEY_PREFIX_NORMAL`, the literal `'normal_back'` would keep the old prefix. Nothing reaches any screen; this is naming only. `constants.py` should own every key, as its own comment already claims (a code comment, not a decision). No user decision found. Introduced in 283e028 (2026-09-26). The seconds/ms twin of the 85 ms back window has its own finding.

*Owner proposed: tools/ch_probe/constants.py CONST_KEY_* for every key; process.verify_targets renamed to back_s/front_s. Candidates: sweep-probe2-c15.*

#### 272. Three test fakes re-implement Process.resolve's base-plus-offset math

The question is how an RVA (an offset inside the game's DLL) becomes a live memory address. `Process.resolve` in `tools/ch_probe/process.py` (lines 145-148) owns it: module base plus RVA. Three test fakes each write the same sum. They are `FakeProcess.resolve` in `tests/test_engine.py` (lines 62-63), `FakeProc.resolve` in `tests/test_engine_finder.py` (lines 38-39), and `FakeProcess.resolve` in `tests/test_runners.py` (lines 42-43). `test_breakpoint_set_at_resolved_ctor_address` in `test_engine.py` (line 153) computes it inline once more.

All agree today, by reading and by the 73-test run passing. The formula is trivial. Nothing reaches any screen. A drift here would only mean the tests check the fake instead of the real address math. `process.Process` should own it. `tests/test_process.py` (line 44) already builds a real `Process` with fake reader and writer callables, so the other tests can do the same. A related literal, `MODULE_SPAN = 0x4000000`, is repeated in four experiment scripts; that is covered in the live-engine filter finding. No user decision found. Introduced in 283e028 (2026-09-26).

*Owner proposed: tools/ch_probe/process.py Process (tests build a real Process with fake reader/writer). Candidates: sweep-probe2-c16.*

#### 273. Tests restate probe timing values; 3 ms key hold default written twice

The question is where the probe's timing and layout numbers live, and whether tests read them or restate them. Several tests restate production values as bare literals. `FindLiveEngineTest` in `tests/test_engine_finder.py` (lines 77, 88) pins the 0.12 s reread gap and 0.4 s retry pause. Those are unnamed literals in `find_live_engine` (lines 171, 183). `test_all_down_then_all_up` in `tests/test_input_driver.py` (line 179) pins the 3 ms key hold. `WalkEdgesTest` in `tests/test_hit_window_scripts.py` (lines 112-113, 142) repeats walk_edges' 80:92 range, 3 repeats, 120 notes and SongClock's 0.05 s fill cap. `tests/test_engine.py` (lines 198, 249, 322) re-pins 0x2E0, 0x1000 and 0x100 on purpose, as guard tests.

The one real production copy is the 3 ms hold default. It is written in both `InputDriver.press_chord` (`input_driver.py` line 163) and the protocol stub in `interfaces.py` (line 231).

All pairs agree today; 66 tests pass. Nothing reaches any screen. The `FakeDebugger` 2.0 s deadline pins no production value and is dropped. Provenance is mixed: 283e028, 288b07a, 396082a and aa3ad89, all September 2026. No ADR or CONTEXT.md entry sets these values; plan body text mentions some. Named constants should own them. Keeping the guard tests is a user call.

*Owner proposed: named constants in engine_finder, input_driver (referenced by interfaces.py) and walk_edges. Candidates: sweep-probe2-c17.*

#### 274. Scan snapshot path rule is written in bench and again in its test

The question is how a scanned path is written relative to the corpus root for `scan_snapshot.json`. Two copies answer it. The `relify` lambda in `scan_mode` in `tools/bench.cpp` (around line 147) writes the snapshot. `rel_of` in `tests/test_analysis.cpp` (around line 67) rebuilds it for the snapshot test. Both strip the root plus one character, then turn backslashes into forward slashes. The test's own comment names bench's `--dump-rel` as its source.

The two bodies are the same line for line, so they agree on every input today. This was shown by reading, and the snapshot test passes when run. One shared quirk: the strip drops one character even when it is not a separator, so root `C:/in` and path `C:/input/x` give `put/x` in both.

Nothing reaches the screen. If one copy changed, the snapshot test would break or stop checking what the writer produced.

A shared helper in `src/core/strutil` should own the rule, called by both the bench writer and the test. Both copies first appear in git in 21ddf53 (2026-08-18), which only began tracking files that already existed. No decision was found in that commit message, the ADRs or CONTEXT.md.

*Owner proposed: a shared helper in src/core/strutil. Candidates: sweep-tests1-c7.*

#### 275. Test cache key keeps its own list of record-changing settings

The question is which engine settings change a stored analysis. The test cache answers it with its own list. `detail::add_settings` in `tests/corpus_util.h` (around line 94) builds the memo key for `corpus::analyzed()` from cap, depth mode, depth value, ms filter, legacy fills and the rules fingerprint. It covers all six `SearchSettings` fields in `src/search/pather.h` today. No test checks that it stays complete.

This is not a copy of the production key. `Settings::lens` and `Settings::record_key` in `src/app/config.cpp` key on GUI settings, not `SearchSettings`. They leave rules out on purpose (ADR 0014 makes rules a staleness stamp), and the lens stores the ms limit as a whole number.

| Two runs differ only in | test key | production key |
|---|---|---|
| rules | two keys | one key, row reads Stale |
| ms filter 10.0 vs 10.5 | two keys | 10.5 cannot be stored |

These differences are by design, shown by reading. The hazard is the future: a new `SearchSettings` field missing from `add_settings` would let two different analyses share one cache entry. A corpus test could then pass or fail on a stale record. Nothing reaches the screen.

A key function next to `SearchSettings` in `src/search` should own the list. `store::Lens` cannot, because it takes GUI settings. The list came in fddfcff (2026-09-26). Plan Task 17 describes the cache but makes no decision on where the list lives.

*Owner proposed: a key function next to SearchSettings in src/search. Candidates: sweep-tests1-c5.*

#### 276. Tests type layout and timing constants by hand instead of reading them

The question in each case is a production number or formula that four tests type by hand. `tests/test_activation_row_layout.cpp` (around lines 32 to 51) writes 104 and 200 instead of `kRowMeasureX` and `kRowMinBarsX` from `src/ui/activation_row_layout.h`. `tests/test_highway_draw.cpp` (around lines 349 and 361) writes 0.1666666 instead of `targets_secs_light` from `src/render/preview_config.h`, and restates the glow formula from `highway_draw.cpp`. The same file (around lines 223 and 428) writes 1.5005, the fill end plus half a tick. That half tick is `kSpanEndTicks`, hidden in an anonymous namespace in `src/render/track_state.cpp`, so the test cannot read it. `tests/test_app_shell.cpp` (around line 191) recomputes ImGui's whole-number truncation of scaled padding, a vendored-library rule rather than Hydra's.

All four match production today, shown by reading, and the suite passes on them. They act as pins: a changed constant makes them fail loudly. The cost is churn, not silent drift. Nothing reaches the screen.

The production constants should own these values. The tests would read `kRowMeasureX`/`kRowMinBarsX`, `PreviewConfig{}.track.targets_secs_light`, an exported `kSpanEndTicks` and `scaled_style()`. They came in 6d2b1a5 (2026-09-27), ba0885a (2026-08-22), 2a573b4 (2026-09-24) and 69eeb3c (2026-09-27). No decision was found in CONTEXT.md or docs/adr/.

*Owner proposed: the production constants (kRowMeasureX/kRowMinBarsX, PreviewConfig, kSpanEndTicks, scaled_style). Candidates: sweep-tests1-c12.*

#### 277. Test helpers are copied between test files

Two pieces of test plumbing are copied between files. The first is a frequency check. `estimate_freq_hz` in `tests/test_audio_decode.cpp` (around line 73) and in `tests/test_audio_mixer.cpp` (around line 54) count zero crossings over the middle half of a buffer. The mixer copy takes a channel argument; with channel 0 they are the same line for line. `read_fixture` is identical in both files. The second is the loop "first corpus chart that analyzes with a non-empty song and at least one path". It appears as `fill_store` in `tests/test_dm_report.cpp` (around line 45), `sample_chart` in `tests/test_fill_report.cpp` (around line 44), and four times in `tests/test_report.cpp`. `tests/test_path_view.cpp` has a near-copy with a different goal.

At equal settings the copies pick the same chart and return the same frequency, shown by reading. `test_report.cpp` uses depth 10, so its loop could stop on a different chart; that is a settings difference, not drift. No user-facing rule is involved and nothing reaches the screen.

The corpus loop belongs in `tests/corpus_util.h`, which already holds `chart_paths` and `first_chart_with_suffix`. The audio helpers belong in a small shared test header. Who introduced them was not checked. The verifier did not look for a decision beyond the test files.

*Owner proposed: tests/corpus_util.h for the corpus loop; a shared tests audio helper header. Candidates: sweep-tests1-c11.*

#### 278. Mixer test oracle re-writes the stem summation

The question is how Preview stems are added together. Production has one owner: `add_into` in `src/audio/mixer.cpp` (around line 62). It grows the mix with zeros when a stem is longer, then adds sample by sample. `mix_stems` and `decode_and_mix` both call it. The test oracle `reference_mix` in `tests/test_audio_mixer.cpp` (around line 72) writes the summation again. It still converts each stem through `mix_stems`, so only the adding is copied.

The two agree on every input checked, shown by reading and by a run. No stems give an empty buffer. Stems of 3 and 2 frames give the sums for two frames, then the longer stem's last frame. A stem with zero channels adds nothing. Both start from 0.0 and add in the same order, so the float bits match. The existing build ran 8 mixer cases and 83 assertions green.

Nothing reaches the screen; this is a test-only oracle. The copy exists on purpose, as a bit-for-bit guard for the one-stem-at-a-time rewrite. `add_into` owns the rule; if the guard stays as a deliberate oracle, nothing needs to move. It came in b868de9 (2026-09-26). Plan Task 8 in `docs/superpowers/plans/2026-09-26-codebase-audit-fixes.md` sets the bit-identical goal and includes `reference_mix`. The plan header says it covers "the seven changes you approved". No ADR or CONTEXT.md entry covers the oracle.

*Owner proposed: add_into in src/audio/mixer.cpp. Candidates: sweep-tests1-c13.*

#### 279. Three tests restate production layout and SP formulas instead of calling them

The question is whether tests check production values by calling the production function, or by working the formula out again.

Three tests work it out again. In `tests/test_preview_renderer.cpp` (lines 178 and 186) the expected track height is 117, typed by hand as 100 times 1.1666. The owner is `render::track_height` in `src/render/highway_draw.cpp` (lines 63-68). In `tests/test_overlay_layout.cpp` (line 174) the expected room for the next-activation box copies the body of `bottom_left_room` in `src/render/overlay_layout.cpp` (lines 67-69), minus its guard for tiny heights. In `tests/test_preview_golden.cpp` (around lines 236 and 255), a dev-aid dump decides "note inside an SP phrase" with its own start-to-end check. Production decides SP highlighting in `src/render/track_state.cpp` (lines 72-79) with an extended end. They agree for notes.

Four other sites the finder listed are not copies. `2 * 13` in `tests/test_path_codec.cpp` deliberately pins the wire format. The .srb metadata builder in `tests/test_preview_source.cpp` has no production encoder to call. `ms * 48.0` in `tests/test_preview_transport.cpp` mirrors the fixture's own sample rate. The literal 85.0 in `tests/test_path_view.cpp` is a test input, equal to `kDefaultHitWindowMs` today.

All three copies agree with production today. The user never sees them. If production changed, these tests would keep passing against the old formula or fail for the wrong reason.

Each test should call its production owner. No rule about this exists in CONTEXT.md or docs/adr/. The 117 literal came in ba0885a (2026-08-22); the overlay copy in 085555c (2026-09-27).

*Owner proposed: render::track_height and render::bottom_left_room, called directly by their tests; a scene helper for the golden dump if the user wants one. Candidates: sweep-tests2-c12.*

#### 280. Unit tests retype the GUI tests' analysis settings and expected Paths text

The question is which analysis settings the Burnout reference results were made with: score mode, depth, ms limit and so on.

The GUI tests get theirs from `tests/ui/uitest_harness.cpp` (line 273), which writes depth 2 to a scratch settings file. Everything else comes from the `app::Settings` defaults in `src/app/config.h`. That works out to 2 scores, 10 ms, cap 4, Pro Drums, 2x Bass, Expert. Two unit tests type those same settings by hand: `burnout()` in `tests/test_path_view.cpp` (lines 46-58) and the Burnout case in `tests/test_stars.cpp` (lines 170-173). `test_path_view.cpp` (lines 866-889) also repeats the Paths-list strings that `tests/ui/uitest_paths.cpp` (lines 80-88) checks, such as "Within 2 scores".

They agree today. The verifier showed this by reading. Set the uitest depth to 3 and only the GUI expectations would move; the unit tests would still expect "Within 2 scores". That was not run.

The codec, preview-controller and golden fixtures also type settings, but they make no GUI claim, so they are independent inputs. Separately, `SearchSettings` in `src/search/pather.h` and `app::Settings` each keep their own default depth of 4.

The user never sees this. One shared helper should build the uitest settings. The `app::Settings` defaults cannot be the owner, because the GUI tests use depth 2 and the default is 4. No decision was found in docs/adr/, CONTEXT.md or the commit messages. `burnout()` came in 577ddc2 (2026-09-27); the uitest depth line in ba0885a (2026-08-22).

*Owner proposed: One shared test helper that builds the uitest scratch settings and runs them through Settings::to_analysis_settings (not the app::Settings defaults). Candidates: sweep-tests2-c13.*

#### 281. Score-box test recomputes the withheld-solo total instead of reading the replay

The question: what running total does the Preview score box show while a solo's bonus is still held back? `replay_path` in `src/core/replay.cpp` (lines 151-157) owns it. It adds each solo chord's points to `solo_pending`, clears that on the solo's last chord, and sets `cum_onscreen_total = cum.total() - solo_pending`. `build_preview_scene` in `src/app/preview_view.cpp` (line 213) only copies that field. The test "score box: a solo's bonus lands on its last note" in `tests/test_preview_view.cpp` (lines 1033-1034) works the expected value out itself: the total minus this chord's own solo points.

That shortcut matches only because the fixture's chord 1 opens a two-chord solo. On the second chord of a three-chord solo, the owner would hold back two chords' points and the shortcut one. No test evaluates that case, so nothing disagrees today. Shown by reading and by a passing hydra_tests run.

Nothing shows on screen. If `replay_path` changed, the scene would follow it and this test would fail loudly.

`replay_path`'s `cum_onscreen_total` should own it. The test can compare `scene.score.steps[i].total` against `r.chords[i].cum_onscreen_total`, keeping one hand-computed number if a fixed oracle is wanted. The test literal came in 545d420 (2026-09-25).

*Owner proposed: replay_path's cum_onscreen_total in src/core/replay.cpp. Candidates: sweep-tests3-c1.*

#### 282. Test helper retypes the Path-to-ReplayScore field pairing in reverse

The question: which Path score field holds which ReplayScore category? `score_of` in `src/core/replay.cpp` (lines 167-176) owns the pairing from Path to ReplayScore: six pairs, from `score_base` to base through `score_ghosts` to ghost. The test helper `priced_path` in `tests/test_preview_view.cpp` (lines 224-229) types the same six pairs again in reverse. No production reverse map exists.

They agree on all six pairs today. Shown by reading pair by pair; the related hydra_tests cases passed.

Some places look similar but answer other questions. `kReplayScoreFields` in `src/core/replay.h` owns category names and order, not the Path pairing. `Engine::emit_path` in `src/search/engine.cpp` maps internal engine slots. The key list in `tests/test_replay.cpp` (line 361) pins the dump's JSON names for outside readers on purpose. Each of these would still need an edit for a seventh category.

Nothing shows on screen. A new or reordered category would need the test helper edited by hand.

`score_of` should own it, with a reverse helper beside it so the test stops typing the map. `priced_path` came in 545d420 (2026-09-25).

*Owner proposed: score_of in src/core/replay.cpp, plus a reverse helper beside it. Candidates: sweep-tests3-c13.*

#### 283. Replay warning test picks fixtures with its own 500 ms window checks

The question: is a chord within the 500 ms squeeze window (`kSqueezeWindowMs`) of a phrase note? Production answers it in `ScoreGraph::is_recent_to_head` in `src/search/graph.cpp` (line 280), again inside `ScoreGraph::build` (line 131), and in `sqout_candidates` in `src/core/replay.cpp` (lines 29-37). All use a strict "under 500 ms" edge. The test "a window ending on a phrase note with no offset is flagged" in `tests/test_replay.cpp` (lines 405-409) picks its fixture chords with its own checks. `just_after` is under 500 ms after a phrase note. `long_after` is over 500 ms after the last earlier phrase note.

| Case | Test | `sqout_candidates` |
|---|---|---|
| gap 499.9 ms | just_after | in |
| gap 500.0 ms | neither group | out |
| chord at 1000, phrase notes at 400 and 1200 | long_after (600 ms gap) | still lists the 1200 chord (200 ms) |

`just_after` matches the owners' edge. `long_after` has a different shape: it looks only backward, while `sqout_candidates` looks both ways. That only affects which fixtures get picked. Shown by reading and a passing run.

Nothing shows on screen. If the rule changed, the test would pick fixtures by the old rule and probably fail.

One core helper meaning "within `kSqueezeWindowMs`" should own it, used by both the graph and the replay. The test could pick fixtures through it. The test copy came in 798026c (2026-09-24).

*Owner proposed: one core helper meaning within kSqueezeWindowMs, used by sqout_candidates and ScoreGraph::is_recent_to_head. Candidates: sweep-tests3-c2.*

#### 284. run_search test spells out the all-0 search options itself

The question: which search options make the all-0 search? `search_allzero` in `src/search/pather.cpp` (lines 75-78) owns the recipe: `ms_filter = 0.0`, `no_skips = true`, `hard_ms_filter = true`, everything else default. The test "run_search: EngineOptions carries each knob to the engine" in `tests/test_search.cpp` (lines 576-579) spells the same three fields out itself. It then compares `run_search` against `search_allzero` path by path.

Both hold the same three values today. Shown by reading and a passing hydra_tests run.

Nothing shows on screen. Because the test compares directly against `search_allzero`, a change to the recipe would fail the test loudly instead of drifting.

`search_allzero` should own it and expose its options builder (for example an `allzero_options()` function), so the test and the owner share one recipe. Both copies came in 124f0f5 (2026-09-26, "Pass run_search its knobs as one EngineOptions value").

*Owner proposed: search_allzero in src/search/pather.cpp (expose its options builder). Candidates: sweep-tests3-c4.*

#### 285. Ghost-kick test re-inlines check_invariants' tick and code checks

The question: do parsed chords keep strictly increasing ticks and round-trip their chord code? `check_invariants` in `tests/test_song.cpp` (line 30) answers it. It also checks that every fill has a positive activation length. The test "mid: Won't Get Fooled Again (O) has 36 ghost kicks" (lines 425-434) retypes the first two checks with the identical loop instead of calling it. Its comment (lines 421-424) says why: this chart has a fill starting on its own last note, so its activation length is 0.

| Input | `check_invariants` | inline copy |
|---|---|---|
| ticks 0, 480 | pass | pass |
| ticks 0, 0 | fail | fail |
| empty code | fail | fail |
| activation length 0 | fail | pass (on purpose) |

They agree on ticks and codes today. Shown by reading.

Nothing shows on screen; this is test-only. If `check_invariants` gained a new tick or code check, this test would miss it.

`check_invariants` should own it, with an option to skip the fill check. The copy came in 396082a (2026-09-21). No user decision covers it in docs/adr/ or CONTEXT.md; the code comment is the only reason given.

*Owner proposed: check_invariants in tests/test_song.cpp, with an option to skip the fill check. Candidates: sweep-tests3-c7.*

#### 286. test_song.cpp keeps its own MIDI writer beside tests/midi_util.h

The question: how does a test build a MIDI file's bytes? `testmidi::smf` in `tests/midi_util.h` (line 18) is the shared writer: one MThd header plus one MTrk chunk. `tests/test_song.cpp` keeps its own helpers, `put_meta` (line 289), `put_bytes` (line 298) and `put_track` (line 302). The test ".mid: EVENTS text metas become practice sections" uses them to build a three-track file. `put_track` repeats smf's MTrk tag and big-endian length. `put_meta` with delta 0 writes the same bytes as `testmidi::track_name`, `text_event` and `end_of_track`. Line 334 hand-types a second MThd header. `put_varlen` (line 276) is not a copy, since the shared header has no varlen writer.

They agree on every input used today. Shown by reading. One theoretical gap exists: the shared `track_name` and `text_event` write the length as one raw byte, so a 128-byte text would come out wrong there and right in `put_meta`. No test uses one.

Nothing shows on screen; both are test helpers.

`tests/midi_util.h` should own it, extended with a varlen delta and a multi-track variant. The helpers came in aab5ff1 (2026-08-31), before the shared header existed. 396082a (2026-09-21) created `midi_util.h` "so test_song can use it" but left these helpers in place.

*Owner proposed: tests/midi_util.h, extended with varlen deltas and a multi-track variant. Candidates: sweep-tests3-c6.*

#### 287. Per-process temp-path recipe copied across about a dozen test files

The question: how does a test build a temp file path unique to its process? About a dozen test files each build one from `GetTempPathW`, a tag and the process id. Examples are `temp_path` in `tests/test_song_panel_state.cpp` (line 29), `temp_db` in `tests/test_store.cpp` (line 400), `sng_fixture_path` in `tests/test_sng.cpp` (line 83), and an inline copy in `tests/test_report.cpp` (lines 296-302) whose comment says it copies `temp_db`. More sit in `test_config.cpp`, `test_app_state.cpp`, `test_analysis.cpp`, `test_cli.cpp`, `test_preview_controller.cpp`, `test_preview_source.cpp`, `test_utf8_paths.cpp` and `tests/ui/uitest_harness.cpp`. No shared helper exists. `fixture_dir` in `tests/test_srb.cpp` (lines 48-60) also hand-rolls the wide-to-UTF-8 conversion, even though it includes `core/winstr.h`, which has `wide_to_utf8`.

They agree today for any valid path. Shown by reading; nothing was run. Tag order and extensions differ by design. The only real gap is a failed conversion: `test_srb` would resize to a huge size and throw, while `wide_to_utf8` returns an empty string.

Nothing shows on screen; this is test plumbing.

A shared header such as `tests/temp_util.h`, next to `tests/corpus_util.h`, should own one `temp_path(tag, ext)` built on `wide_to_utf8`. The srb copy came in 21ddf53 (2026-08-18); the report copy in f5320dd (2026-09-03).

*Owner proposed: a shared test header such as tests/temp_util.h built on core/winstr wide_to_utf8. Candidates: sweep-tests3-c17.*

#### 288. Two WCAG contrast formulas in tests with different thresholds

The question is: what is the WCAG contrast ratio of two colours? WCAG is the web accessibility standard that sets minimum text contrast.

Two test files each code the formula by hand. `channel` and `contrast` in `tests/test_theme.cpp` (lines 17 and 25) use a linearisation threshold of 0.04045. `luminance` and `contrast` in `tests/test_report.cpp` (lines 588 and 596) use 0.03928. No production code computes contrast.

| Channel value | test_theme | test_report |
|---|---|---|
| byte 10 (0.039216) | straight line | straight line |
| byte 11 (0.043137) | power curve | power curve |
| 0.04 (not a byte) | 0.0030960 | 0.0030955 |

The finder claimed they disagree today. The verifier refuted that with a run. Every 8-bit value takes the same branch in both. Only fractional values between the thresholds split them, and no test feeds one. The largest gap there is about 7.6e-7.

Nothing is visible. These are test-only checks. Both copies came on 2026-09-27 (5457d7d and ae2a4a6), copied from the plan in f1c6a3d. The plan cites WCAG 2.2 but chose no threshold.

One shared test helper using WCAG 2.2's 0.04045 should own the formula.

*Owner proposed: one shared test helper (e.g. tests/wcag_util.h) using WCAG 2.2's 0.04045. Candidates: sweep-tests4-c4.*

#### 289. Analyze button label rule retyped in the GUI test helper

The question is: what does the song panel's analyze button say?

`render_headline` in `src/ui/details_panel.cpp` (around line 84) says "Analyze this song" before any result and "Re-analyze" otherwise. Line 86 types "Analyze this song" again to size the button. `analyze_button_ref` in `tests/ui/uitest_harness.cpp` (around line 495) has the same condition and both strings.

Both read the same `viewed.status` field, so they agree for all three statuses today. Not analyzed gives "Analyze this song". Stale and Ready give "Re-analyze". Shown by reading.

Nothing changes in the app. If the wording or condition changed in the panel only, the GUI tests would click a widget that no longer exists.

The code came in 99cbda2 (2026-09-27). The redesign plan fixes the two labels as a label contract, but that does not cover keeping two copies of the rule.

`details_panel` should own one label function. The headline, the width sample and the test helper would all call it.

*Owner proposed: details_panel (an analyze_button_label(RecordStatus) in src/ui/details_panel.cpp). Candidates: sweep-tests4-c6.*

#### 290. Analyze search button label rebuilt in tests and twice in the toolbar

The question is: what does the batch button say while a search is typed?

`render_actions_row` in `src/ui/library_toolbar.cpp` (around line 77) builds "Analyze search (N)..." with `group_thousands` of the match count. It types the same wrapper again at line 85 for the width sample. `batch_search` and `test_report_buttons` in `tests/ui/uitest_batch_reports.cpp` (lines 34 and 251) each rebuild it by hand.

All copies wrap the same count, so they agree today. One match gives "Analyze search (1)..." everywhere. 1,234 matches gives "Analyze search (1,234)..." everywhere. Shown by reading.

Nothing changes in the app. A wording change in the toolbar alone would break the tests' clicks.

The toolbar wording dates back to the C++ port. The test copies came in 3f91a0e and 842fd76 (2026-09-26/27). No user decision covers the copies.

The toolbar should own one label function that takes the search state and count. The label, the width sample and the tests would call it.

*Owner proposed: a label function in src/ui/library_toolbar.cpp. Candidates: sweep-tests4-c7.*

#### 291. Backend table ID format rebuilt in the GUI test

The question is: what internal ID does an activation's backend table get? ImGui, the GUI library, finds widgets by these ID strings.

`render_backend_table` in `src/ui/paths_tab.cpp` (around line 324) builds "##backends<number>_<w1>_<w2>_<w3>" from the activation number and three column widths. `backend_table` in `tests/ui/uitest_paths.cpp` (around line 364) builds the same string from the same parts in the same order.

They agree today. Activation 1 with widths 48, 40 and 56 gives "##backends1_48_40_56" in both. Shown by reading, and the paths-backend-fit test passes.

Nothing is visible. If the format changed in `paths_tab` only, the test lookup would find nothing and fail.

`paths_tab` should export the ID builder, and the test should call it.

*Owner proposed: paths_tab (export the backend table id builder from src/ui/paths_tab.cpp). Candidates: sweep-tests4-c11.*

#### 292. Layout tests type their own longest activation badge

The question is: what is the longest text an activation badge can show?

`activation_badge` in `src/app/path_view.cpp` (around line 37) owns the wording. It says "early fill", "squeeze in" or "squeeze out", then the milliseconds. `test_paths_row_layout` in `tests/ui/uitest_paths.cpp` (line 450) types "early fill 999 ms" and "squeeze out 999 ms" as the longest badges. `tests/test_activation_row_layout.cpp` (line 56) hard-codes the same case as 18 characters.

They match the current format exactly, and paths-row-layout passes. Shown by reading and by a run.

Nothing is visible today. If the wording grows, these tests keep checking the old longest badge. Real outputs can carry a minus sign, such as "early fill -20 ms". So a negative three-digit value may already be longer than the tests' case. The verifier did not check whether such a value can occur.

`activation_badge`, or a sibling that returns the longest possible badge, should own this. The tests would build their case from it.

*Owner proposed: activation_badge in src/app/path_view.cpp (or a sibling giving the longest badge). Candidates: sweep-tests4-c12.*

#### 293. WARP test device and texture read-back written twice

The question is: how do the tests make a headless graphics device and read pixels back? WARP is Windows' software renderer, used so tests need no graphics card.

`warp::make_device` in `tests/warp_util.h` (line 16) owns the device recipe. `Harness::init` in `tests/ui/uitest_harness.cpp` (around line 167) repeats the same call argument for argument. It could just call `make_device`.

The read-back half is only partly a copy. `warp::read_pixels` (line 26) reads a whole image through a shader view. `capture_pixels` in `uitest_harness.cpp` (around line 34) repeats the staging-texture setup and copy. But it reads a clipped sub-rectangle from a texture with no shader view. So `read_pixels` cannot replace it as written.

They agree today: same driver, same feature level, same bytes for a full-size read. Shown by reading.

Nothing is visible; this is test plumbing.

`tests/warp_util.h` should own the device recipe. If the read-back is to be shared, the staging step needs its own helper there.

*Owner proposed: tests/warp_util.h. Candidates: sweep-tests4-c13.*

#### 294. GUI test name matching written twice in hydra_uitest

The question is: which GUI tests does a `--test` name select?

`Harness::queue` in `tests/ui/uitest_harness.cpp` (around line 140) skips the internal "script" test and takes "all" or an exact name. `run_parallel` in `tests/ui/uitest_main.cpp` (around line 88) has the same rule. It repeats it once more to report names that matched nothing.

| Name given | queue | run_parallel |
|---|---|---|
| all | every test but script | every test but script |
| scan | scan | scan |
| script | nothing | nothing |
| Scan | nothing | nothing |

The name rule agrees today. Shown by reading. The only difference is a step before it. `queue` first runs an existing file of that name as a script, and `run_parallel` never does. Its error message says that is on purpose.

Nothing is visible.

One select function in `uitest_harness.cpp` should own the name rule. Both callers would use it, and the file check stays where it is.

*Owner proposed: one select function in tests/ui/uitest_harness.cpp. Candidates: sweep-tests4-c14.*

#### 295. ImGui text size formula retyped three times in GUI tests

The question is: at what pixel size does ImGui draw text? ImGui is the GUI library.

ImGui owns the answer through `GetFontSize`, and the app uses it. The tests work it out themselves as FontSizeBase times FontScaleMain times FontScaleDpi. `text_w` in `tests/ui/uitest_paths.cpp` (line 333) does it once. `test_preview_path_picker` in `tests/ui/uitest_preview.cpp` does it twice, at lines 519 and 545, with the mono font.

All three are the same expression, so they agree. They would differ from ImGui only inside a window with its own font scale, which none of these set. Shown by reading.

Nothing is visible.

`text_w` sits in a separate file, so the preview test cannot reach it. One helper in `uitest_harness`, taking an optional font, should own this for all GUI tests.

*Owner proposed: one text-width helper in tests/ui/uitest_harness taking an optional font. Candidates: sweep-tests4-c15.*

#### 296. GUI tests type 4 instead of the Clone Hero SP cap constant

The question is: which Star Power cap does the leaderboard comparison need?

The rule lives once, in `render_actions_row` in `src/ui/library_toolbar.cpp` (around line 96). The Compare button is off unless the difficulty is Expert, the cap equals `kCloneHeroSpCap`, and 1.0 fills is off. `kCloneHeroSpCap` is 4, in `src/core/model.h` (line 32).

Two GUI tests in `tests/ui/uitest_batch_reports.cpp` type the 4 by hand. `test_compare_disabled` does it at line 434, and `test_dm_compare_flow` at line 156. The 6 and 8 they also set are deliberate off values, not copies. The finder said the tests cover only two of the three conditions. That is a coverage remark, not a second rule: `test_legacy_fills` in `tests/ui/uitest_details.cpp` covers the fills condition.

They agree today, and compare-disabled passes.

Nothing is visible. The literal came in 3f91a0e (2026-09-26).

The tests should read `kCloneHeroSpCap`.

*Owner proposed: kCloneHeroSpCap (src/core/model.h). Candidates: sweep-tests4-c18.*

#### 297. Column-order test types the Best path column index as 4

The question is: which column of the library table is Best path, and how many columns are there?

`src/ui/library_table.cpp` already owns both. `kColumnBestPath` is 4 (around line 36), and `BeginTable` declares 5 columns (line 273). `test_library_column_order` in `tests/ui/uitest_library.cpp` types `best = 4` (line 470) and the count 5 in its INI string (line 473).

The owner sits in an anonymous namespace, so the test cannot reach it. Inside the app, the setup order around lines 276-290 must also match those constants, and nothing enforces that. `LibrarySort` in `src/ui/library_model.h` repeats the same numbers as column IDs.

All copies say 4 and 5 today, and library-column-order passes.

Nothing is visible. The test came in cfa926f (2026-09-27).

The existing `kColumn*` constants, plus a column count, should be exposed beside `LibrarySort`. The test would read them.

*Owner proposed: the kColumn* constants in src/ui/library_table.cpp, exposed beside LibrarySort in library_model.h. Candidates: sweep-tests4-c19.*

#### 298. Database schema number is written every open but never read

The question: which schema is this database file at, and which upgrades still need to run? The real answer comes from column checks. `RecordStore::RecordStore` in `src/store/record_store.cpp` asks `has_column` for `count_version`, `sig`, `length_ms` and `stars` (around lines 599-620). `add_fill_rule_column` (around line 631) checks `legacy_fills` for the schema 2 to 3 step.

A second statement of the level sits beside them. The constructor writes `PRAGMA user_version = 3` on every open (around line 627). That is SQLite's spare number for a file version. No app code reads it back. Only two checks in `tests/test_store.cpp` do. The header comment in `src/store/record_store.h` says "schema user_version 3" as if the number were the gate.

So it cannot disagree with any decision today. Its only cost is that a future upgrade must bump the 3 by hand, and only those two tests would notice. Nothing on screen or in the CLI shows it.

History: older upgrade steps wrote 1 and 2 themselves until 3e5d634 (2026-09-26). Commit 1f3efdf (2026-09-29) bumped the final write to 3.

The column checks should stay the one owner. The user can pick between two fixes. One drops the write and the header wording. The other makes the number the real gate and moves it into `src/store/stored_versions.h`.

*Owner proposed: The has_column probes in RecordStore::RecordStore (src/store/record_store.cpp); user to choose drop vs gate. Candidates: R4-A6-3.*

#### 299. The rescan-cache column is checked twice; the second check is dead

The question: does this database's charts table have the `sig` column? That column holds the rescan-cache fingerprint, the size-and-time key for each chart file.

Two places ask. The constructor `RecordStore::RecordStore` in `src/store/record_store.cpp` (around line 604) checks with `has_column` and adds the column if it is missing. It does this on every open, before anything else can run. `RecordStore::chart_library_cache` (around line 1655) asks again with its own `pragma_table_info` query. It returns an empty cache if the column is missing.

They always agree. An old database gets the column from the constructor, so the second check says yes. Its old rows with an empty `sig` are already skipped by a check just below. A new database also says yes. Nothing drops or rebuilds the charts table. So the second check can never fail.

The second check dates from 21ddf53 (2026-08-18) or earlier. It became dead when 3e5d634 (2026-09-26) added the constructor's column fix. The user sees nothing either way.

The constructor's check-and-add should own it. The probe and its comment in `chart_library_cache` can be deleted.

*Owner proposed: RecordStore::RecordStore has_column + ALTER (src/store/record_store.cpp). Candidates: R3-A6-5.*

#### 300. Hand-built test Songs mark SP phrase ends without sp_phrase_start

The question is which fields an SP phrase-end note carries in a parsed Song. The parser answers it in `mark_sp_phrase_end` in `src/parse/song.cpp` (lines 138-141). It sets `flag_sp` and `sp_phrase_start` together, and nothing else in `src/` sets `flag_sp`. Several tests hand-build Songs on the same grid (192 ticks per beat, 768 per measure, 120 BPM). `build_tail_song` in `tests/test_search.cpp` (line 391), `song_with` in `tests/test_replay.cpp` (line 479) and an inline Song at `test_replay.cpp` line 229 set `flag_sp` only. `make_overfill_song` in `tests/test_preview_view.cpp` (line 174) sets both, and its comment says it was rebuilt for that reason.

| phrase end at tick 768 | flag_sp | sp_phrase_start |
|---|---|---|
| parser | true | set |
| make_overfill_song | true | 768 |
| build_tail_song, song_with | true | none |

So those fixtures are Songs the parser cannot produce. No running check sees the gap, shown by reading. `song.h` documents the field as display-only, and its one reader is `preview_view.cpp` line 250. No test on these fixtures calls the Preview builder. This is a test-hygiene note: no production rule is re-derived here.

Nothing shows on screen. A future Preview test built on these fixtures would see a phrase with no start.

One shared test Song builder should own the phrase marker, setting both fields the way `mark_sp_phrase_end` does. `tests/midi_util.h` is the precedent for a shared test builder.

*Owner proposed: tests: one shared test Song builder whose phrase marker mirrors mark_sp_phrase_end. Candidates: R3-A8-6.*

#### 301. test_search retypes the record byte-equality check that record_bytes.h owns

The question is whether two records store the same bytes through the store's own writer. `record_bytes` in `tests/record_bytes.h` (line 15) owns this. It runs `flatten_record`, then appends every node payload to the structure blob, giving one byte run. The tests in `test_path_codec.cpp` and `test_store.cpp` call it. The case "a 4-bar graph built at the song's phrase count stores the same paths" in `tests/test_search.cpp` (lines 645-650) retypes it instead. It flattens both records and compares the structure, the node count and each payload in a loop.

They agree today, shown by reading. The structure blob holds each node's hash inline, so equal structures mean equal payloads in both copies. The only theoretical split runs the other way: two different structures whose joined bytes happen to line up. `record_bytes` would call them the same; the hand loop would not. No real record can do that, because the structure starts with a fixed format stamp and fingerprint and carries counts.

Nothing shows on screen. This is test code. The risk is upkeep: if the `record_bytes` rule changes, this one test keeps checking the old rule.

The hand loop came in 0bb26ba (2026-09-26). `record_bytes.h` arrived an hour later in 711ba62, which moved the other tests over but missed this one. `record_bytes` should own it, and the test should compare `record_bytes(tall) == record_bytes(built)`.

*Owner proposed: tests/record_bytes.h record_bytes. Candidates: R6-A8-2.*

## Undecided assumptions (51)

These are thresholds, cutoffs, fallbacks and special cases with no decision from you on record. Only an ADR, CONTEXT.md or a quote from you counts as a decision; a code comment does not.

### Visibility 1: wrong or contradictory on screen today

#### 302. Dynamics tab percentages round down, so shares read up to one point low

The question is how a share becomes a whole-number percent on screen.

`render_dynamics_panel` in `src/ui/dynamics_tab.cpp` answers it twice, both times by truncating. Truncating means dropping the decimals instead of rounding. The "2x kicks" line (line 154) and the "Dynamic notes" line (line 205) both use `static_cast<int>(100.0 * part / total)`. The other percent displays in the app round to nearest. The analyze progress overlay in `src/ui/details_panel.cpp` (around line 183) and the Preview loading overlay in `src/ui/preview_tab.cpp` (around line 223) use `%.0f%%`. Those two are progress bars, so they measure something different, but they show the app has no single rule. `src/app/display_format.h` has no shared percent formatter.

| Share | Dynamics tab (truncated) | Rounded |
|---|---|---|
| 1 of 201 (0.50%) | 0% | 0% |
| 3 of 201 (1.49%) | 1% | 1% |
| 12 of 453 (2.65%) | 2% | 3% |
| 49.9% | 49% | 50% |
| 999 of 1000 (99.9%) | 99% | 100% |

They disagree today, shown by reading the code. On "Jamie All Over" Expert, 12 of 453 kicks are 2x, which is 2.65%, and the tab shows 2%. That count came from a scratch MIDI count, not from the GUI.

On screen, every Dynamics percentage can read up to one point low. A near-total share never shows 100% until it is exact. A chart with a handful of ghost notes or 2x kicks can show "(0%)" next to a nonzero count.

One percent formatter in `display_format.h` should own the choice, so every screen rounds the same way.

This came in with ec3ade9 (2026-09-21, "Count ghosts and accents per pad in a Dynamics tab") and moved to its own file in 2facc04 (2026-09-27). The user did not ask for truncation. No decision was found in `docs/adr/`, `CONTEXT.md`, the plans, the ec3ade9 commit message, or `docs/UserGuide.md` line 168, which describes the line but not its rounding.

*Owner proposed: src/app/display_format.h (one shared percent formatter). Candidates: display-10, screenA-21.*

### Visibility 2: visible in an edge case, in docs or in CLI text

#### 303. Nobody chose the 1 ms impact gate or the 0.005 scale tolerance

The question is when a frontend timing-scale change is worth showing in Activation details. A timing scale (also called a ratio) is how much a frontend hit error moves the SP end. Two hand-picked thresholds answer it, and no user decision backs either one.

The first threshold is `kTransferImpactMs = 1.0`, in `src/core/squeeze_rating.h` at line 87. It is defined once, so it has no copies to drift apart. `src/core/squeeze_rating.cpp` uses it in three places. Line 75 is inside `transfer_is_material`, which decides whether a scale matters to a row or a SqIn/SqOut note. Lines 135 and 166 decide whether a backend row or a phrase note gets an "eff." figure. All three use a strict "more than 1 ms" test. A scale that moves a gap by 0.9 ms changes nothing on screen, unless the gap is already past the combined budget. A scale that moves it by 1.1 ms makes the row material and adds an eff. figure.

The second threshold is 0.005, in `build_activations` in `src/app/path_view.cpp` at lines 227 and 228. A scale "shows" when it is at least 0.005 away from 1. Two scales count as "same" when they are less than 0.005 apart. The comment says 0.005 is half the last digit of `%.2f`, so it is meant to stand in for "prints differently".

The two thresholds work together on screen. The frontend timing-scale line ("Frontend timing scales x1.01 (late) at the SP end") appears whenever a scale shows, by the 0.005 rule alone. The line turns orange only when a shown scale is also material by the 1 ms rule (lines 256 to 259). The eff. figures follow the 1 ms rule alone.

The `shows` test matches `%.2f` well enough. The `same` test does not. It compares the raw numbers, but what the user sees is the rounded ones. It decides whether the SqIn's own clause ("at the SqIn's SP end") is printed next to the main one.

| pre scale | post scale | `same` says | `%.2f` prints | result |
|---|---|---|---|---|
| 1.012 | 1.016 | same | x1.01 vs x1.02 | SqIn clause hidden though it would read differently |
| 1.012 | 1.013 | same | x1.01 vs x1.01 | correct |
| 1.0051 | 1.0149 | differs | x1.01 vs x1.01 | SqIn clause printed with the same number twice |

I checked these rows with a short Python run of the same comparisons and `%.2f`. I did not run them through the app. So today the two rules disagree near the rounding edges. On a chart with a SqIn whose pre and post scales land like the first row, the user loses a clause that would have shown a different number. In the third row, the user sees two clauses that look identical.

The 1 ms gate came in with dea4228 (2026-08-19, "Transfer-aware squeeze display: two ends, joint constraint, impact gating"). That commit replaced an older "|r - 1| > 0.05" rule (a 5% scale gate) with the 1 ms shift. Claude co-wrote it, and its message quotes no user. The 0.005 tolerance came in with ba0885a (2026-08-22). It was last edited in 8d4d9e0 (2026-09-29, "Paths tab: show every frontend timing multiplier, early first"). One earlier candidate (screenA-15) named the wrong line and commit for the 1 ms gate; the verifier corrected both. The verifier looked for a user decision in docs/adr, CONTEXT.md and the 2026-09-24 plan decision list. It also checked the messages of dea4228, a4bbad6, 4f5fcc1, ba0885a and 8d4d9e0. It found none. Both numbers look like Claude's choices, and the user never asked for either value.

`core/squeeze_rating` is already the right home for the 1 ms gate, because it is the one module that rates squeezes. What is missing is a recorded user decision on the 1 ms value. The 0.005 tolerance should come from the formatter that prints `%.2f`. "Same" should mean "prints the same", decided by comparing the rounded text, not by a second number that only approximates it.

*Owner proposed: core/squeeze_rating (kTransferImpactMs); the %.2f scale formatter in app/path_view (0.005 tolerance). Candidates: screenA-15, squeeze-10, tempo-31.*

#### 304. A note exactly 3.0 ms after SP end counts as uncounted, with no decision

The question: is a backend note sitting exactly on the leeway edge still scored? The leeway is the small grace window after the SP end.

One place answers it. `counted_without_squeeze` in `src/core/backend_value.h` (lines 34-36) returns true when `paid_by_sp_walk(off)` or `off < leeway` (strict). The comment on `Rules::backend_leeway_ms` in `src/core/rules.h` (line 27) says a note "this close" still scores, without saying which way the edge falls.

| Offset after SP end | Counted? | Label |
|---|---|---|
| 0.0 ms | yes (walk pays it) | Standard |
| 2.999 ms | yes | Standard |
| 3.0 ms | no | Hard (uncounted) |

The inclusive edge at the SP end itself is not an assumption. It follows the engine: `ScoreGraph::build` in `src/search/graph.cpp` (lines 183-185) handles the deactivation after the chord at D is stored.

On screen, a row at exactly +3.0 ms shows 0 points and "Hard (uncounted)". One at +2.999 ms shows full points and "Standard".

The strict `<` first appeared in 3b69958 (2026-08-15) and was carried into `counted_without_squeeze` in 706782b (2026-09-24). The 3 ms value is the user's (decisions 7 and 13 in the 2026-09-24 derivation-fixes plan). No decision on the edge itself was found. The verifier looked in `docs/adr`, `CONTEXT.md` (no mention of the leeway), that plan's header, `UserGuide.md` (ambiguous wording) and the 2026-09-24 backend-scoring-drift handoff (describes the code, not a user quote).

*Owner proposed: core/backend_value.h (already the single owner; only the edge needs a decision). Candidates: backend-9.*

#### 305. Optional early fills get a badge, reversing the approved plan's test

The question is: does an activation that skips fills get a badge for its optional early fill? An optional fill is one the player does not need to hit.

`activation_badge` in `src/app/path_view.cpp` (line 42) says yes. When `difficulty()` is empty and the activation `is_e_critical()`, it uses `e_difficulty(true)` instead. `path_view.h` (around lines 124-131) documents this. Everything else counts required fills only: the path timing, the warning colour and the squeeze<= filter. CONTEXT.md defines Difficulty as the hardest *required* squeeze.

This is a special case, not drift. On screen, a path whose button shows no timing can have an activation row with an uncoloured "early fill 20 ms" badge.

It came in 1eb8063 (2026-09-28, "Early fill: rename calibration fill, and badge E activations that skip fills"). The commit message explains the reason but quotes no user. The approved 2026-09-27 UI-redesign plan had the opposite test: an optional fill is not required, so no badge. The spec (`docs/superpowers/specs/2026-09-27-ui-redesign-design.md`, line 47) says the badge shows when an activation needs a squeeze. The verifier found no ADR or CONTEXT.md line approving the change.

`Activation` in core/model should own the rule. The user should confirm whether optional fills get a badge.

*Owner proposed: core/model Activation. Candidates: screenA-19.*

#### 306. Backend rating bands start at a fixed 10 ms nobody chose

The question: which rating label does a backend row get?

One function answers it: `BackendSqueeze::summarystr` in `src/core/model.cpp` (lines 311-327). The outer edges follow the hit window W. The inner edges are bare -10 and +10 ms numbers that ignore both W and the leeway. For plain rows, the Standard edge calls `core::counted_without_squeeze`, which follows `backend_leeway_ms`. The comment in `src/core/model.h` (around line 211) still says "-10/3/10 edges", but the 3 is now the leeway setting.

| offset (W=85, leeway 3) | SP row | plain row |
|---|---|---|
| -85 | Hard SqOut | Easy |
| -10 | Standard SqOut | Standard |
| 2.99 | Standard SqOut | Standard |
| 3 | Standard SqOut | Hard (uncounted) |
| 10 | Easy SqOut | Hard (uncounted) |
| 85 | Free SqOut | Insane (uncounted) |

`docs/UserGuide.md` (line 122) also says "3ms to 85ms". It says the top edge follows the hit window, but it writes the leeway default as a fixed 3. With `backend_leeway_ms = 15`, a plain row at 12 ms rates Standard, but the guide still points at 3 ms. That was shown by reading.

The user sees these bands on every backend rating label. With a leeway of 10 or more, plain rows read Standard up to the leeway while SP rows turn Easy SqOut at 10.

J. A. Salazar added the numbers upstream in c9c00cf (2025-08-12). They were ported in 3b69958 (2026-08-15). No user decision was found. The verifier checked docs/adr, CONTEXT.md and the 2026-09-24 plan. Decision 19 there keeps only the label text, and decisions 7 and 13 cover the leeway. Core should own the value as a named constant once the user picks it. The guide line should name `backend_leeway_ms`, like line 131 does.

*Owner proposed: core (a named constant or a Rules field, once the user decides). Candidates: squeeze-11, backend-7, tempo-30, display-20, screenA-16.*

#### 307. An exact tie with the cap ceiling is not counted as a clamp

The question is what happens when the cap ceiling and the plain +2 measures land on the same tick. Is that a clamp (the meter overfilled) or not?

Only one place decides. `ScoreGraph::extend_deacts` in `src/search/graph.cpp` (line 274) counts a clamp only when the ceiling is strictly earlier. `build_sp_meter_curve` in `src/app/preview_view.cpp` (lines 167-169) computes the same meter level but has no clamp flag, so it does not copy this choice. `squeeze_rating.cpp` and `path_view.cpp` only read the stored `clamp_tick`.

The end tick is the same either way, so nothing disagrees today. The test "SP cap overfill: a mid-SP phrase that only ties the cap does not clamp" in `tests/test_search.cpp` (line 700) pins the tie: end 6912, no `clamp_tick`.

On screen, an exact tie shows no overfill warning, and the squeeze anchor stays on the earlier note. "No clamp" fits the reading that nothing is lost when the meter fills exactly.

`extend_deacts` already owns it, which is right, since it computes the ceiling. The clamped flag came in 396082a (2026-09-21); the strict comparison as a min dates from the port, 3b69958. No user decision was found in ADR 0013, CONTEXT.md "Cap-clamped window", or the 396082a commit message.

*Owner proposed: ScoreGraph::extend_deacts in src/search/graph.cpp. Candidates: spwin-20.*

#### 308. Unanalyzed song's Preview gauge pins at 4 bars, ignoring the settings cap

The question is which SP cap the Preview gauge uses when the song has no Ready record.

`render_preview_panel` in `src/ui/preview_tab.cpp` (lines 194-196) uses `kCloneHeroSpCap`, a hard 4, in that case. It does not use the cap in the user's settings. With no Ready record, `details_panel.cpp` (lines 256-259) clears the selected path. With no path, `build_sp_meter_curve` fills the gauge to its cap and pins it there.

This is visible today in one case, shown by reading. Set the settings cap to 8 bars and open a song that has not been analyzed. The gauge fills to 4/4 and stays there. A new analysis of that song would run at 8.

CONTEXT.md (line 214) says only "Without a path it fills and pins at the cap". It does not say which cap.

The settings cap is the natural owner, since that is the cap the user picked. The user should confirm. Introduced by aab5ff1 (2026-08-31, then in `details_view.cpp`), moved by 2facc04 (2026-09-27). The user did not ask for the Clone Hero cap here. The verifier found no ADR and nothing relevant in a grep of `docs/`. The aab5ff1 message repeats the CONTEXT.md wording but is Claude co-authored, not a user quote.

*Owner proposed: Settings::sp_cap (the cap the user picked), unless the user decides otherwise. Candidates: store-17.*

#### 309. Leaderboard calls every score at or below optimal 'matched'

The question is what status and percent a dmleaderboards score gets.

One place decides it: `collect_dm_rows` in `src/app/dm_report.cpp` (lines 198-204). A score above Hydra's optimal gets "above optimal". Every other joined score gets "matched". A row with no stored result gets "not analyzed" or "not in library". The percent of optimal is filled in only when the run was at 100% speed and optimal is above zero. The page script and `tally_dm_rows` only read the status back.

Here "matched" means "joined to a Hydra result by chart hash". The UserGuide (line 216) uses it that way. It does not mean the score equals optimal. So a score 50,000 under optimal shows a "Matched" chip, with the 50,000 in the Points left column. Runs at other speeds show a dash under % of opt.

This came in with 21ddf53 (2026-08-18), a commit that only started tracking already-written files. No user decision defines either rule. The verifier looked in `docs/adr/` (0002, 0010 and 0016 mention dmleaderboards but not statuses), `CONTEXT.md` (no "matched" entry), `docs/UserGuide.md` lines 214-224, the commit message, and the 2026-09-26 audit-fixes plan. The user should pick the status names and the speed rule, and `collect_dm_rows` should keep owning them.

*Owner proposed: collect_dm_rows in src/app/dm_report.cpp (the user should name the statuses). Candidates: score-18.*

#### 310. Average multiplier shows 0.000x when a path has no scoring notes

The question is what average multiplier a path gets when its base score is zero.

One place answers it: `Path::avg_mult` in `src/core/model.cpp` (lines 610-615) returns 0.0 when the chart's base score is 0. Two callers show the result: the details text in `src/app/path_view.cpp` (line 372) and the summary column in `src/store/record_store.cpp` (line 489). No other average-multiplier formula exists in `src/`.

There is no second copy, so nothing drifts. The open point is the choice itself. A path with no scoring notes would read "Avg. Multiplier: 0.000x". That needs a chart with zero base score, which is unlikely in practice.

The guard is older than the C++ port. Python commit e8ce2f1 (2026-08-09, "Stop a failed save hanging the analyze modal") added it, and 3b69958 (2026-08-15) ported it. That commit message records the 0.0 as a crash fix for a divide-by-zero on save. It does not record a choice about what the screen should show. Nothing on it was found in `docs/adr/`, `CONTEXT.md` or `docs/UserGuide.md`. `Path::avg_mult` should stay the owner.

*Owner proposed: Path::avg_mult in src/core/model.cpp (already the single owner). Candidates: score-19.*

#### 311. A negative path depth is fixed only in the GUI; INI and CLIs crash the search

The question is simple: what path depth is a valid setting? Path depth is the "how far below the best score to keep" number in the Score range box.

Only one place answers it. `render_score_range` in `src/ui/settings_bar.cpp` (around line 118) turns a negative value into 0, but only when the user types in the box. `Settings::load_file` in `src/app/config.cpp` (line 73) reads `depth_value` with plain `atoi` and no check. The same function does clamp the preview volume, the hit window and the SP cap, so depth is the odd one out. `settings_from` in `tools/replay.cpp` (line 157) passes `--depth` straight through. `hydra_batch` reads the app's INI, so it inherits the same gap.

They disagree today, and the verifier showed it by a run. `hydra_replay dump` on Allister - Overrated with `--depth 0` gives 2 paths. With `--depth -1` it stops with "search reached a broken state", in both Scores and Points mode. The reason: with -1, every survivor in a group of two or more counts as "outscored", so the whole group is thrown away.

A hand-edited `hydra_settings.ini` with `depth_value=-1`, or a CLI `--depth -1`, makes every analysis fail with that error.

It came in with the C++ port checkpoint 3b69958 (2026-08-15). No user decision was found in CONTEXT.md or docs/adr. `Settings::load_file` should own the check, because every entry point (GUI, INI, CLIs) passes through settings.

*Owner proposed: Settings::load_file in src/app/config.cpp (one validator shared by GUI, INI and CLI settings). Candidates: fills-13.*

#### 312. Fill comparison labels a chart with no score anywhere as 'Only in 1.1 db'

The question: for a chart in the 1.0-vs-1.1 fill comparison, which database does it exist in?

`collect_fill_rows` in `src/app/fill_report.cpp` (around lines 195-203) answers it by looking at scores, not at records. If both sides have a score, it compares them. If only the old side has one, it writes "only 1.0". Anything else falls through to "only 1.1". `tally_fill_rows` (around line 217) maps the strings back with a catch-all `else`, and the page script (line 126) counts them a third time.

| old record | old score | new record | new score | label |
|---|---|---|---|---|
| yes | yes | no | - | only 1.0 |
| yes | no | yes | no | only 1.1 |
| yes | no | no | - | only 1.1 |

The last two rows are wrong by reading. A score is unset when a stored record has no paths, and the store accepts such records. The verifier did not run it, and did not show that the engine ever writes a Ready record with zero paths.

If it happens, the user sees a chart that exists only in the 1.0 database listed under "Only in 1.1 db".

It came in with 406d473 (2026-08-31). No user decision was found in CONTEXT.md, ADR 0010, the `fill_report.h` comments or the plan headers. `collect_fill_rows` should own the label and decide it from which record exists.

*Owner proposed: collect_fill_rows in src/app/fill_report.cpp. Candidates: display-39.*

#### 313. 60 ms early-fill window has no recorded user decision for its value

The question: how early can an activation come before its fill, and when does it earn the E label? E marks an early-fill activation that is hard to time.

One constant answers both. `kEarlyFillWindowMs = 60.0` lives only in `src/core/model.h` (line 84). `Engine::branch_activate` in `src/search/engine.cpp` (around line 540) uses it as the legality cutoff. `Activation::is_e_critical` in `src/core/model.cpp` (around line 414) and `is_e0` in `model.h` (around line 177) use it for the E label. `is_e0` writes the comparison again instead of reusing `is_e_critical`, a small in-house copy.

| e_offset (ms) | legal? | E label? |
|---|---|---|
| -60.01 | no | - |
| -60.0 | yes | yes |
| 0 | yes | yes |
| 59.99 | yes | yes |
| 60.0 | yes | no |

The uses agree today, by reading. This is not a duplicate. The open point is who chose 60.

The value decides which activations exist at all and where E appears in path notation.

The window was 85 ms in 11305b3 (2026-08-19). Commit 396082a (2026-09-21) narrowed it to 60 using a corpus check (hardest real E0 is 57.7 ms). 1eb8063 (2026-09-28) renamed it. That commit is agent-written and quotes no user. CONTEXT.md "Early fill (E)" says the window is fixed but gives no number. No ADR or plan-header decision was found. `core/model.h` is already the right owner; it needs a decision record.

*Owner proposed: kEarlyFillWindowMs in src/core/model.h. Candidates: fills-16, tempo-32.*

#### 314. A beat is always a quarter note in every meter, with no named rule, used by the fill deadline and the beat lines

The question is how many ticks make one beat. A tick is the chart's smallest time step; the chart's resolution says how many ticks fit in a quarter note.

Every place in the code assumes a beat is a quarter note. So one beat is `tick_resolution()` ticks, whatever the time signature says. Nobody names this rule. Each place multiplies the resolution itself.

These places use it. `Timecode::Timecode` in `src/core/timing.cpp` (around line 132) counts the measure.beat.tick numbers this way. `build_beat_events` in `src/app/preview_view.cpp` (around lines 377-411, the beat step around line 393) draws a Preview beat line every resolution ticks. In `src/parse/song.cpp`, `Song::check_activations` (around line 272) and `fill_lands_on_chord` (around line 109) turn fill distances in beats into ticks. So does `apply_timesig` (around lines 118-120) when it builds ticks per measure. `activation_fill_deadline_ms` in `src/search/graph.cpp` (lines 46-74) is the only owner of the fill deadline, and its CH 1.1 branch subtracts `4 * timing.tick_resolution()` (line 53).

The fill deadline is the cut-off for a drum fill to spawn: a fill only appears if the player has enough Star Power before it. Under CH 1.1 it is the fill start minus 4 beats. Under CH 1.0 it is the fill length plus a 1/16-beat pad, clamped to 250..10000 ms (lines 65-73). That function's only production caller is in the same file.

The Preview also has two smaller rules. `build_beat_events` draws a half-beat line at `tick_r / 2` with whole-number division (line 404). `build_preview_scene` (around lines 274-279) runs the grid two measures past the last note, using the meter at that note.

All of these read the same `tick_resolution()` and agree today. This was shown by reading; no chart was run. A beat line falls exactly where the time box shows .0 ticks. In 6/8 the time box counts a bar as beats 1 to 3, next to a "6/8" label, and that matches the fill rules, so it is not a drift between copies.

What is undecided is whether Clone Hero counts beats this way in 6/8 or 7/8. Nobody established that.

On screen, a 6/8 chart draws three quarter-note beat lines per bar instead of six eighth-note lines. An odd resolution puts the half line half a tick early: resolution 193 gives 96, not 96.5. If Clone Hero counts fill beats differently in compound meters, some fills on those charts would be wrongly offered or refused as activation points.

The user decided part of this. CONTEXT.md (lines 114-116) and ADR 0010 record "a flat 4 beats" and the 250..10000 ms clamp. But CONTEXT.md never defines a beat, and its "Beat line" entry (around line 187) names bars, beats and half-beats without defining them either. The res/16 pad is decision 22 in the header of `docs/superpowers/plans/2026-09-24-derivation-fixes.md`. The Task 18 body of that plan calls it decision 25, so the numbering disagrees. The code comment at `graph.cpp` line 64 credits the pad to ADR 0010, which never mentions it. ADR 0008 says the Preview is a faithful port of Onyx, but Onyx's source is not in the repo, so that could not be checked. No decision on 6/8 or odd resolutions was found.

The members disagree on when the 4-beat literal arrived. Git settles it. The exact line `4 * timing.tick_resolution()` was written in 406d473 (2026-08-31), along with the CH 1.0 pad and clamp. An earlier spelling of the same rule in that file dates to the C++ port, 3b69958 (2026-08-15). The Preview beat-line and half-line rules came in with ba0885a (2026-08-22). The song.cpp and timing.cpp copies date from the C++ port.

A named ticks-per-beat on `SongTiming` should own this. Then "beat" has one definition, and one place to change if 6/8 turns out to count differently.

*Owner proposed: A named ticks-per-beat on SongTiming (activation_fill_deadline_ms and build_beat_events read it). Candidates: fills-17, tempo-17, tempo-28, tempo-34.*

#### 315. Fill placement tie-breaks, truncation and silent drops have no user decision

The question: where do generated fills go, how long are they, and where does an authored fill end?

Each part has one owner in `src/parse/song.cpp`. The judgement calls inside them were never decided by the user.

`Song::check_activations` (around line 207) stops generating fills for the whole chart if even one authored fill exists. At line 262, `dist <= bestdist` means a tie in distance goes to the later chord. At lines 271-272 and 282, the distance and fill length are cut down to whole ticks. `fill_lands_on_chord` (around lines 103-110) truncates the slop to whole ticks, and a tie between the previous and next chord lands on the next one. `apply_fill_end` (around lines 126-130) silently drops a fill whose last chord sits before its own start.

There is no copy to disagree with. The effect shows on charts with equidistant chords or odd tick resolutions: activation points can move or vanish.

The tie and truncation rules came in with 3b69958 (2026-08-15). The silent-drop guard is from the same commit and moved into a shared helper in 4ce409a (2026-08-18). The user did choose the numeric fill constants (decision 13 in the 2026-09-24 plan header, mirrored in `core/rules.h`). The tie, rounding, "one authored fill disables generation" and silent-drop rules were not found in CONTEXT.md, docs/adr, docs/UserGuide.md, docs/development.md or the plan headers.

*Owner proposed: Song::check_activations and fill_lands_on_chord in src/parse/song.cpp. Candidates: fills-23.*

#### 316. Tied paths merge without comparing when their meter reached 2 bars

The question: which live paths count as ties and get folded into one leader during the search?

`Engine::reduce_iteration_paths` in `src/search/engine.cpp` (around lines 888-897) groups paths by SP meter, or by SP end time while SP is active. Inside a group, `reduce_group` (around line 748) merges equal scores. Neither key includes `sp_ready_ms`, the moment the meter reached 2 bars. Yet `branch_activate` (around lines 539-540) uses `sp_ready_ms` to decide whether an early fill can be summoned. `skipped_e_offset` (around lines 569-581) is also left out, and it later feeds the E label and early-fill difficulty.

So a follower that could reach an early fill the leader cannot may be folded away. After the merge, only the leader's timing decides future activations.

No chart was found or run where this changes anything. The only evidence is a code comment from c89596b (2026-09-25): 0 score changes and 0 variant changes on 96 corpus charts. The verifier could not re-check it without a build.

If it ever bites, a tied variant would be missing from the Paths list, and in theory the optimal score could be lower.

The key dates from 3b69958 (2026-08-15). No user decision was found. Task 18 of the 2026-09-24 plan only measures it, its 28 header decisions do not mention it, and neither do CONTEXT.md or docs/adr. The engine should own it: either add `sp_ready_ms` to the key or record a decision.

*Owner proposed: Engine::reduce_iteration_paths in src/search/engine.cpp. Candidates: fills-15.*

#### 317. Auto cleanup recognises only default-ladder rows; custom-ladder rows wait for re-analysis

The question: when Hydra removed the Auto cap mode, which old Auto results does the one-time cleanup delete?

`Rules::retired_auto_fingerprint` in `src/core/rules.cpp` (around lines 53-60) hashes a frozen literal of the default ladder, `16,32,64,128,256,512`. That matches what 1.8.4 produced under the default. `delete_auto_results` in `src/store/record_store.cpp` (around lines 1028-1066) deletes only rows carrying that fingerprint. `load_rules_file` in `src/app/rules_file.cpp` (around lines 65-67) now reads and ignores `auto_cap_ladder`.

So a user who hand-edited their ladder in 1.8.4 (say `32,64`) keeps those Auto rows after the cleanup. That part holds, by reading. The finder's claim that they are never cleaned up was refuted. The rows already fail the Ready test, so they read Stale. The next analysis of that chart purges them through the store's normal removal of non-Ready rows (`write_row`, around lines 930-943).

On screen, custom-ladder Auto rows show Stale until that chart is analyzed again, then disappear.

The literal came in with eaf3b73 "Remove Auto" (2026-09-27). The user's header decisions 6 and 7 in `docs/superpowers/plans/2026-09-27-ui-redesign.md` say to drop Auto and delete its results on first start. The custom-ladder scope appears only in that plan's agent-written "Questions settled during planning" section, not as a user quote.

*Owner proposed: Rules::retired_auto_fingerprint in src/core/rules.cpp. Candidates: fills-24.*

#### 318. Preview time box can read 0:60.000 instead of 1:00.000

The question is: how should a playhead time print as minutes, seconds and milliseconds?

One place answers it: `clock_str` in `src/app/preview_view.cpp` (around lines 415-422). It picks the minute first, then prints the leftover seconds with `%06.3f`. That format rounds to three decimals after the minute is already fixed. So 59.9996 seconds stays in minute 0 and prints as 60.000. The time box uses it for both the playhead and the length (line 452). The other minutes-and-seconds formatter, in `src/ui/library_dialogs.cpp`, rounds first and does not have this bug.

A Python run of the same arithmetic and format showed it. 59999.6 ms prints `0:60.000` instead of `1:00.000`. 119999.7 ms prints `1:60.000`.

On screen, for up to half a millisecond before each whole minute, the time box reads m:60.000. Stepping or scrubbing can park the playhead there.

It came in with cbb72aa (2026-08-31, "Preview time box reads the engine's timing"). That commit adopts Moonscraper's m:ss.mmm layout but says nothing about rounding. No user decision was found. `clock_str` should own the fix: round to whole milliseconds, then split into minutes and seconds.

*Owner proposed: clock_str in src/app/preview_view.cpp (round to whole ms before splitting). Candidates: display-33.*

#### 319. Charts with no time signature read as 4/4, the default is written three ways, and the Preview time box keeps its own 4/4 fallback

The question is what timing Hydra assumes when a chart leaves it out, and in particular which time signature the Preview time box shows before any change.

For time signatures, `Song::Song` in `src/parse/song.h` (around lines 91-94) starts every song at 4/4. A .chart `TS n` line with no second number means n/4 (`song.cpp` around lines 703-705). For tempo there is no default. If a chart has no tempo at tick 0, `MsIndex` in `core/timing.cpp` (around lines 25-26) throws and the chart fails to load. No 120 BPM fallback exists.

The 4/4 default is written three ways. `Song::Song` inlines the ticks-per-measure formula (resolution times 4, which is 1920 only at 480 ticks per beat) instead of calling `apply_timesig` (`song.cpp` around lines 118-121), which owns that formula. `Song::Song` also sets the time signature to {4, 4} itself. And `build_time_box` in `src/app/preview_view.cpp` (line 470) starts from its own 4/4 literal, then takes the last signature at or before the playhead. `tests/test_preview_view.cpp` (line 515) pins "BPM 0.000 · 4/4" for an empty scene.

All three agree today, shown by reading. Every scene built from a Song copies Song's tick-0 signature (`preview_view.cpp` lines 294-295). So the time box's own literal is reached only for a default-built scene with no song.

On screen, a chart with no tick-0 tempo does not load. A chart with no time signature is analysed and shown as 4/4. An empty Preview shows "BPM 0.000 · 4/4" from the time box's literal. If the Song default ever changed, real charts would follow it but an empty Preview would still say 4/4.

`Song` should own the default, since it is the parser's answer, and it should set it through `apply_timesig`. The time box should read only the scene. The tick defaults came in with 3b69958 (2026-08-15). The time-signature default and the time box fallback came in with 34b2e51 (2026-09-27, "Preview: time signature line in the time box"). The user did not ask for the fallback. The plan `docs/superpowers/plans/2026-09-27-preview-sp-drain-box.md` (line 475) states 4/4 as a planner constraint, and its quoted "User decisions (revision)" list (lines 481-486) covers only the label and its placement. No ADR or CONTEXT.md entry covers either default.

*Owner proposed: Song (src/parse/song.h): the constructor sets the tick-0 default by calling apply_timesig(0, 4, 4); build_time_box reads scene.time_sigs only. Candidates: sweep-tests3-c16, tempo-35.*

#### 320. .chart files always count ghosts and accents and report dynamics on

The question is whether a .chart file has dynamics turned on. Dynamics means ghost and accent notes, which score differently.

One place answers it. `ChartParser::parse` in `src/parse/song.cpp` (around line 1059) always sets the flag to true. The parser also applies the N 34-43 accent and ghost flags with no gate (around lines 884-891). So the flag and the scoring agree with each other. A MIDI chart, by contrast, needs the `[ENABLE_CHART_DYNAMICS]` tag.

This is a single place, so nothing disagrees today. The open part is whether Clone Hero treats .chart dynamics the same way. That has not been checked.

On screen, every .chart shows "Dynamics enabled: yes" and is scored with its accent and ghost flags. If Clone Hero gates them, those scores and counts would be wrong for every .chart.

The ungated pricing dates from the C++ port, 3b69958 (2026-08-15). The reported flag came from ec3ade9 (2026-09-21). No user decision was found. ADR 0012 only says the .chart parser was untouched for kick dynamics, and CONTEXT.md is silent. The ec3ade9 commit message is agent-written.

`ChartParser` should stay the owner. It needs a recorded user decision or a check against Clone Hero.

*Owner proposed: ChartParser (src/parse/song.cpp), pending a recorded decision or a Clone Hero check. Candidates: parse-20.*

#### 321. Scan reads .sng and .srb names from only the first 1 MB

The question is where the scan reads a .sng or .srb file's song name and artist.

`discover_charts` in `src/app/analysis.cpp` (around line 440) keeps only the first 1 MB while it hashes the file. The limit is `kSngHeadCapture` (around line 195). It then reads the metadata block from that slice. Two other readers use the whole file instead. `sng_delay_ms` in `src/app/preview_source.cpp` (around lines 224-228) reads .sng metadata for the Preview. `load_songpath_srb` in `src/parse/song.cpp` (around lines 1123-1132) reads .srb metadata when loading.

They agree on every normal file, since real metadata is a few KB. They would disagree only on a metadata block that ends past byte 1,048,576. That case is hypothetical; no such file was checked in testdata.

On screen, such a file would show "(unknown)" and "<unknown artist>" in the library, while the Preview still read its keys.

The limit came in with 21ddf53 (2026-08-18). Only a code comment justifies it. No user decision was found in CONTEXT.md, docs/adr or the 2026-09-26/27 plans.

`discover_charts` can stay the owner, but the limit needs a recorded decision. The whole-file readers could share the same rule.

*Owner proposed: analysis.cpp discover_charts (src/app/analysis.cpp), pending a recorded decision; the whole-file readers could share its rule. Candidates: parse-27.*

#### 322. Settings value ranges: some enforced only in the UI, some never

The question is which values each setting may hold, and what replaces a bad one.

The answers are spread out. `Settings::load_file` in `src/app/config.cpp` keeps `preview_volume` only inside 0 to 100 (lines 80-83). It keeps `hit_window_ms` only above 0 (84-87). It keeps `sp_cap` only at 1 or more, so "auto", 0 and junk become 4 (88-92). It reads `depth_value`, `depth_mode` and `mslimit_value` with `atoi` and no check at all (73-76). `Settings::backend_limit` applies `std::abs` (159-163). The Path limit range of -500 to 500 lives only in `render_path_limit` in `src/ui/settings_bar.cpp` (line 144). The Backend limit range of 0 to 500 lives only in `render_path_footer` in `src/ui/paths_tab.cpp` (line 503). Both 500s are hand copies of `kSqueezeWindowMs` in `src/core/model.h` (line 75).

This was shown by reading. A hand-edited `mslimit_value=900` loads as 900, and the search runs with that limit. One click on +1 gives 901, which snaps to 500. A `backendlimit_value=-30` is used as 30, then snaps to 0 or higher on first edit. A `depth_mode=2` searches Scores but files the result under lens depth_mode 2.

The user would see a setting change by itself the first time they touch it.

Settings should own every range and the UI should call it. The 500 bound came from 9a77258 (2026-08-28); the load checks from ba0885a and d49c624; `abs` from 133b52e. Only `sp_cap=auto` reads as 4 (2026-09-27 ui-redesign plan, decision 6 and the accepted call at line 242) and difficulty in any case (2026-09-24 plan, decision 24) trace to the user. Nothing else was found in docs/adr, CONTEXT.md or the plan headers.

*Owner proposed: app::Settings (one range table built from kSqueezeWindowMs, used by load_file and the UI). Candidates: display-30, store-21.*

#### 323. Library search 'squeeze<N' quietly means 'at most N'

The question is whether the library search `squeeze<20` keeps a song whose hardest squeeze is exactly 20.0 ms.

One place answers it. `parse_library_query` in `src/app/library_query.cpp` (lines 344-351) sends both `squeeze<` and `squeeze<=` to the same bound. `query_matches` (lines 383-390) drops a row only when the hardest squeeze is above that bound. So `squeeze<20` keeps a 20.0 ms song. `docs/UserGuide.md` (line 84) says the two forms work the same.

There is one owner, so nothing can drift. The user sees a song at exactly the number they typed after a strict "less than". That is what the guide promises.

The question is who chose it. It came in with b89beed on 2026-09-27. CONTEXT.md (line 59) defines only `squeeze<=N`. The 2026-09-27 ui-redesign plan's decisions 1-12 never mention `squeeze<`. The rule sits in that plan's task text (around line 1390). Decision 8 approved "the search upgrades and the rest of the recommendations" as a whole, which covers it only broadly. No explicit user decision was found.

*Owner proposed: parse_library_query in src/app/library_query.cpp (already the only place). Candidates: display-32.*

#### 324. One duplicate hides the whole 'Best all-0 path' section

The question is when the Paths tab shows the "Best all-0 path" section. An all-0 path is one that activates at the earliest chance every time.

`build_path_list` in `src/app/path_view.cpp` (lines 406-420) answers it. If any stored all-0 path matches any listed path in both total score and notation, the whole section is hidden. So with several all-0 variants, one duplicate hides the others too. The engine has its own, different test. `attach_allzero` in `src/search/pather.cpp` (lines 43-49) skips the all-0 search when the best path is already all-0 with difficulty 0 or less. The verifier did not find an input where the two tests disagree.

The user would see the section vanish, including variants that were not duplicates.

The rule came in with 21ddf53 on 2026-08-18. No user decision was found. CONTEXT.md's "All-0 path" entry defines the path, not when to hide it. The 2026-09-27 ui-redesign plan's accepted call (line 249) covers only the label wording. `build_path_list` should own it after a decision, ideally agreeing with the engine's test.

*Owner proposed: build_path_list in src/app/path_view.cpp, after a user decision. Candidates: screenA-20.*

#### 325. hydra_rules.ini lower bounds were picked in code, not by the user

The question is which values hydra_rules.ini accepts.

`load_rules_file` in `src/app/rules_file.cpp` answers it alone. It requires `backend_leeway_ms` of 0 or more (line 57). It requires `max_tied_paths` of 1 or more (63) and `fill_cooldown_measures` of 1 or more (67). It requires `fill_max_distance_beats` of 0 or more (68) and `fill_length_measures` above 0 (69-71). It requires `fill_land_slop_beats` of 0 or more (73). So a leeway of 0 is accepted, but a fill length of 0 or a tied-path count of 0 is refused.

The user would see an error, and the GUI keeps Analyze off until the file is fixed.

The bounds came in with 7e14401 on 2026-09-24. The user decided that a bad value is an error (2026-09-24 plan, decision 18 and line 40). Which values count as bad was not decided. The verifier found no ranges in that plan's header (lines 40-73), CONTEXT.md, docs/adr or `docs/UserGuide.md` (lines 236-253). The bounds appear only in the plan's Task 1 step code, which is not a decision.

*Owner proposed: load_rules_file in src/app/rules_file.cpp, after a user decision on the ranges. Candidates: store-22.*

#### 326. hydra_bench corpus mode benchmarks with no timing limit, unlike the GUI default

The question is: what search settings does hydra_bench's corpus mode time?

`corpus_bench` in `tools/bench.cpp` (around line 188) sets its own values: cap 4, depth by scores at 4, Pro Drums and 2x Bass on, Expert. It also sets `ms_filter` to none, meaning no timing limit. Folder mode in the same file (`folder_breakdown`, around line 52) instead takes the GUI default from `app::Settings::to_analysis_settings` in `src/app/config.cpp`. That default turns the limit on at 10 ms.

So the two modes differ today on one setting: the timing limit. The verifier found this by reading. Everything else matches.

Corpus mode never claims to run the GUI default. Its one output line is labelled "cap4 d4". The approved plan `docs/superpowers/plans/2026-09-26-codebase-audit-fixes.md` uses that line as a fixed before/after timing yardstick. Tying it to Settings would make the yardstick move. What the user sees is a benchmark number for a search the app never runs, with a label that omits "no ms limit". The literal 4 also restates `kCloneHeroSpCap`.

The no-limit setting came in 515bf37 (2026-08-22); the label in ba0885a (2026-08-22). 1b356db (2026-09-25) moved folder mode onto Settings and left corpus mode alone. No ADR or CONTEXT.md entry decides the missing limit. The plan treats "cap4 d4" as a named yardstick but never says why it has no limit. Corpus mode should stay fixed, with a label like "cap4 d4 no-ms".

*Owner proposed: tools/bench.cpp corpus_bench (keep it fixed, but label it honestly). Candidates: sweep-core1-c9.*

#### 327. Preview, audio and leaderboard literals with no recorded user decision

The question: which thresholds, fallbacks and tuning numbers in the render, audio and network code carry a user decision?

The verifier checked twelve literals. All exist where listed.

| Literal | Where | Origin | Decision |
|---|---|---|---|
| ghost width 0.7, pad gem half-size | `highway_draw.cpp` ~152 | 28660f3 | ADR 0008 (Onyx numbers in general) |
| specular 0.5, shininess 32 | `draw_command`, `preview_renderer.cpp` ~258 | ba0885a | ADR 0008 in general |
| channels 1..2, 48 kHz | `decode_ogg_opus` ~173 | ba0885a | ADR 0006 |
| OpusHead at least 19 bytes | `decode_ogg_opus` ~173 | ba0885a | Opus format's own header size |
| overlay min scale 0.6 | `overlay_layout.h` ~61 | 76c5530 | agent-written plan and spec only |
| 50 ms cancel poll | `await_step`, `dmbot_client.cpp` ~123 | 2130ca3 | agent-written commit message only |
| flat-line epsilon 1e-3 | `highway_span_at`, `overlay_layout.cpp` ~37 | 76c5530 | none |
| magenta colour fallback | `parse_hex_color`, `preview_config.cpp` ~20 and ~24 | ba0885a | none |
| 64-byte Ogg tag window | `sniff_format`, `decode.cpp` ~58 | ba0885a | none |
| chunk 4096 frames, int16 / 32768 | `decode.cpp` ~84 and ~119 | ba0885a | none |
| WinHTTP timeouts, status 200 only, speed 100 | `dmbot_client.cpp` ~164, ~209, ~271 | before 21ddf53 | none |

21ddf53 only started tracking files that already existed, so the true origin of the network literals is unknown. The verifier looked in docs/adr/, CONTEXT.md, the 2026-09-27 drain-box plan and spec, and the commit messages of 76c5530, 2130ca3, 28660f3 and 21ddf53.

No effect was observed; these are fallbacks and tuning values, and no run was made. A malformed colour in `3d-config.json` would draw magenta. A slow network would hit the timeouts.

Two small in-file copies also surfaced. The magenta fallback is written twice inside `parse_hex_color`. The 'OpusHead' tag appears in both `sniff_format` and `decode_ogg_opus`. Each literal should stay in its module, with a decision recorded in an ADR or CONTEXT.md if the user wants it.

*Owner proposed: each literal's own module (src/render, src/audio, src/net); decisions belong in an ADR or CONTEXT.md. Candidates: sweep-media1-c18.*

#### 328. Probe decision cut-offs have no user decision behind them

The question is: what cut-offs do the probe scripts use when they judge what the game did? Several are typed as bare numbers.

`hit_time_report` in `watch_window.py` (line 221) accepts the `+0x2e0` field as "the hit time" only if the gap is at most 10 ms. `SongClock.__init__` in `walk_edges.py` (line 95) treats a clock reading within 2 ms as fresh and caps its fill-in at 50 ms. `clamp_verdict` in `analysis.py` (lines 157-158) uses a 1.0 ms tolerance and an 80% majority. `find_live_engine` in `engine_finder.py` skips windows below 0.001, counts the clock as moved only past 0.000001, and sleeps 0.12 s and 0.4 s. `active_probe.py` sweeps offsets of 70 to 100 ms by default (line 65). The sleeps and the sweep are polling pace and a CLI default, so they are weaker than the decision thresholds. `active_probe.py` also repeats walk_edges' `SETTLE_MS = 250`.

They came from these commits. The 10 ms, 2 ms and 50 ms values: 283e028 (2026-09-26). The finder's numbers: 283e028, moved by 288b07a. The 1.0 and 0.8: 396082a (2026-09-21). The offset sweep: aa3ad89.

No user decision was found. The verifier searched `docs/adr/`, `CONTEXT.md`, the 2026-09-25 hit-window plan and the 2026-09-26 audit-fixes plan, plus those commit messages. Where the hit-window plan speaks at all, it says something different: "within a few ms" for the hit time, and a +80 to +92 ms walk.

These cut-offs change only probe console verdicts at the edges, such as exactly 10.0 ms. The Hydra GUI is untouched.

*Owner proposed: tools/ch_probe/constants.py (one named, commented constant per threshold) or a recorded user decision for each. Candidates: sweep-probe1-c27.*

#### 329. Preview beat grid stops a hard-coded two measures past the last note

The question: how far past the last note does the Preview's beat grid run? `build_preview_scene` in `src/app/preview_view.cpp` (line 278) answers with a bare literal, `last_tick + 2 * tpm`: two measures. The measure length comes from the last note's section, so a meter change inside those two measures shifts the end. `tests/test_preview_view.cpp` (line 389) pins the result: last note at tick 720, grid end 4560, last beat line at 4320.

There is only one copy, so nothing disagrees. The problem is that nothing ties the number to anything else. The transport runs to the larger of the last note and the audio end (`src/ui/preview_transport.cpp` line 17). So, by reading, a song whose audio runs more than two measures past the last note keeps playing with no beat lines. The time box keeps naming measures meanwhile (the test at line 414 shows m16 at 30 s).

What the user sees today: beat lines stop two measures after the last note, even when the music continues.

`build_preview_scene` should own it as a named constant, or derive the grid end from the transport length. It came in ba0885a (2026-08-22, Patrick, "SP cap as a runtime setting, transfer-aware squeeze ratings, Onyx-port Preview"). No user decision was found. The verifier looked in docs/adr (the "two measures" hits in ADR 0011 are SP-bar rules), the "Beat line" entry in CONTEXT.md, the plans and specs under docs/superpowers, and the ba0885a commit body.

*Owner proposed: build_preview_scene in src/app/preview_view.cpp (named constant, or derive from the transport length). Candidates: sweep-tests3-c15.*

#### 330. Path limit keeps an over-limit path when it ties the optimal score

The question is: which paths over the Path limit does the search still keep?

`Engine::reduce_group` in `src/search/engine.cpp` answers it. Its tie key (around line 748) keeps a filtered path apart from an in-limit path with the same score. A filtered path is removed only when a reachable score strictly beats it (around line 824). In practice, an over-limit path survives only if it ties the optimal score. Every over-limit path below optimal is dropped. The comment near line 779 states this intent.

The User Guide (line 51) says an extra path is kept only when its hardest squeeze is within the limit. Line 49 uses "extra paths" to mean paths below optimal, so the guide is not strictly wrong. But it never says optimal-score paths ignore the limit.

A run showed it. On "I Am... All Of Me" (Crush 40) in `testdata/input/common/Summer Blast _25 Setlist/Tier 5`, cap 4, Path limit 10 ms, rank 2 "0+ 2 0 0- 2 E0" at 694,985 needs a 78.9 ms SqOut and is kept. "3 E0 1 0- 2 E0" at 694,265 needs the same 78.9 ms and is dropped.

On screen, with a 10 ms limit, Optimal shows a second path with an orange 78.9 ms timing, and the report lists it as rank 2 with tier Extreme.

`Engine::reduce_group` should own the rule, and the guide should state it. The filter came from upstream 90690ac (2025-08-11, "Added ms filtering"). No user decision was found in `docs/adr`, `CONTEXT.md` or commit messages. No test pins this case.

*Owner proposed: Engine::reduce_group in src/search/engine.cpp, with the User Guide stating its rule. Candidates: R5-A2-1.*

#### 331. A .chart cymbal, ghost or accent marker with no note under it fails the whole chart

The question is: what happens to a .chart cymbal, ghost or accent marker when the note it modifies is missing at that tick?

Today the whole chart fails. `Chord::apply_cymbal`, `apply_ghost` and `apply_accent` in `src/core/model.cpp` (around lines 262 to 277) throw a logic error when the note is absent. The only reason given is the code comment 'Python asserts the note is present'. `ChartParser::push_timestamp` in `src/parse/song.cpp` (around line 934) catches only chart-file errors, so this one escapes and aborts the parse. The same handler silently drops a duplicate note: `Chord::add_note` throws a chart-file error, and that does get caught.

The verifier showed this by a run. Adding '384 = N 66 0' to a red-only chord made `hydra_replay score` fail with 'apply_cymbal: note not present'. N 41 and N 35 failed the same way for ghost and accent. A duplicate 'N 1 0' parsed and kept the first note. The finder's comparison with a stray .mid tom marker was refuted: that marker is a span flag, and .mid carries ghost and accent as velocity.

Such a chart would show the raw C++ message as its analysis error, and Preview, song length and Dynamics would fail too. No chart in the user's 4,340 .chart files triggers it today.

The .chart parser should own one stated rule, ignore or reject, applied the same way as the duplicate-note rule. It came from the port, 3b69958. No user decision was found in docs/adr, CONTEXT.md or docs/UserGuide.md. ADR 0015's 'rejects anything malformed' is about chord codes, not this.

*Owner proposed: The .chart parser in src/parse/song.cpp, with one user-chosen malformed-modifier rule. Candidates: R4-A5-2.*

### Visibility 3: visible only if the copies drift later

#### 332. Unknown transfer ratios are silently stored as x1.00

The question is what ratio a record gets when the ratios cannot be worked out.

`frontend_transfer_scales` in `src/core/squeeze_rating.cpp` returns nothing when `deact_tick` is unset (line 28). `transfer_scale_between` (line 16) returns nothing when a measure length is zero or less. The engine copy-out in `src/search/engine.cpp` (lines 1266-1275) then keeps the 1.0 defaults. Its comment says "A nullopt keeps the 1.0 defaults". The stored fields are plain numbers (`src/store/path_codec.cpp` lines 131-134), so the record cannot say "unknown".

ADR 0011's "No fallback" section says the opposite. A reader that finds `deact_tick` unset "says it cannot tell", and the transfer scales "come back unset".

On screen, such a record shows x1.00, no scale line and no warning. That looks the same as a flat-tempo chart. The verifier did not show that a fresh record can reach this branch.

The 1.0 comment came in with 4fa0245 (2026-08-20). The zero-length guard came in with 21ddf53 (2026-08-18) and moved in 0548565 (2026-08-22). No user decision was found for the 1.0 default. ADR 0011 argues against it, and only a comment in `model.h` (lines 304-306) mentions it.

The engine copy-out should own this, with an explicit "unknown" value in the record.

*Owner proposed: engine copy-out (explicit 'unknown' in the record). Candidates: squeeze-14.*

#### 333. Timing tier ladder shape has no recorded user decision

The question: where do the report's timing tiers start and stop?

One place answers it: `timing_tiers` in `src/core/squeeze_rating.cpp` (lines 197-205). The cutoffs are a 2 ms floor (`kDifficultMs`), then W/2, W, 3W/2 and 2W, then Beyond and None. `tier_for` in `src/app/report.cpp` and `beyond_edge_ms` both read this table. Tests in `test_report.cpp` and `test_squeeze_rating.cpp` pin it.

| ms at W=85 | tier |
|---|---|
| 1.99 | Normal |
| 2.0 | Hard |
| 42.5 | Extreme |
| 85 | Insane |
| 127.5 | Insane+ |
| 170 | Beyond |
| null | None |

There is only one copy, so there is nothing to disagree with. The open part is the shape itself.

The user sees this ladder in the report's tier chips, the Timing filter, and the Beyond tile and footer.

The shape is old. DragonDelgar's upstream c29d122 (2025-01-05) had fixed ratings of 1/35/70/105/140. The report adopted that ladder in b257836 (2026-08-09). 7dded44 (2026-08-19) scaled it to W, and b9506f9 (2026-08-23) made it one table. No user decision was found. The verifier looked in docs/adr, CONTEXT.md, the 2026-09-24 plan header, and the b257836 message. That message only says the tiers "reuse Hydra's own squeeze ratings". `timing_tiers` already owns it. Only the shape needs a decision on record.

*Owner proposed: core/squeeze_rating (timing_tiers). Candidates: squeeze-12.*

#### 334. The two-bar activation minimum is a bare literal in four engine lines

The question is how many SP bars must be banked before SP can be activated.

Four lines answer it with a bare number. `Engine::branch_activate` in `src/search/engine.cpp` (line 534) refuses when `p.sp < 2`. `Engine::advance` (line 516) starts `sp_ready_ms` when the meter crosses from below 2 to 2. The next line (517) picks the phrase that did it with `k = 1 - old_sp + buffered`, a hidden "threshold minus one". `ScoreGraph::add_act_edge` in `src/search/graph.cpp` (line 337) builds activation ends only for meters 2 and up. A comment in `src/core/replay.h` (line 236) restates it.

All four agree today. If one changed alone, a path could reach a meter with no activation end. It would be marked NODE_BROKEN (a dead search state), and the search throws "search reached a broken state" (`engine.cpp`, line 1330). The user would see analysis fail, or E offsets (early-activation timings) timed from the wrong phrase.

A named constant beside `kCloneHeroSpCap` in `src/core/model.h` should own it, read by the graph and the engine, including the `k` index. That file already holds the other Clone Hero SP rules.

All three code lines came in the C++ port, 3b69958 (2026-08-15). No user decision was found in CONTEXT.md, `docs/adr/`, `docs/UserGuide.md`, or the 2026-09-24 and 2026-09-27 plan headers. The 2026-09-24 derivation-fixes plan body mentions the minimum only to leave it alone. It is a Clone Hero game rule, but no doc states it.

*Owner proposed: A named constant beside kCloneHeroSpCap in src/core/model.h. Candidates: spwin-11, fills-6.*

#### 335. Cymbal, solo and multiplier-step values have no recorded decision

The question is where the core scoring values come from: the cymbal bonus, the solo bonus, and the combo counts where the multiplier steps up.

`src/core/model.h` (lines 89-93) holds `kNoteBasePoints = 50`, `kCymbalBonusPoints = 15` and `kSoloBonusPerNote = 100`. `to_multiplier` in `src/core/timing.cpp` (lines 9-14) steps up at 10, 20 and 30 notes and caps at x4. ADR 0012 covers the 50 base and ghost/accent doubling. The cymbal 15, solo 100 and the 10/20/30 steps have no ADR or `CONTEXT.md` entry. They come from the upstream project (`to_multiplier` 7521456, 2024-11-25, and the cymbal value d76c430, 2024-12-12, both by DragonDelgar), ported in 1f38f07/3b69958 and named as constants in 7dafadd (2026-09-24). The stars-tab plan (line 21) states 65 per cymbal and 100 per solo note, but in Claude-written design text, not the user quote.

The step rule also lives in a second form. `MultSqueeze::applies` in `src/core/model.cpp` (lines 344-357) hard-codes combos 7, 8, 17, 18, 27, 28 plus a modulo-10 test. A test checks it against `to_multiplier` for 2- and 3-note chords only. Its own comment says why the set stops there is not recorded.

They agree today. If the steps changed in one place, the list of multiplier squeezes would stop matching the score. The values can stay where they are. `MultSqueeze::applies` should derive its combos from `to_multiplier`.

*Owner proposed: src/core/model.h and to_multiplier in src/core/timing.cpp; MultSqueeze::applies should derive from to_multiplier. Candidates: score-27.*

#### 336. Star cutoffs use a 32-bit float multiply not recorded as a decision

The question is how a star cutoff (the score needed for each star) is computed.

One place answers it. `star_cutoff` in `src/core/stars.cpp` (lines 7-12) multiplies the base score by the star's multiplier as a 32-bit float, then rounds up. The multipliers 0.1 to 4.4 live in `kStarMultipliers` in `src/core/stars.h` (lines 23-24), sized by `kMaxStars = 7`. The Stars tab and the tests are the only callers. No second formula exists.

Most of this is a user decision. `CONTEXT.md` "Star cutoff" (lines 105-108) records the seven multipliers, rounding up, 1 to 7 stars, and leaving out the solo bonus. The stars-tab plan (lines 49-51) quotes the user's request and approval. The one part with no decision is the 32-bit float multiply. A 32-bit float carries fewer digits than the usual 64-bit kind, so the product can land a point differently. It is described only in the plan's Claude-written "Where the math comes from" (lines 17-27), from the decompiled game. A unit test pins it at base 786,437.

If it were wrong, the Stars tab cutoffs, the summary's star count and the `stars:` library filter would be off by one point at some bases. This came in with da26438 (2026-09-27). `core/stars` should stay the owner, and the float detail belongs in `CONTEXT.md`.

*Owner proposed: core/stars (src/core/stars.cpp, already the owner). Candidates: score-28.*

#### 337. Preview spans end half a tick after their last note, with no recorded choice

The question is: where does a Preview span end? A span is a stretch drawn with a special look: an SP phrase, a solo, or a fill.

One place answers it. `kSpanEndTicks = 0.5` in `src/render/track_state.cpp` (line 23) is used once, in `build_track_state`'s span lambda (around lines 72-78). The end is the last note's tick plus half a tick, turned into milliseconds through the song's timing. No second copy was found.

There is one copy, so nothing can disagree today. The value decides which gems draw with the SP, solo or fill look. On ordinary charts the result matches the old half-millisecond edge. If the value changed, notes at a span's edge could gain or lose the special look.

It came in with 2a573b4 (2026-09-24, "Preview: end spans half a tick past their last note, not half a millisecond"). Task 14 of `docs/superpowers/plans/2026-09-24-derivation-fixes.md` (around line 8914) explains the move from milliseconds to ticks. But the plan's list of user decisions has nothing on span ends, and CONTEXT.md does not define them. The verifier also checked docs/adr/ and the commit message. No user decision was found. `track_state.cpp` is the right owner; the half-tick value needs a recorded choice.

*Owner proposed: kSpanEndTicks in src/render/track_state.cpp (single owner). Candidates: tempo-25.*

#### 338. Preview audio offset rule is a user decision but lives only in a plan

The question is: how far should the Preview shift the audio against the notes?

One place answers it. `preview_audio_offset_ms` in `src/app/preview_source.cpp` (around lines 330-335) uses the song.ini `delay` when it is present and not 0. Otherwise it uses the .chart `Offset`, in seconds times 1000. A positive value moves notes later (`kOffsetDirection = +1`, line 171). Its three callers in `resolve_preview_source` only pass inputs: .sng delay, nothing for .srb, and the folder's song.ini delay. A search of src/, tools/ and tests/ found no other copy.

There is one copy, so nothing disagrees today. A wrong sign or precedence would shift the Preview audio against the highway.

This rule is a user decision. The user measured it at the game. It is header decision 9 and the Task 17 user gate in `docs/superpowers/plans/2026-09-24-derivation-fixes.md`, with results in commit 1b7eeb3 (2026-09-25). The .sng/.srb extension came with b868de9 (2026-09-26) under decision 4 of `docs/superpowers/plans/2026-09-26-codebase-audit-fixes.md` ("Yes, same rule"). The code needs no fix. The only gap is that no ADR or CONTEXT.md entry records it. CONTEXT.md "Transport" does not mention the offset. `preview_audio_offset_ms` is already the right owner.

*Owner proposed: preview_audio_offset_ms in src/app/preview_source.cpp (already the single owner). Candidates: tempo-23.*

#### 339. Preview's own thresholds and fallbacks have no user decision

The question is which small thresholds the Preview applies on its own.

There are four. `kOnActivationMs = 0.5` in `src/app/preview_view.cpp` (line 576) is the slack for "on an activation"; it drives the Act jumps and the next-activation box. `kSpanEndTicks = 0.5` in `src/render/track_state.cpp` (line 23) draws SP, solo and fill spans half a tick past the last note. `build_time_box` in `preview_view.cpp` shows BPM 0.0 with no tempos (line 463) and 4/4 before any time signature (line 470). Hiding fills after the last activation (lines 325-337) is a user decision in CONTEXT.md ("Path overlay", lines 196-204).

The 4/4 fallback is a second copy of the parser default in `Song` in `src/parse/song.h` (line 93). They match today. For a loaded song neither fallback can run, since the song always has tick-0 entries. This was shown by reading.

If they drifted, the edges of jumps and spans would shift slightly. Owners: the slack 59a3dd9 (2026-09-27), the span end 2a573b4 (2026-09-24), BPM 0.0 ba0885a (2026-08-22), 4/4 34b2e51 (2026-09-27). Nothing was found for these in docs/adr, CONTEXT.md or the plan headers. The 4/4 default should come from `Song`.

*Owner proposed: Preview view-model (src/app/preview_view.cpp) for the slack and span constants; parse/Song for the 4/4 default. Candidates: screenB-24.*

#### 340. Show in Preview parses an overlay key it was told not to parse

The question is whether the overlay the Preview has drawn belongs to the path the user selected.

`overlay_key` in `src/ui/preview_controller.cpp` (around line 22) builds the key as the path key plus "|cap" plus the cap. The header comment on `overlay_path_key` in `preview_controller.h` (around line 75) says to treat that spelling as opaque: compare two, never parse one. `render_preview_panel` in `src/ui/preview_tab.cpp` (around line 238) parses it anyway. It checks whether the overlay key starts with the selected path's key. Several tests repeat the same prefix check.

| Case | Prefix test | Correct? |
|---|---|---|
| Same path | true | yes |
| Different path | false | yes |
| Same path text, score 12 vs 123 | true | no |
| Same path, older cap | true | debatable |

It gives the right answer today. The wrong row needs two paths with the same text and different totals, and no record has one. Found by reading.

Nothing is wrong on screen today. If `overlay_key` changed its spelling, Show in Preview would stop seeking or seek on the wrong overlay, with no compile error.

It was introduced in c5e9758 (2026-09-27, Task 11 implementer). The "|cap" suffix dates from aab5ff1. No user decision was found: the verifier searched `docs/adr/`, CONTEXT.md, the plans and both commit messages.

`PreviewController` should own the comparison, beside `overlay_key`.

*Owner proposed: PreviewController (a matches(path_key) or drawn_path_key method). Candidates: sweep-ui2-c6.*

#### 341. Scrubber marks copy ImGui's hidden 2 px slider padding

The question is where a 0-to-1 fraction sits along an ImGui slider, so the gold activation marks line up with the grab.

`draw_scrub_marks` in `src/ui/preview_tab.cpp` (around line 162) answers it with a 2 px unscaled pad plus half a grab at each end. That copies an ImGui internal. `SliderBehaviorT` in `third_party/imgui/imgui_widgets.cpp` (around line 3120) hardcodes the same 2 px with a FIXME, and exposes no way to read it.

Both agree today, shown by reading. Fraction 0, 1 and anything between land in the same spot. They would only differ if the slider were narrower than the grab plus 4 px, because ImGui clamps the grab there and Hydra does not. The scrubber is at least 120 px wide, so that cannot happen now.

Today the marks sit on the grab centre. After an ImGui update that changes or scales the padding, the marks would sit a few pixels off, worst at the ends.

It came in with c5e9758 (2026-09-27, Task 11). The code first appears in the agent-written ui-redesign plan (f1c6a3d). No user decision was found: the spec only says gold marks show every activation, and CONTEXT.md and `docs/adr/` say nothing about padding.

`draw_scrub_marks` should stay the single owner, since ImGui gives nothing to call. A GUI test comparing a mark to the real grab rect would catch an ImGui change.

*Owner proposed: draw_scrub_marks in src/ui/preview_tab.cpp (only Hydra copy). Candidates: sweep-ui2-c17.*

#### 342. Engine scan silently assumes the front window sits 8 bytes after back

The question is how the probe recognises Clone Hero's drums engine object in memory. `scan_for_engine` in `tools/ch_probe/engine_finder.py` (line 79) glues the back-window bytes and the front-window bytes into one 16-byte search. That only works if the front window starts exactly one double (8 bytes) after the back window. `hits_in_region` (line 68) then backs up by `C.OFF_BACK_WINDOW`. The module never reads `OFF_FRONT_WINDOW`. So the 8-byte gap is a hidden third fact beside the two offsets in `constants.py` (0x30 at line 69, 0x38 at line 72). Related copies: four experiment scripts rebuild the byte pattern by hand instead of calling `engine_finder.normal_pattern`, and `test_engine_finder.py` expects a literal 0x30.

They agree today: 0x38 − 0x30 = 8, checked with Python. If a Clone Hero update moved the front window to 0x40, `EngineModel` would read the new offset, but the scan would match nothing. `find_live_engine` would then print dots forever. Hydra is not affected. It came in with 283e028 (2026-09-26) and was carried into engine_finder.py by 288b07a. No user decision was found in docs/adr, CONTEXT.md, the hit-window plan header, or either commit message. `engine_finder` should derive the gap from the two constants (or assert it), so `constants.py` stays the only place the layout lives.

*Owner proposed: tools/ch_probe/engine_finder.py, deriving the gap from constants.OFF_FRONT_WINDOW - OFF_BACK_WINDOW. Candidates: sweep-probe2-c14.*

#### 343. Stored tempo map is written once and never refreshed or stamped

The question: when the parser's tempo or meter output changes, does re-analysis replace the tempo map the store keeps per chart? It does not.

`RecordStore::upsert_song` in `src/store/record_store.cpp` (around line 861) updates only the names and length when a chart already exists. Both `save_analysis` and `add_song` pass in a fresh map, and the update throws it away. A code comment says the map "cannot have changed" because it shares the chart's content hash. `get_record` (around line 1213) then rebuilds measure text and timeline positions from that stored map. No stamp in `src/store/stored_versions.h` covers it, though the file says every kind of stored computed data is stamped.

Today they agree. The verifier checked git history and found no tempo or meter output change since the C++ port. It did not check the user's real database.

If the parser changed, say to ignore a time signature of 0 beats, a re-analyzed record would mix two sources. Its millisecond facts would come from the new parse. The Paths tab measure text, timeline position and Preview timing would come from the old map. Re-analyze would not fix it.

The insert-once rule dates from the C++ port (`INSERT OR IGNORE`). Commit 3e5d634 (2026-09-26) added the update clause and the comment. The user did not ask for it. The verifier searched the ADRs, CONTEXT.md and the 2026-09-26 plan; the decision that plan cites covers names only. The owner needs a user choice: rewrite the map on every save, or give it its own stamp.

*Owner proposed: src/store/record_store.cpp upsert_song, or a StampRule in src/store/stored_versions.h (needs a user decision). Candidates: R6-A6-1.*

#### 344. Results stamp's bump rule leaves out the parser; the dynamics rule includes it

The question: which code changes make stored results out of date and require a `kResultsStamp` bump? Two stamps guard data built from the same parser output, but they list different code.

The `kResultsStamp` comment in `src/store/stored_versions.h` (around line 38) names the engine in `src/search`, the scoring in `src/core`, and what a record holds. ADR 0018's release check in `docs/adr/0018-stored-data-has-one-set-of-version-stamps.md` says the same. Neither names `src/parse`. The `kDynamicsCountStamp` comment (around line 60) does name the parser files. Yet `analyze_chart_file` in `src/app/analysis.cpp` (around line 498) builds every result from that same parser output.

| Change | Dynamics rule | Results rule |
|---|---|---|
| `src/search` or `src/core` | bump | bump |
| `src/parse` only | bump | no bump |
| a rules value | fingerprint covers it | fingerprint covers it |

Nothing is stale today. The only parser commit since the 1.8.2 release is 9ebee4c, which changed comments only.

After a parser-only fix, say one that moves an SP flag, library chips would still read Ready. They would show old scores and paths a fresh analysis would not produce.

Commit df192dc (2026-09-27) wrote both comments and the ADR. The ADR is a user-level record, but it says nothing about the parser either way. So the gap reads as an oversight, not a decision. The fix is one scope list in the stamp comment and ADR 0018 that names `src/parse`, recorded as a user decision.

*Owner proposed: src/store/stored_versions.h kResultsStamp comment and ADR 0018 (needs a user decision). Candidates: R6-A6-2.*

#### 345. Rescan cache reuses chart names and hashes with no reader stamp

The question: when the code that reads a chart's hash, title, artist or charter changes, does a rescan pick up the new values for files that did not change? Mostly not.

`sig_of` in `src/app/analysis.cpp` (around line 267) builds the cache key from file size and modified time only. On a match, `discover_charts` (around line 427) takes the hash, title, artist and charter straight from the cache. It skips the INI, .sng and .srb readers. `RecordStore::chart_library_cache` in `src/store/record_store.cpp` (around line 1649) returns every cached row with no version check. Only titles get a later patch: `title_or_unknown` (around line 479), added in 06a178c.

| Reader change | Unchanged files after rescan |
|---|---|
| none | same as fresh |
| blank-title fallback | patched, same as fresh |
| artist, charter, .sng/.srb or hash | keep the old value |

Today the cache matches the readers, as far as git shows.

After an artist or charter reader change, library rows, report names and search would mix old and new spellings. Which one a chart shows would depend on whether its file was touched since its last scan. `hydra_batch` reads the same cache.

The cache came with the C++ port (3b69958). The user did not ask for reuse-until-the-file-changes. The verifier searched `docs/adr/`, CONTEXT.md, the user guide, `docs/development.md` and the history of `sig_of`, and found no decision. ADR 0018 says every kind of stored computed data carries a stamp. A metadata stamp, or an ADR accepting the reuse, would close the gap.

*Owner proposed: A charts-metadata StampRule in src/store/stored_versions.h, checked in RecordStore::chart_library_cache (or an ADR accepting reuse). Candidates: R6-A6-3.*

### Visibility 4: not visible (internal or test-only)

#### 346. A backend row with no stored offset is read as 0 ms in seven places

The question is what a backend row means when it has no stored `offset_ms`. A backend row is a note near the SP end, with its distance from that end in ms.

Seven places answer "treat it as 0 ms" with `value_or(0.0)`. They are `Engine::create_deactivated_path` in `src/search/engine.cpp` (around line 628), `BackendSqueeze::summarystr` (around line 312) and `Activation::display_backends` (around line 591) in `src/core/model.cpp`, `build_activations` in `src/app/path_view.cpp` three times (around lines 288, 293 and 313), and `squeeze_sentences` (around line 80). `rate_activation` in `src/core/squeeze_rating.cpp` skips such a row instead (around line 106). Its cap-clamped loop (around lines 183-185) also treats the row as 0.

They agree on screen today. The verifier showed by reading that skipping and reading 0 give the same visible result, because no scale is ever material at 0 ms. The claim that `rate_activation` shows something different was refuted. The engine never leaves `offset_ms` unset (`graph.cpp` around lines 238 and 357, `engine.cpp` around line 1204).

Only a hand-built or corrupt row reaches this. It would show a confident 0.0 ms Standard row.

Core should own it: make `offset_ms` non-optional on stored rows, or give one accessor. It was introduced in 3b69958 (2026-08-15, the C++ port), with later sites in ba0885a, 133b52e and 8668c04. No user decision was found in `docs/adr/`, CONTEXT.md, or the commit messages read.

*Owner proposed: core: make offset_ms non-optional on stored rows, or one accessor that answers it. Candidates: backend-10.*

#### 347. Preview guesses a 4-bar cap for a record missing one, which can't happen

The question is which SP cap the Preview gauge uses when a Ready record carries none.

`render_preview_panel` in `src/ui/preview_tab.cpp` (lines 194-196) uses `record->sp_cap.value_or(kCloneHeroSpCap)`, so it falls back to 4 bars. `build_preview_scene` in `src/app/preview_view.h` (lines 345-347) has the same default argument. The two production callers in `preview_load_job.cpp` always pass a cap, so only tests reach that default.

No stored record reaches the fallback. The store's save throws without a cap (`record_store.h` line 155). `rebuild_record` reads it from a format-6 blob that only that save writes. `pather.cpp` always sets it. So the fallback guards a case that cannot occur. It does use the one named constant, not a bare 4.

Nothing shows on screen today. If a capless record ever appeared, the gauge would draw 4 bars whatever cap the run used.

The store should own this guarantee, and the Preview should read the cap directly. Introduced by aab5ff1 (2026-08-31, then in `details_view.cpp`), moved by 2facc04 (2026-09-27). The user did not ask for it. The verifier found no decision in `docs/adr/`, CONTEXT.md lines 206-214, the Preview plan headers, or either commit message (both Claude co-authored).

*Owner proposed: Store: every Ready record carries sp_cap, so the Preview can read it with no fallback. Candidates: spwin-14.*

#### 348. Half-millisecond slack for "playhead is on an activation" has no recorded choice

The question is: when is the Preview playhead "on" an SP activation?

One constant answers it: `kOnActivationMs = 0.5` in `src/app/preview_view.cpp` (line 576). Two functions read it. `activation_jump_ms` (around lines 579-589) skips any activation within half a millisecond of the playhead, in both directions. `build_next_act_box` (line 596) still names an activation as next if it is up to half a millisecond behind the playhead. One constant with two readers is not a duplicate. `build_drain_box` (line 535) asks a different question, whether SP is running, with no slack. It agrees: half a millisecond past an activation, both boxes describe the same activation.

The finder claimed two activations one tick apart could merge. The verifier showed that cannot happen. An activation needs half a meter, and SP drains at two measures per bar (`kMeasuresPerSpBar` in `core/timing.h`, around line 128). So activations sit at least 4 measures apart. That is far more than half a millisecond at any tempo. The slack only absorbs rounding noise when the playhead is parked on an activation. Nothing shows on screen.

It came in with 59a3dd9 (2026-09-27). It also appears as code in the plan `docs/superpowers/plans/2026-09-27-ui-redesign.md` (around line 9326). No user decision was found in docs/adr/, CONTEXT.md, the commit message or that plan. The constant is already the single owner.

*Owner proposed: kOnActivationMs in src/app/preview_view.cpp (single owner). Candidates: tempo-24.*

#### 349. Rules fingerprint encoding is a code choice, not in ADR 0014

The question is how the rules become the stored fingerprint. A fingerprint is a short number that changes whenever any rule changes.

Only `src/core/rules.cpp` answers it. `add_line` (line 12) writes each field as `name=value` with 17 significant digits. `fnv1a64` (lines 16-23) hashes that text with FNV-1a 64, a common fast hash. `hash_rules_text` (lines 45-48) turns a hash of 0 into 1. `Rules::retired_auto_fingerprint` (lines 54-60) keeps a literal ladder text for the removed Auto cap. Tests in `tests/test_rules.cpp` (lines 247-248) freeze the results.

There is one owner, so nothing can drift, and the user never sees it.

The encoding came in with 7e14401 on 2026-09-24; the ladder text took its current form in eaf3b73 on 2026-09-27. The user asked for a stored fingerprint (2026-09-24 plan, decision 15). The encoding itself was not decided. ADR 0014 says only that records carry "the fingerprint of the rules". It never mentions FNV, the digit format or the 0-to-1 move. The verifier also checked CONTEXT.md and the plan headers.

*Owner proposed: core/rules (already the only place). Candidates: store-23.*

#### 350. Dynamics bad-blob test assumes the stamp is byte 0 and 2 is rejected

The question is whether a stored dynamics blob is current. A "blob" here is the packed bytes of a song's ghost and accent counts. `encode_dynamics` in `src/app/dynamics_breakdown.cpp` writes the stamp `kDynamicsBlobStamp.written` as the first byte. `decode_dynamics` checks that byte against the accepted list, which is {1} in `src/store/stored_versions.h`. The test "dynamics decode rejects bad blobs" in `tests/test_dynamics_store.cpp` (around line 91) hard-codes `blob[0] = 2` and expects a rejection.

It agrees today, shown by reading: 2 is not in the accepted list. It would break on a stamp bump. With the stamp moved to 2 and only 2 accepted, the test's blob would decode and the check would fail. The fix is the pattern the count-stamp test (around line 235) already uses: stamp `kDynamicsBlobStamp.written + 1`. Nothing reaches the screen.

Two other parts of the original claim were dropped. The old-schema table in the test is a fixture of a schema that no longer exists in `src`. The hand-built difficulty strings are opaque to the store, so they re-derive nothing.

The literal came in ec3ade9 (2026-09-21). The user did not ask for it. ADR 0018 puts the dynamics stamps in `stored_versions.h` (citing a user decision of 2026-09-27) but says nothing about tests knowing the blob layout. ADR 0012, CONTEXT.md and the b66abda message were also checked.

*Owner proposed: store::kDynamicsBlobStamp (src/store/stored_versions.h). Candidates: sweep-tests1-c14.*

#### 351. Batch 'time left' waits for three finished charts

The question is when a batch analysis starts showing "about X left".

One place answers it. `kEtaMinFinished = 3` in `src/ui/library_jobs.h` (line 88) is applied only by `batch_eta_s` in `src/ui/library_jobs.cpp` (lines 132-135). `src/ui/library_dialogs.cpp` (line 396) just formats the result. `tests/test_library_jobs.cpp` (lines 98-104) pins the edge with 2 and 3, which is a test pin, not a second rule. The verifier found no other time-left calculation.

| Elapsed 30 s | Shows |
|---|---|
| 2 of 10 done | nothing |
| 3 of 10 done | 30 / 3 x 7 = 70 s |
| 10 of 10 done | 0 s |
| total 0, or done above total | nothing |

There is nothing to disagree with, so there is no drift. On screen, the estimate stays hidden until the third chart finishes.

The threshold came in e6d8159 (2026-09-27). The literal first appears in plan commit f1c6a3d, in `docs/superpowers/plans/2026-09-27-ui-redesign.md` (line 4755). Nothing in docs/adr/ or CONTEXT.md covers it, and the commit message has no user quote. The closest thing to a decision is design prose: `docs/superpowers/specs/2026-09-27-ui-redesign-design.md` (line 81) says the estimate appears once three charts have finished. The plan header calls that spec "the design approved on 2026-09-27". So the number traces to an approved design spec, but not to an ADR, a CONTEXT entry or a direct user quote. `batch_eta_s` is already the right owner.

*Owner proposed: batch_eta_s / kEtaMinFinished in src/ui/library_jobs (already the single owner). Candidates: sweep-tests2-c14.*

#### 352. Preview overlay test assumes the button list order equals all_paths order

The question is: what order are a record's paths listed in the Preview's path picker?

`build_path_buttons` in `src/app/path_view.cpp` (around line 444) owns the order. It walks `build_path_list`'s groups, which split `all_paths()` in order, then adds the all-0 rows when they are shown. `test_preview_path_overlay` in `tests/ui/uitest_preview.cpp` (line 141) assumes the list "follows all_paths()". It passes an `all_paths()` index to `pick_preview_path` (around line 105), which indexes the buttons.

This holds today. For every index inside `all_paths()`, button i is path i. Shown by reading. `tests/test_path_view.cpp` pins only the first button, not the full order.

The test also retypes the combo item ID, label + "##" + index, which `src/ui/preview_tab.cpp` (line 144) builds too. And it counts the all-0 rows even when they are hidden.

Nothing shows on screen. If the order changed, the test would click the wrong path.

It came in bb65964 (2026-09-27), from plan commit f1c6a3d. No user decision was found. The verifier looked in CONTEXT.md, `docs/adr/`, the commit message and the UI redesign plan.

`build_path_buttons` should own the order. The test should find the button by its path pointer.

*Owner proposed: build_path_buttons (src/app/path_view.cpp). Candidates: sweep-tests4-c16.*

## Round 7 and the 2026-10-03 code (added after delivery)

Five finders ran after delivery. Two read the code committed on 2026-10-03 after the audit's snapshot 707c285. Three ran completeness round 7 by new angles: functions that were only lightly read, every literal in the source, and the screens traced least. Four verifiers then checked every candidate against HEAD 02976c1, and the stop rule is still not met, because round 7 found new items.

The findings here are numbered R7.1 to R7.51 so they do not collide with 1 to 352. Entries R7.1 to R7.44 are findings. Entries R7.45 to R7.51 are notes on what today's commits did to findings that already exist. Within each subsection, the entries are ranked by how visible the effect is to you.

### Drift

These copies give different answers for some input today. Each entry names the input.

#### R7.1 The Preview load and the batch strip show "time left" by two rules and two wordings

The question is how long is left, and how that is worded. Two copies answer it.

The batch strip uses `batch_eta_s` in `src/ui/library_jobs.cpp` (lines 132-135): the average time per finished chart, shown after 3 charts. `format_duration` in `src/ui/library_dialogs.cpp` (lines 66-75, used at line 396) words it as "about m:ss left". The Preview load uses `ByteRateClock::update` in `src/ui/preview_load_job.cpp` (lines 40-59): the last second's byte rate, shown after 3 s. `Progress::time_left_text` (lines 269-277) words it as "about N s left" in whole seconds rounded up under 59.5 s, and as rounded whole minutes above that.

They disagree today. This table comes from a Python port of both rules. It rounds halves away from zero, as the C++ `llround` does. The finder's port used Python's own `round`, which differs only at exact halves such as 150 s.

| Seconds left | Batch strip | Preview load |
|---|---|---|
| 0.0 | about 0:00 left | about 1 s left |
| 30.2 | about 0:30 left | about 31 s left |
| 59.4 | about 0:59 left | about 60 s left |
| 90 | about 1:30 left | about 2 min left |
| 150 | about 2:30 left | about 3 min left |
| 3700 | about 1:01:40 left | about 62 min left |

So the app says "about ... left" in two styles. Finding 351 says the verifier "found no other time-left calculation". That line is now out of date. The Preview rule came in with ae75ef7 (2026-10-03). Its wording and its 3 s and 1 s gates appear only in Task 4's body in the preview-loading plan (line 414), which is an agent's spec and not a decision of yours.

The proposed owner is one time-left formatter in `src/ui`, beside `format_duration`, with you picking the wording. It was shown by reading and by two Python ports.

*Candidates: N2-6, R7-B-4.*

#### R7.2 "Open automatically" says it covers finished batches, but it also opens every leaderboard comparison

The question is which reports the `auto_open_report` setting opens by itself.

The checkbox `Open automatically` in `render_batch_done` (`src/ui/library_dialogs.cpp`, lines 505-508) has the hint "Open the report in the browser whenever a batch finishes". The User Guide (line 211) says the same. The code reads the setting in two places. `AppState::update_background_jobs` (`src/ui/app_state.cpp`, line 466) passes it to the path report. `AppState::start_dm_report` (line 561) also passes it to `DmReportJob`, and `DmReportJob::run` (`src/ui/dm_jobs.cpp`, line 57) opens the comparison page when it is set. The dmleaderboards window has no control for it.

| `auto_open_report` | Path report after a batch | dmleaderboards comparison |
|---|---|---|
| off | not opened | not opened ("The report is ready.") |
| on | opened | opened ("The report opened in your browser.") |

The text promises the second column is unaffected. It is not. This was shown by a run. A verifier re-ran a `hydra_uitest` script that ticks `Open automatically` on the batch strip and then compares with dmleaderboards. The window read "The report opened in your browser." and `state` showed `opened_urls=1`.

On history, the hint is older than the coupling, not newer. The checkbox and the "whenever a batch finishes" hint were already in `library_view.cpp` at 21ddf53 (2026-08-18), when only `ReportJob` read the setting. The DM job started reading it in ba0885a (2026-08-22). The User Guide copied the hint in 4e230b8 (2026-09-27). No ADR or CONTEXT.md entry says which reports the setting covers.

The meaning should be written once, beside `Settings::auto_open_report` in `src/app/config.h`. You choose: either the DM job stops reading it, or the hint and the User Guide say "whenever a report is built".

*Candidates: R7-C2.*

#### R7.3 Two side doors start a library scan in the middle of a batch, and the toolbar button refuses

The question is whether a library scan may start right now. Three places answer it, and none calls the others.

The toolbar button `Scan library`, in `render_actions_row` (`src/ui/library_toolbar.cpp`, line 64), is on only when there is a song folder, no batch is running and no scan job exists. Its tooltip says "Busy: a batch is running." The handler for `request_scan` in `render_main_window` (`src/ui/library_view.cpp`, lines 127-133) checks for folders and an existing scan job, but not for a batch. `AppState::start_scan` (`src/ui/app_state.cpp`, lines 420-425) refuses only an unfinished scan. Two controls send `request_scan`: `Scan now` in the Song folders window (`src/ui/library_dialogs.cpp`, line 164) and `Rescan library` in the song panel (`src/ui/details_panel.cpp`, line 154). `Manage folders...` is never disabled.

| State | Toolbar button | `request_scan` path | `start_scan` |
|---|---|---|---|
| no folders | off | refused | would run |
| batch running | off ("Busy") | scan starts | would run |
| finished scan not yet dismissed | off | refused | would run |
| idle, folders set | on | scan starts | runs |

They disagree on the "batch running" row. During a batch, `Scan library` is grey, yet `Manage folders...` then `Scan now` starts the scan anyway. A verifier read the code for harm and found none. The batch owns its own copy of its chart list (`start_batch` moves `batch_scope` into `BatchJob`, `src/ui/library_jobs.cpp`, lines 243-248), so the scan's rewrite of the `charts` table cannot change what the batch analyzes. `rebuild_chart_library` (`src/store/record_store.cpp`, lines 1618-1657) runs under the store's recursive mutex, the same lock the batch workers take to store results. The costs are smaller. The scan competes for CPU and disk, the workers wait on the lock, and the "Scanning charts" window covers the batch strip until `Continue`. If a folder was removed, the batch keeps analyzing charts that are no longer in the library, and the end-of-batch report is built from the new library.

The overlap was not run, because both side doors need the native folder picker or a missing chart file, and `hydra_uitest`'s library is fixed. The toolbar's batch check came in a591bba (2026-09-27). The `request_scan` handler is older (21ddf53, 2026-08-18). No user decision covers it. It is separate from finding 254, which is about how "is a batch running" is spelled.

The proposed owner is one `AppState::can_scan()` (folders present, no scan job, no batch), which `start_scan` enforces and the toolbar and the handler both call. It was shown by reading.

*Candidates: R7-C1.*

#### R7.4 The settings bar says "this song" is analyzing while the panel shows a different song

The question is which song a running single-song analysis belongs to, as the screen states it.

The owner is `AppState::analyze_job_shown` (`src/ui/app_state.cpp`, lines 260-263). It is true only when the job's chart is the one the panel shows. `render_headline` (`src/ui/details_panel.cpp`, lines 99-102) asks it, and names the other song when it is false. `render_settings_bar` (`src/ui/settings_bar.cpp`, lines 230-232) does not ask. It picks its text only from `batch_running()`, so whenever settings are locked and no batch runs, it prints "Settings are locked while this song analyzes."

They disagree today. A verifier re-ran a `hydra_uitest` script that analyzes "(b) The Decade of Statues" and steps to the next song at once. `state` showed `selected=87` and `analyze=running`. The panel read "Not analyzed yet." and the bar read "Settings are locked while this song analyzes." The headline tooltip named the other song. The wording came in 99cbda2 (2026-09-27), and the User Guide (line 35) quotes it without saying which song it means. The audit mentions `analyze_job_shown` only in finding 143, which is a different question.

The proposed owner is `AppState::analyze_job_shown`. The bar should say "this song" only when it is true, and name the job's song otherwise.

*Candidates: R7-C3.*

#### R7.5 A .chart Offset with a unit is honoured, and a song.ini delay with a unit is dropped

The question is what audio offset a chart asks for. Two parsers read the number with different strictness. Finding 338 covers which of the two wins.

`parse_delay_ms` (`src/app/preview_source.cpp`, lines 130-139) reads song.ini and .sng `delay`. It accepts the text only when all of it is a number. `load_songpath` (`src/parse/song.cpp`) reads the .chart `Offset` first through `try_parse_int` (lines 40-52), which is strict. If that fails, it falls to a bare `std::stod` (line 1157), which keeps the leading number and ignores the rest. A verifier ran the Windows C runtime's `strtod`, which MSVC's `stod` wraps.

| Text | song.ini / .sng delay | .chart Offset |
|---|---|---|
| `0.25s` | absent | 0.25 s = 250 ms |
| `500ms` | absent | 500 s = 500,000 ms |
| `0.25` | 0.25 ms | 250 ms |
| `nan` | NaN, and it wins because NaN is not 0 | NaN |

So an Offset typed with a unit is read as seconds and silently honoured. `Offset = 500ms` would shift the Preview's audio by more than eight minutes. A delay with a unit is dropped and the Preview falls back to the chart. The `nan` row is outside the finder's claim. Both parsers accept it, and a NaN delay wins in `preview_audio_offset_ms` (line 330). This shows only on malformed charts. Hydra itself was not run on such a chart.

The Offset parse came in 1b7eeb3, and the strict delay parse in b868de9. No ADR or CONTEXT.md entry decides how strict the parse should be, and decision 9 in the 2026-09-24 plan header does not either. The proposed owner is one "number from chart text" helper in `core/strutil` that both call, with you choosing the strictness. It was shown by a run of the C runtime's parser and by reading.

*Candidates: R7-A6.*

#### R7.6 Three new lookups assume milliseconds never fall as ticks rise, and a negative .chart tempo breaks that

The question is which activation window or tempo is in force at a given moment. Before 2026-10-03, every copy scanned the whole list. Three new lookups now rely on milliseconds rising with ticks. This extends findings 149 and 159, which already note that time running backwards would split ms tests from tick tests. It also bears on findings 32 and 57.

The first is the open-window walk in `replay_path` (`src/core/replay.cpp`, reasoning in lines 75-91, code in lines 134-167). A window leaves the open list for good once a chord is past its deactivation node and the window does not pay it. The second is the BPM line in `build_time_box` (`src/app/preview_view.cpp`, lines 513-517), a binary search over `scene.tempos`. The third is `build_next_act_box` (lines 654-669), a binary search over `scene.activations`. The old every-window loop survives in the tests as `reference_replay_path` (`tests/test_replay.cpp`, from line 790).

Nothing enforces the assumption. `ChartDataEntry` (`src/parse/song.cpp`, line 786) turns the .chart line `B -60000` into -60 BPM. `op_tempo` stores it as is (line 894), and `MsIndex` (`src/core/timing.cpp`, lines 22-45) builds a negative ticks-per-second, so ms runs backwards after that tick. A .mid tempo is an unsigned microsecond value, so only .chart can do this. `hydra_batch` analyzed such charts with no error.

This was shown on the real binaries. First run: a chart at 120 BPM with `B -60000` at tick 1000 and chords at ticks 0, 480, 960, 990, 1100 and 1500, scored with `hydra_replay score --acts "0:960"`. The chord at 990 (1031.25 ms) is past the leeway, so the window leaves the open list. The chords at 1100 (833.33 ms) and 1500 (0 ms) then get no SP. Without the 990 chord they do. So one chord changes whether later chords are paid. The old loop judges each chord alone and would pay both in both charts.

Second run, against the engine. The chart has an SP fill, `B -60000` at tick 11700 and a return to 120 BPM at 12500. With a chord at 11760, the engine scores 15,150 with 5,400 SP, and the new replay agrees exactly. Without that chord, the engine still scores 14,950 with 5,400 SP. The replay pays every chord from 12000 to 13200 and gives 7,200 SP and 16,750. So on a negative-tempo chart the replay and the engine disagree whatever the code version. When the totals differ, `PathReplay::faithful()` fails and the Preview shows "Score unavailable" (`src/app/preview_view.cpp`, lines 230 and 557).

The BPM line splits too, shown by a Python check of `std::upper_bound`. Take tempos at ticks 0, 1000, 1100, 1200 and 1300 (120, -60, 90, 90 and 90 BPM). Their ms are 0, 1041.67, 833.33, 972.22 and 1111.11. At 900 ms the old front-to-back scan says 120 BPM and the binary search says 90 BPM. The next-activation box was shown by reading only. No chart with a negative tempo was found in `testdata` (a grep for `= B -` in the .chart files). No ADR or CONTEXT.md entry decides the tempo sign.

The walk came in 8bc214f and the two searches in 42f37e0 (both 2026-10-03). The proposed owner is one parser rule where both parsers write `bpm_changes` (`ChartParser::op_tempo` and `MidiParser::op_tempo`, or the `MsIndex` constructor). It would reject or clamp tempos at or below zero. You decide which.

*Candidates: N1-1.*

#### R7.7 Practice sections come sorted from .chart but in track order from .mid, and the new time box keeps the unsorted answer on purpose

The question is which practice section is in force at the playhead. The time box answers "the section before the first one past the playhead, front to back", which is right only when the list is sorted by tick.

`ChartParser::parse` sorts the list with `stable_sort` (`src/parse/song.cpp`, lines 1190-1208). `MidiParser::parse` pass 3 (lines 673-687) appends each track named EVENTS in turn and never sorts. `build_preview_base` copies the list as it is (`src/app/preview_view.cpp`, lines 322-323). `build_time_box` (lines 532-548) checks `std::is_sorted`. A sorted list gets a binary search, and an unsorted list keeps the old scan. The test "preview lookups: searches match the old scans on a busy synthetic chart" (`tests/test_preview_view.cpp`, lines 1857-1878) rotates the sections and requires the old answer. It passes.

This was shown by a run. A verifier built a .mid with two tracks named EVENTS: Intro at tick 0 and Chorus at tick 1920 in the first, Verse at tick 960 in the second. `hydra_replay score` printed the sections as Intro 0, Chorus 1920, Verse 960.

| Playhead tick | Section in force | Time box today |
|---|---|---|
| 1000 | Verse | Intro |
| 2000 | Chorus | Verse |

The same chart saved as .chart would be sorted first and show Verse and Chorus. The time box itself was shown by reading, because `hydra_uitest`'s library is fixed to `testdata/input`. Whether any real chart has two EVENTS tracks was not checked. The audit mentions `practice_sections` only in finding 122, a different question. The MIDI pass dates from aab5ff1 (2026-08-31), and the kept branch from 42f37e0 (2026-10-03). No user decision covers section order.

The proposed owner is the parser. `MidiParser::parse` should sort the way `ChartParser::parse` does, so `Song` carries one "sections are in tick order" rule, and the time box can drop its `is_sorted` branch.

*Candidates: N1-5.*

#### R7.8 A stem's length: the mix trusts the file header, and the old mixer counted what decoded

The question is how many frames a stem has. That sets what the Preview plays and where its audio ends.

`StreamMix::StreamMix` (`src/audio/stream_mix.cpp`, lines 188-217) takes each reader's `length_frames()` as the stem's length. That length caps every read (`StreamMix::read`, line 238) and sets the mix length (line 217). The audio end that `PreviewTransport::load` uses (`src/ui/preview_transport.cpp`, lines 16-17) comes from there. The old path read every frame the decoder gave. At 707c285 `decode.cpp` looped until the decoder stopped, and today's `read_all` (`src/audio/decode.cpp`, lines 39-66) still reads past the promise.

The promise comes from the header. `MaReader` (`src/audio/ma_reader.cpp`, lines 182-184) uses `ma_decoder_get_length_in_pcm_frames`, which for FLAC is the STREAMINFO total as written in the file. `VorbisReader` (`src/audio/vorbis_reader.cpp`, line 44) uses `stb_vorbis_stream_length_in_samples`, which returns 0 when no page is found or the last granule is -1.

| Stem | Old mixer | Preview today |
|---|---|---|
| header length equals decoded frames | N frames | N frames |
| FLAC whose STREAMINFO total is 0 | plays | silent for the whole song, and adds nothing to the audio end |
| FLAC cut short, header promises more | plays to the cut | plays to the cut, then silent up to the header length, so the audio end moves later |
| WAV or MP3 | plays | same as before (dr_wav clamps, and an MP3 length is a frame count) |

A FLAC with total 0 is a real-world case. A run of ffmpeg writing FLAC to a pipe left the total at 0, while the same 3 s sine written to a file read 132,300. Any encoder that writes to a stream it cannot seek back in does this. It is not in your library today. A scan of `C:\Clone Hero` found one FLAC stem, and its total is set. Of 51,840 Vorbis files, none has a last granule of -1. 784 have a last granule of 0, but all are under 10 KB, and the one decoded with ffmpeg holds 0 samples, so old and new agree. One file has no end-of-stream page, `Fall Out Boy - Dance, Dance\drums_1.ogg`. Its last good page ends at 3,500,864 samples, which is what ffmpeg decodes, so the lengths match there too. The Vorbis row is by reading of `stb_vorbis.c` only.

This is not in the audit, which predates `StreamMix`. It came in with fffa79c and a0913d0. The proposed owner is the `StemReader` contract in `src/audio/stem_reader.h`: one rule for "length unknown or wrong", decided once, for example "a 0 length means read to the end". You should pick it. See also R7.37 and R7.38.

*Candidates: N2-1.*

#### R7.9 After a decode error, seeking back revives every stem except an Opus one

The question is whether seeking back before a decode error brings a stem back.

`OpusReader::decode_next` (`src/audio/opus_reader.cpp`, lines 380-383) sets `failed_` and `at_end_`. `OpusReader::seek` returns before touching `at_end_` when `failed_` is set (line 286), and `read` stops on `at_end_` (line 277). So the stem stays silent after any seek until the chart is reopened. The other readers recover: `MaReader::seek` (`src/audio/ma_reader.cpp`, line 218), `VorbisReader::seek` (`src/audio/vorbis_reader.cpp`, line 82) and `Mp3Reader::seek` (`ma_reader.cpp`, lines 388-411) all reset `at_end_`. The mixer would replay the stem, because `StreamMix::Stem::seek_to` clears `ended` (`stream_mix.cpp`, line 139).

| Damaged stem, you seek back before the damage | Opus | Vorbis | MP3 | WAV and FLAC |
|---|---|---|---|---|
| plays again | no | yes | yes | yes |
| `failed()` reports it | yes | no | no | yes |

`MaReader` keeps `failed_` sticky even though it plays again, so `failed()` means "ever failed" there. For Opus it means "dead". MP3 seldom goes silent at damage at all, because dr_mp3 skips a bad frame (comment at `ma_reader.cpp`, line 16). ADR 0019 (lines 81-87) approves "plays up to the damage, then goes silent". No document decides the seek-back case. This is narrow on screen. The library scan cited in ADR 0019 found no damaged file. It was shown by reading, and no damaged fixture was run. It came in with a0913d0 and 9f9c759.

The proposed owner is the `StemReader` contract in `stem_reader.h`, with one stated rule for `failed()` and for seek after failure.

*Candidates: N2-2.*

#### R7.10 For SP that outlasts the chart, the graph and the display apply the 500 ms window from different anchors

This extends finding 30. The question is which backend rows survive for the last activation of a path whose Star Power outlasts the chart.

The graph keeps the notes within 500 ms of the song's last note (`ScoreGraph::tail_backends`, `src/search/graph.h`, lines 121-127). `rebuild` (`src/search/engine.cpp`, lines 1196-1206) then measures every kept tail note against `final_sp_end`, which lies after the last note. `Activation::display_backends` (`src/core/model.cpp`, line 591) drops any row at or beyond 500 ms. `write_activation` (`src/store/path_codec.cpp`, line 113) and `rate_activation` (`src/core/squeeze_rating.cpp`, line 100) both read `display_backends()`.

For rows on a deactivation edge, the display check changes nothing, because the graph already keeps only rows within 500 ms of the SP end (finding 48). For tail rows, the display check is the only filter. A tail row survives today only when the SP ends less than 500 ms after the last note. With the last note at 10,000 ms and the SP end at 10,800 ms, every tail note lies between 9,500 and 10,000 ms, so its offset is between -1,300 and -800 ms, and the display keeps none. The finder's second table row, a note at 10,350 ms, cannot happen.

The point for the fix plan is this. Finding 30 treats the dropped tail rows as the bug, and cites the test at `tests/test_squeeze_rating.cpp` (line 689). This item treats the drop as correct. Which is right is the "one agreed anchor" question that finding 30 leaves to you. If finding 30 is deduplicated by deleting the display-side check, a tail activation would store and show rows as far as (SP end minus last note) plus 500 ms before the SP end. Rows already stored would not change, because they were filtered when written. Only records written after the change would gain them.

It was shown by reading and hand arithmetic, and not run. The proposed owner is the one window helper finding 30 proposes, applied in `rebuild` against `final_sp_end` when it copies `tail_backends`.

*Candidates: R7-A5.*

#### R7.11 MidiFile::from_file sizes a file with the 32-bit ftell that the 2 GB fix removed elsewhere

The question is how to read a whole file's bytes. `read_file_bytes` in `src/core/winstr.cpp` (lines 146-161) is the owner. It uses `_fseeki64` and `_ftelli64`, and its own comment names the failure. `MidiFile::from_file` (`src/parse/midi.cpp`, lines 169-185) re-implements it with `std::fseek` and `std::ftell`, whose result is a 32-bit `long` on Windows. It is the production path for .mid charts: `load_songpath` calls `load_songpath_mid` (`src/parse/song.cpp`, lines 1234-1236 and 1331).

At 2 GiB or more, `ftell` returns -1, the buffer comes back empty, and the parse fails as "not a MIDI file" instead of a size error. The two failure wordings also differ ("cannot open MIDI file:" against "cannot open file:"), but `plain_error_text` maps both to one sentence. No real .mid is that big, so they agree for every real chart. This was shown by reading. The audit has no hit for `from_file`. It dates from 3b69958. The proposed owner is `read_file_bytes`, with `from_file` becoming `MidiFile(read_file_bytes(path))`.

*Candidates: N2-5.*

### Duplicates that agree today

Each entry is a second copy of a rule. They agree today. The entries are ranked by how visible a split would be.

#### R7.12 "Is this path too long?" is asked with two cut-offs, the shell edge is typed four times in src and three in a test, and the prefix is spelled twice

The question is when a path counts as long. `win32_path` in `src/core/winstr.cpp` (lines 39 and 48) says 248 and up (`kPlainPathLimit = MAX_PATH - 12`). `shell_path` (lines 68 and 79) says 260 and up. `open_in_browser` in `src/app/report_files.cpp` (line 115) asks `path.size() < MAX_PATH` itself before calling `shell_path`. `copy_to_short_temp` (line 105) asks `>= MAX_PATH`. In `tests/test_long_paths.cpp`, line 236 rebuilds `shell_path`'s output rule (`raw.size() - 4 < MAX_PATH`), and lines 241 and 257 pin `< MAX_PATH` again. No other path-length check against `MAX_PATH` was found in `src` or `tools`. Lines 100-102 of `report_files.cpp` size the `GetTempPathW` buffer, which is a different question. The `\\?\` and `\\?\UNC\` prefix is added in `win32_path` (lines 49 and 58-60) and stripped with separate literals in `shell_path` (lines 76-77).

| Path length | `win32_path` | `shell_path` | `open_in_browser` |
|---|---|---|---|
| 247 | plain | plain | shell |
| 248 to 259 | prefixed | plain | shell |
| 260 or more | prefixed | short 8.3 name, or empty | short name, else temp copy |

Every copy agrees today, so nothing differs on screen. The check in `open_in_browser` is not dead code. It picks ShellExecute for short paths and the direct browser launch for long ones, and "branch on whether `shell_path` changed the path" keeps that. The 248 edge is undecided. ADR 0020 gives only 260, the commit message of 11db329 has no quote from you, and a grep of `docs/adr` and `CONTEXT.md` for 248, `kPlainPathLimit` and "MAX_PATH - 12" found nothing. The shell-limit half is already written up in the addendum at the end of section 4, but not as a numbered finding. That addendum says verifier V2 was still checking. V2 has since confirmed it. The 248 cut-off and the twice-spelled prefix are new. The 248 edge came in 11db329, and the `MAX_PATH` checks in `report_files.cpp` came in 02976c1.

The proposed owner is `core/winstr`. The two cut-offs become named constants side by side, ADR 0020 records the 248 edge and why, and one predicate such as `fits_shell` serves `shell_path`, `open_in_browser`, `copy_to_short_temp` and the test. See also R7.40. It was shown by reading.

*Candidates: N2-3, R7-A1, R7-B-3, R7-B-6.*

#### R7.13 The state of a span at an instant is worked out twice in track_state.cpp

The question is whether a span field (SP phrase, solo, fill, taken fill, SP running, fill lane) is On, Start, End, Restart or Empty at instant t. `toggle_at` (`src/render/track_state.cpp`, lines 248-262) scans every interval. `TrackState::synthesize` (lines 379-387) still uses it for a window that holds no stored instant. `SpanSweep::at` (lines 86-104) answers the same question by counting sorted starts and ends, and `sweep_span_fields` (lines 230-246) uses it for every stored instant.

They agree today. A verifier re-read the finder's Python port against the code, including NaN and malformed-interval handling, and ran it: 840,000 random queries, 0 mismatches. The edge rows agree: `[1,3]` at 2 is On, `[1,2]` and `[2,3]` at 2 is Restart, a zero-length `[2,2]` inside `[1,3]` at 2 is On, and a NaN start with end 2 at 2 is End.

If one copy drifted, a span would draw one way at a stored instant and another way in a window between instants, and would flicker on or off while the view sits between two instants. The header comment calls `toggle_at` "the reference rule", but a code comment is not a decision of yours. The proposed owner is one rule: either `synthesize` asks a one-query `SpanSweep`, or `toggle_at` moves to the tests as the oracle only. It came in with 8528da9 (2026-10-03). It was shown by a Python port and by reading.

*Candidates: N1-2.*

#### R7.14 The taken-fill lane tie-break is written twice in track_state.cpp and a third time in the draw code

This extends finding 76. The question is which taken fill's pad colours the fill lane at instant t. The rule is: the lowest list index that starts at t or holds t wins, and failing that, the lowest that ends at t. `TrackState::synthesize` (`src/render/track_state.cpp`, lines 388-403) writes it as two scans. `LaneSweep::at` (lines 133-153) writes it again with an active set. `build_highway_draws` in `src/render/highway_draw.cpp` keeps its own tie-break (finding 76).

The same port run as R7.13 covers them, with 0 mismatches. The port calls `LaneSweep::at` on every instant, while the C++ calls it only when the lane toggle is not Empty. That does not change the answer, because the active set only grows by start and shrinks by end as t rises. If one copy changed, a touching or overlapping taken-fill lane would get a different colour at a stored instant than in an empty window. The proposed owner is as finding 76 says: `TrackState` should carry the pad on each span, so no instant has to pick. It came in with 8528da9. It was shown by a Python port and by reading.

*Candidates: N1-3.*

#### R7.15 The rule that turns moments into timeline instants is written twice

The question is which instants the highway timeline holds, and what time each carries. `build_track_state` (`src/render/track_state.cpp`, lines 264-338) sorts all events and groups equal times. `rebuild_overlay_fields` (lines 340-377) groups again, using the same "time of the last moment" rule and the same NaN skip. The overlay field list `{fill_, fill_taken_, sp_active_, fill_lane_ivs_}` is typed at line 294 and again at line 345.

They agree today, by reading. The two equality tests that hold `rebuild_overlay_fields` to a full build, in `tests/test_track_state.cpp`, pass today (2 cases, 6,050 assertions). If a new overlay span were added to one list only, a path change would give a different highway from a fresh load of the same path. The proposed owner is `build_track_state`: it builds the path-free instants and then calls the same overlay merge that `rebuild_overlay_fields` uses, so grouping and the field list live once. It came in with 0d0bfe9 (2026-10-03).

*Candidates: N1-4.*

#### R7.16 The Preview's highway options are built by hand four times and compared on one field twice

The question is which `render::TrackStateOptions` the Preview draws with. The struct is built from `pro_` in `PreviewLoadJob::run` (`src/ui/preview_load_job.cpp`, lines 184-185) and in `PreviewController` (`src/ui/preview_controller.cpp`, lines 73-74, 144-145 and 214-215). Two places decide "built for the same options?" by comparing `.pro` alone: `PreviewController::render` (line 220) and `PreviewSceneJob::run` (`preview_load_job.cpp`, line 326).

They agree today because the struct has one field (`src/render/track_state.h`, lines 53-55). If a field were added, a stale timeline built for other options would be reused without anyone noticing. The `.pro = pro_` lines trace to 9b12344 and 0d0bfe9 (2026-10-03). 8528da9 was not checked. The proposed owner is one `PreviewController::track_opts()` and an `operator==` on `TrackStateOptions`. It was shown by reading.

*Candidates: N2-11.*

#### R7.17 A load asks "is this a container, and which one?" in three ways

This extends finding 187. The question is whether a notes path is a .sng, a .srb or a loose chart. `chart_format_of` (`src/parse/chart_files.cpp`, lines 9-15) is the owner. `is_container_path` (`src/app/preview_source.cpp`, lines 356-358) tests the `.sng` and `.srb` suffixes itself. `resolve_preview_song` (line 380) and `resolve_preview_stems` (line 395) test `.sng` again and treat anything else as .srb. Meanwhile `load_songpath_from_bytes` asks `chart_format_of` (`src/parse/song.cpp`, line 1318).

The suffixes and the case folding match everywhere today, by reading. If `chart_format_of` gained a third container type, `is_container_path` would call it a loose chart, and the Preview would look for loose audio and play silence. Finding 187 cited the older suffix checks in `resolve_preview_source`. Those moved into these three functions, and `is_container_path` is new in 3686dd6 (2026-10-03). The proposed owner is `chart_format_of`, asked once per load.

*Candidates: N1-7.*

#### R7.18 The Preview overlay box sizes are worked out twice, once to fit and once to draw

The question is how big each Preview text box is at a given scale. `render_preview_panel` in `src/ui/preview_tab.cpp` answers it twice. The fit pass (lines 396-442) measures at scale 1 to choose the overlay scale. The draw pass (lines 446-587) types every formula again. Every pair matches: line height `* 1.25f` (lines 401 and 450), score text `* 1.8f` (409 and 471), score line `* 1.2f` (414 and 472), time box size (406-407 and 457-458), score box width (415 and 478), and drain box size (421-422 and 575-576).

They agree today, because ImGui text width grows in step with font size. If one copy changed, `overlay_scale` would fit boxes of a different size than the ones drawn, and a box would spill onto the highway or shrink for no reason. On history, the fit pass and both drain-box heights came in 76c5530 (2026-09-27). Only the time box's `size * 1.25f` goes back to ba0885a. No audit finding covers this sizing. The proposed owner is one small box-measuring helper beside `overlay_scale` that both passes call with a scale. It was shown by reading every pair.

*Candidates: R7-B-2.*

#### R7.19 The MIDI parser lists its drum pitches three ways

The question is which MIDI pitches the parser acts on, and how. `is_handled_note` (`src/parse/song.cpp`, lines 370-382) lists the difficulty's five pitches from `base` to `base+4`, plus 95, 103, 109-112, 116 and 120. `MidiParser::optype` repeats the set in a note-on switch (lines 476-499) and a note-off switch (lines 503-522). Between them sits a bare gate, `if (is_noteoff && note < 103) return {};` (line 465). The Expert base 96 is typed in `difficulty_base_pitch` (lines 357-364) and again as `int base_ = 96` (line 434).

They agree today. Every note-on case is in the list, every note-off case is 103 or higher, and every pitch the gate drops has no note-off case anyway. If someone adds a marker pitch to one list only, the parser would silently ignore it. The pitch set, the switch and the gate come from the Python parser `hydra/hysong.py`, where a comment said "Every note with a note-off case is >= 103". The C++ gate came in 1f38f07 (2026-08-13), `is_handled_note` in 3b69958, and `base_ = 96` in 133b52e (2026-08-28). The comment did not survive the port, so the gate's reason is now unwritten. The audit covers other questions about these functions, not this three-way listing. The proposed owner is one pitch table in `song.cpp` that all three read, with the note-off gate derived from it. It was shown by reading and a truth table.

*Candidates: R7-B-1.*

#### R7.20 Error matching has fallen behind its throwers (extends finding 193)

The question is whether an error reads as a plain sentence. Finding 193 says `plain_error_text` (`src/app/user_messages.cpp`, lines 81-146) matches each thrower's exact wording. Two newer drifts sit under it.

The first is "cannot read file size: ". Commit adee9a6 (2026-10-03, "Read files over 2 GB") added it in `read_file_bytes` (`src/core/winstr.cpp`, lines 151-156) and `file_size_bytes` (lines 139-144). The matcher has no entry, so it falls to `kSomethingWentWrong`. `file_size_bytes` has one caller, `PreviewLoadJob::open_audio` (`src/ui/preview_load_job.cpp`, lines 95-99), which catches the error and uses a size of 0. That throw also fires on a plain missing file, so a future caller that surfaced it would turn "may have been moved" into "Something went wrong". The `read_file_bytes` throw can reach you through the chart parsers (`src/parse/song.cpp`, lines 1242, 1307, 1312 and 1335) and `read_song_ini_keys` (`src/app/analysis.cpp`, line 249). A single Analyze would then say "Could not analyze <title>. Something went wrong." (`app_state.cpp`, line 329). The batch strip would show the same sentence, with the details line "<title>: cannot read file size: <path>" (`library_jobs.cpp`, lines 287-288).

The second is the audio mixer. `StreamMix` throws "StreamMix: invalid output format" and "StreamMix: data converter init failed" (`src/audio/stream_mix.cpp`, lines 83, 86 and 186), and the matcher has no entry. The matcher's "mix_stems:" prefix is dead, because `mix_stems` is now only the test reference (`src/audio/mixer.h`, lines 12-15). The whole "decode_audio:" branch is dead outside tests too, because `decode_audio` and `decode_stem` have no callers in `src`, and `open_audio` swallows every stem-open error (`preview_load_job.cpp`, lines 127-130).

You would not see either today. The `StreamMix` failure happens inside `PreviewLoadJob::run`. `run_guarded` (`src/ui/job_base.h`, lines 79-88) keeps both the raw text and the plain text, but `PreviewController::poll` copies the raw text (`preview_controller.cpp`, line 187), and `preview_tab.cpp` (line 214) prints "Preview failed: StreamMix: ...". That matches finding 193's own point that the matcher's Preview branch is dead. A verifier ran `hydra_tests -tc="user_messages*,file_size_bytes*,read_file_bytes*"`: 10 cases passed, including "anything else falls back". That confirms the fallback. It does not fire the `read_file_bytes` path, because no way was found to make a 64-bit seek fail on a file `fopen` already opened. The typed error kinds that finding 193 proposes would cover both.

*Candidates: R7-A2, N2-9.*

#### R7.21 The front-pad ms-to-frames conversion moved and was not removed (extends finding 182)

Finding 182 lists `pad_front_ms` in `src/audio/mixer.cpp` as one of four hand-written ms-to-frames conversions. Commit ae75ef7 (2026-10-03) deleted `pad_front_ms`, and its message says "decode_and_mix and pad_front_ms are gone". The same formula now sits in `PreviewLoadJob::run` (`src/ui/preview_load_job.cpp`, line 199): `std::llround(-offset_ms * kOutRate / 1000.0)`. `Playhead::seek_ms`, `position_ms` and `length_ms` (`src/audio/player.cpp`, lines 29, 33 and 37) keep their copies. So there are still four.

There is one new point for the owner. Finding 182 chose an owner in `src/audio` because "every caller already lives in that folder". One copy now lives in `src/ui`, so the owner must be callable from the UI too. Nothing differs on screen today. It was shown by reading.

*Candidates: R7-B-5.*

#### R7.22 The "at least one worker" floor is written three times, and the worker count is only partly decided

The question is how many workers hash the library and analyze charts. The floor of 1 is written in `batch_worker_count` (`src/app/analysis.cpp`, line 489), in `run_work_pool` (`src/app/work_pool.h`, line 52) and in `BatchJob::set_analyzer_for_test` (`src/ui/library_jobs.cpp`, line 155). They agree today. `run_work_pool` guards every caller, so the other two are redundant. This is low priority.

The cap of 8 is your decision. Decision 22 in the header of `docs/superpowers/plans/2026-09-24-derivation-fixes.md` says the 8 batch workers "stay fixed, each with a code comment saying why". Commit c89596b (2026-09-25, plan Task 18) added that comment above `kBatchMaxWorkers` (`analysis.cpp`, lines 481-484). The value itself dates from 3b69958. Three parts of `batch_worker_count` (lines 486-490) are not covered by that decision: "hardware threads minus one", the guess of 1 when Windows reports 0, and the use of the same count for the scan's hashing pool (`discover_charts`, line 386). The decision says only "batch workers". The proposed owner is `batch_worker_count`, with `run_work_pool` and the test seam trusting the count they are given. It was shown by reading and `git log -S`.

*Candidates: R7-A3.*

#### R7.23 An ID3 tag in front of FLAC is MP3 to one rule and FLAC to another (extends finding 74)

The question is which format a stem's bytes are. `sniff_format` (`src/audio/decode.cpp`, line 85) calls any ID3 start MP3, and `open_stem_reader` (`src/audio/stem_reader.cpp`, line 13) dispatches on that. `open_ma_reader` (`src/audio/ma_reader.cpp`, lines 528-535) then sniffs a second time and overrides the answer with `flac_behind_id3` (lines 514-524, from 9f9c759), which parses the tag size itself and calls the stem FLAC.

Three more copies of the ID3 tag-size rule exist. One is a test helper, `long_untagged_mp3` (`tests/test_stem_reader.cpp`, lines 477-479), which ignores the footer flag. The other two sit inside vendored miniaudio: dr_flac (`miniaudio.h`, lines 89745-89762) and dr_mp3 (lines 94841-94849). There is one input where they differ: two stacked ID3 tags in front of `fLaC`. dr_flac skips every tag in a loop, but `flac_behind_id3` looks past only one, so the stem goes to `Mp3Reader` first. The result is probably still FLAC, because `Mp3Reader` most likely fails to open and the code falls back to `MaReader`. That is by reading only. No visible effect was found. The proposed owner is `sniff_format`, which should return Flac for an ID3-wrapped FLAC, and `open_ma_reader` should not sniff again.

*Candidates: N2-7.*

#### R7.24 The MP3 bit-reservoir rule is written twice in one file, with its 511 typed twice

The question is how many reservoir bytes dr_mp3 holds after a frame whose audio was not decoded. `simulate_reset` (`src/audio/ma_reader.cpp`, lines 335-337) and the start search in `Mp3Reader::build_seek_points` (lines 429-434) each write `min(kMaxReservoir, min(reserv, back) + own)`, and both test `reserv >= back`. The formula matches dr_mp3's own `L3_restore_reservoir` and `L3_save_reservoir` for that case (`miniaudio.h`, lines 93796-93819). `kMaxReservoir = 511` (line 274) restates `MA_DR_MP3_MAX_BITRESERVOIR_BYTES` (line 52). That macro is itself a verbatim copy of `miniaudio.h` line 62689, guarded by a version `static_assert` on line 47.

They agree today, by reading. If one copy changed, the seek points would predict a landing frame dr_mp3 does not reach. `landed()` would catch that and fall back to a slow exact seek, so you would see slow seeks, not wrong audio. All of it came in with 9f9c759. The proposed owner is one `reservoir_after(reserv, back, own)` helper used by both loops, with `kMaxReservoir` defined from the macro.

*Candidates: N2-8.*

#### R7.25 A loose stem's file size is read twice per Preview load

The question is how many compressed bytes a loose stem has. That drives the "Opening audio: X of Y MB" bar. `PreviewLoadJob::open_audio` (`src/ui/preview_load_job.cpp`, line 96) sizes it with `file_size_bytes` (`GetFileAttributesExW`). `MappedFile::open` (`src/audio/mapped_file.cpp`, line 25) sizes it again with `GetFileSizeEx`, and the Opus index reports progress against that second size (`opus_reader.cpp`, lines 101, 112 and 208). The progress callback drops the reader's total and clamps its "done" to the first size (`preview_load_job.cpp`, lines 121-122). `progress()` (line 223), `fraction()` (line 241) and `label()` (line 259) clamp again.

The two sizes differ only if the file changes while it opens, so nothing differs on screen. The `file_size_bytes` call came in with ae75ef7. The finder also listed the wider "how big is this file" question, with readers in `read_file_bytes` (`_fseeki64`/`_ftelli64`), `ImFileGetSize` (`src/ui/imgui_files.cpp`, lines 22-28) and `list_dir`'s find data (`winstr.cpp`, line 117). Each sits next to an open it needs, so only the Preview pair is a true double read. The proposed owner is `MappedFile`, which already knows its size. The job should map first and take the total from `bytes.size()`. It was shown by reading.

*Candidates: N2-4.*

#### R7.26 The meta key "engine_mode" is typed three times in record_store.cpp (extends finding 54)

Finding 54 names the "ch10" literal. The key `"engine_mode"` is also typed three times in `src/store/record_store.cpp`: line 644 in the schema 3 migration, line 705 in the getter and line 710 in the setter. There is no named constant. The migration also reads the key itself instead of calling the getter at line 705, so "read the stored stamp" is answered twice in one file. A grep of `src`, `tools` and `tests` found no other copy. The key came in with 406d473 (2026-08-31). They agree today. It was shown by reading.

*Candidates: Extension of finding 54, angle B.*

#### R7.27 The Preview time box types "m1.1.0" twice instead of asking format_measure (extends finding 184)

`build_time_box` (`src/app/preview_view.cpp`, lines 503-504) types `"m1.1.0"` twice as the fallback when the scene has no timing. `format_measure` (`src/app/path_view.cpp`, lines 25-31) gives "m1.1.0" for tick 0, so they agree today. If the numbering rule changed, the time box on a scene with no song would keep the old spelling. The fallback is reached only by a default-built scene, with no song loaded. It came in with 59a3dd9 (2026-09-27). It was shown by reading.

*Candidates: Extension of finding 184, angle B.*

#### R7.28 The stem converter's settings and passthrough test are written twice (extends finding 278)

The question is how a stem is converted to the output format, and who picks the resampler. `StreamMix::StreamMix` (`src/audio/stream_mix.cpp`, lines 196-204) and `convert_stem` (`src/audio/mixer.cpp`, lines 23-30) each write the "already at the output rate and channel count" test and the same `ma_data_converter_config_init` call. `mix_stems` has no caller in `src` (`mixer.h`, lines 12-15 call it the test reference), so production has one copy and the test oracle has the other. Finding 278 judged an oracle like this deliberate. The new point is that the converter settings are copied rather than shared.

The settings are miniaudio's defaults: a linear resampler, low-pass filter order 1, and no dither (`miniaudio.h`, lines 55905-55931). A grep of `docs/adr`, `CONTEXT.md` and the preview-loading plan for "resampl", "lpf" and "linear resampl" found nothing that picks them. There is no visible effect. The proposed owner is one converter-config function in `src/audio` used by both. It was shown by reading.

*Candidates: N2-10.*

#### R7.29 The targeted search's "keep everything" band is a bare billion, retyped in a test

`search_target` sets `options.depth_value = 1'000'000'000` in `src/search/pather.cpp` (line 114), so nothing is pruned. `tests/test_search.cpp` (line 592) types the same value for the same purpose. They agree today. A real chart scores a few million at most, so the band is safe. The pather line came in with 30c4008 (2026-09-03), and the test copy with 124f0f5 (2026-09-26), which only reshaped the pather line. `kEveryPathSentinel = 1000000000` in `src/app/report.h` (line 105) has the same value but answers another question, and finding 86 covers it, so it is not a third copy. The proposed owner is a named constant in `pather.h` that the test reads. This is low priority. It was shown by reading.

*Candidates: R7-B-11.*

#### R7.30 The equivalence tests keep frozen copies of the replaced loops (new instances of findings 164 and 278)

The 2026-10-03 commits proved "identical results" by copying each old loop into the tests as a reference. `reference_replay_path` is in `tests/test_replay.cpp` from line 790 ("copied verbatim", line 778). The `old_scan` namespace is in `tests/test_preview_view.cpp` from line 1518, and it even re-declares `kOnActivationMs`, `clock_str` and `tick_at`. "The old builder", `reference_build`, is in `tests/test_track_state.cpp` from line 70. Each answers a production question a second time.

They agree today. Nothing shows on screen. Findings 164 and 278 already ask whether test oracles are exempt from the derive-once rule, so these are new oracles under that same undecided question. The cost comes later. When a finding such as 32, 57, 175 or R7.7 is fixed in production, these oracles will fail until someone edits them to match, and they pin today's questionable answers, such as the unsorted-section scan. If you rule that oracles are not exempt, they should go once the switch is proven, and the tests should pin literal values. They came in with 8bc214f, 8528da9, 42f37e0 and 0d0bfe9. It was shown by reading.

*Candidates: N1-6.*

### Undecided assumptions

These are choices that no ADR, CONTEXT.md entry or plan header records as yours. Commit messages in this group are agent-written, so they do not count.

#### R7.31 The Path limit is on at 10 ms and the backend display limit defaults to 50 ms, with no recorded decision

The question is what the Path limit and "Hide backend rows beyond" start at. `src/app/config.h` answers it: `mslimit_enabled = true` and `mslimit_value = 10` (lines 68-69), `backendlimit_enabled = false` and `backendlimit_value = 50` (lines 75-76). The 10 ms limit decides which alternate paths are kept on every analysis, so on a new install it changes which paths every analysis stores. No ADR, CONTEXT.md line or plan header decides either default. ADRs 0003 and 0009 mention the ms limit only as part of a record's key. The User Guide (lines 51 and 133) describes both settings without their defaults. The audit's only mentions are findings 322 and 326, which cover ranges and `hydra_bench`.

On history, the 10 ms default did not start in the port or in d49c624. It came from the upstream Hydra author J. A. Salazar's b73597b (2025-08-12, "Added ms filter to UI as the 'ms limit' setting", "Resolves #31"), which set `mslimit_enabled` to True and `mslimit_value` to 10 in `hydra_app.py`. So a human designer chose it, but not you, and nothing records you keeping it. `backendlimit_value = 50` came in 133b52e (2026-08-28, co-authored by Claude), and its message says nothing about 50. That commit's User Guide text said "Off (the default) shows everything", so what you see by default is "off", and 50 shows only once the box is ticked. The proposed owner stays `app::Settings`, with both defaults recorded in CONTEXT.md or the User Guide once you confirm them. It was shown by reading and `git log -S`.

*Candidates: R7-B-8.*

#### R7.32 The Preview loading bar's step shares and units have no decision from you

The question is how far the bar moves for each step, and how its numbers are shown. `src/ui/preview_load_job.cpp` answers with `kReadShare = 0.08f`, `kOpenShare = 0.82f` and `kSceneShare = 0.045f` (lines 28-30). `whole_mb` (line 34) rounds to the nearest 10^6 bytes. `ByteRateClock::update` (lines 40-59) publishes seconds left, worked out from the last second's bytes, at most once a second. `Progress::time_left_text` (lines 269-277) switches from seconds to minutes at 59.5 s. The code comment cites two charts: the blink-182 Discography set the numbers, and Kissing the Shadows is a check. All of it came in with ae75ef7 (2026-10-03), whose trailer names agent abfade81c4380659d and whose message quotes no words of yours.

The principles were adopted. The plan's only decision of yours is "make all of those fixes" (`docs/superpowers/plans/2026-10-03-preview-loading-fixes.md`, line 27). The spec it adopted, `docs/handoffs/2026-10-03-preview-loading-audit.md` section 3 (lines 110-133), asks for steps sized by how long they take, real units and a time left, and gives no numbers. The numbers are in the plan's task criteria (lines 411-414), an agent's spec. The three shares, the MB rounding, the one-second pacing and the 59.5 s switch were not decided by you. On a chart whose time splits differently, such as one with many small stems, the bar would race through one step and stall in another. The time-left wording is R7.1. It was shown by reading, with a search of the ADRs, CONTEXT.md, the User Guide, the plan and the handoff. The proposed owner stays `preview_load_job.cpp`, with your decision recorded if you want one.

*Candidates: R7-B-7, N2-12.*

#### R7.33 Short UI timings are bare numbers with no decision

The question is how long transient messages stay up and how often the UI re-checks things. "Done!" stays 0.5 s (`src/ui/app_state.cpp`, line 347). "Copied" stays 2.0 s (`src/ui/paths_tab.cpp`, line 471). The library search re-filters at most every 0.15 s (`kSearchThrottleSeconds`, `src/ui/library_table.cpp`, line 33). Batch results refresh at most once a second, a bare `1.0` (`app_state.cpp`, line 115), even though `kFileCheckSeconds` and `kReportCheckSeconds` beside it are named. Each has one owner, so nothing disagrees. Finding 219 covers only the 6.0 s fade, so three "how long does a confirmation stay" values (0.5, 2.0 and 6.0 s) exist with no shared rule. No ADR, CONTEXT.md entry or plan header decides any of them.

On history, the 0.5 s is older than the port. It comes from the Python app's `time.sleep(0.5)` before dismissing the analyze window, which the upstream author DragonDelgar added in 6e06f5e (2025-01-09, "Progress modal for analysis"). The 2.0 s first appears in 21ddf53, whose message says it only began tracking files that already existed. The Python app had no "Copied" text, so its true origin is unknown. The 0.15 s and 1.0 s came in with a50b3c0 (2026-09-27). The proposed owner is one named set of UI timings beside `kFileCheckSeconds` in `app_state.h`, after a decision from you. It was shown by reading and `git log -S`.

*Candidates: R7-B-9.*

#### R7.34 Numeric choices in the new audio readers have no decision

The finder searched every ADR, CONTEXT.md, every plan and spec under `docs/superpowers`, and the preview-loading handoff for each value below. A verifier spot-checked 15 of the listed constants at their lines and confirmed them. A plan task body is an agent's spec, not the header's list of your decisions, so a value that appears only there counts as undecided. Your decisions on the audio side are ADR 0019's 48 kHz stereo (line 27), 400 ms pre-roll (line 35), 1e-3 after 20 ms (line 66) and Opus end trimming (line 74).

Not decided anywhere:

- A zero-byte Opus packet counts as 120 ms (`packet_samples`, `src/audio/opus_reader.cpp`, line 69).
- The Opus reader queues at most 512 packets (line 408).
- Opus progress is reported every `4ull << 20` bytes (`kProgressStep`, line 51), and its comment says "4 MB" while the label's MB is 10^6 bytes. That only changes how often the bar moves.
- MP3 seek points sit about every 0.5 s (`build_seek_points`, `src/audio/ma_reader.cpp`, lines 440-441), and plan Task 2b's body (line 317) is the only place that says so.
- A short MP3 hop decodes ahead up to 1 s (`Mp3Reader::seek`, line 398).
- Two settle frames, a 32-frame back search and at least 8 frames to build a table (lines 443, 449, 458 and 423). The plan says "at least 2 frames" in the same task body.
- The MP3 scan returns nothing for data over 4 GiB (line 296), and a Vorbis file over 2 GiB throws at open (`src/audio/vorbis_reader.cpp`, lines 33-35).
- `StreamMix::kBlockFrames` is 4096 (`src/audio/stream_mix.h`, line 89), and the same 4096 is the cut-off for "odd" rate pairs (`stream_mix.cpp`, line 158). The plan names the block size at Task 3 (line 349), not the cut-off.
- The 4096-frame chunk constants appear as `kSkipChunk` (`ma_reader.cpp`, line 166), in `read_all` (`decode.cpp`, lines 55-56) and as `kChunkFrames` (`vorbis_reader.cpp`, line 28).

A later Opus link with another channel count ends the stem. That choice contradicts ADR 0006, and R7.39 covers it. These came in with a0913d0, 9f9c759 and fffa79c (2026-10-03). They were shown by reading and by a grep of the docs.

*Candidates: N2-12.*

#### R7.35 The engine turns a missing fill deadline or squeeze timing into 0 ms (extends finding 346)

Finding 346 lists seven places that read a missing backend offset as 0 ms. `enumerate` in `src/search/engine.cpp` (lines 147-148) does the same for two more optional fields: `activation_fill_deadline_ms.value_or(0.0)` and `sqinout_timing.value_or(0.0)`. The same file says at its NaN convention (lines 25-28) that a missing timing should "fail loudly".

Neither fallback can fire today. The deadline is read only in `branch_activate` (line 539), which the engine calls only on non-SP nodes (line 1091 onward). A non-SP node's `branch_edge` is set only by `add_act_edge` (`src/search/graph.cpp`, line 342), which always stamps the deadline (line 333). `sqinout_timing` is read at lines 616 and 671, only on the SqIn/SqOut branch, which needs `sqinout_time` set. Both graph sites set the two fields together (lines 243-244 and 361-362). Finding 127 covers `sqinout_timing` being stored twice, not this fallback. If a future graph change left a deadline unset, the engine would price that fill as due at 0 ms and silently accept or refuse activations. Both lines date from 3b69958, and no decision covers them. The proposed fix is the file's own convention: map a missing value to `NO_DOUBLE`, or refuse the graph. It belongs with finding 346's owner decision. It was shown by reading.

*Candidates: R7-A4.*

#### R7.36 Three tuning values have no decision and one owner each

These are low priority and are listed so the next round need not re-read them. `kParkedLookups = 16` (`src/ui/app_state.h`, line 445, used at `app_state.cpp` line 234, from ddde90c) is how many decoded records the details view keeps. The engine reports progress every 0.005 of the bar (`src/search/engine.cpp`, line 1070, from 3b69958). `kMainProgressShare = 0.9f` (`src/search/pather.cpp`, line 31, used at lines 164 and 170) gives the all-0 pass the last tenth of the bar. Its name was first tracked in 21ddf53 and reached `pather.cpp` through eaf3b73 and 283e028. None changes a game fact on screen. The batch worker cap is not listed here, because decision 22 decides it (R7.22). A grep of the audit and ledger for these names found no finding. It was shown by reading and `git log -S`.

*Candidates: R7-B-10.*

### Docs that disagree with the code

Each entry names the sentence and the code that differs. A verifier read ADR 0019, ADR 0020, ADR 0006 and `CONTEXT.md` against the code at HEAD.

#### R7.37 "A stem that hits a decode error part way through goes silent from that point" is not what the code does

ADR 0019 and `CONTEXT.md` ("Mixer", lines 226-227) say this. The code silences a stem on any short read (`src/audio/stream_mix.cpp`, line 244), decode error or not. That includes a truncated FLAC whose header promises more. ADR 0019 (lines 83-84) also says a file cut short "behaved the same before and after (it plays to where it ends)". That holds for the sound but not for the audio end of a truncated FLAC, which now runs to the STREAMINFO length (R7.8). The seek-back case differs by format (R7.9). Neither document covers these two. It was shown by reading.

*Candidates: D4, N2-1, N2-2.*

#### R7.38 "A straight read with no seek gives exactly the old samples" holds for one reader and not for the mix

ADR 0019 (line 61) says this. For a single reader the sentence holds, and the Opus pin test is unchanged. For the mix it fails on the inputs of R7.8: a FLAC with total 0 used to play and is now silent. For a resampled stem, the code's own comment (`src/audio/stream_mix.h`, lines 60-62) says "within float rounding", and the test allows 1e-6 (`tests/test_stream_mix.cpp`, line 218). A verifier did not measure whether the real difference is non-zero. It was shown by reading.

*Candidates: D1, N2-1.*

#### R7.39 "Chained files: each link plays in turn" is not what the Opus reader does

ADR 0006 (line 31) and ADR 0019 (lines 71-72) say each link plays. The `OpusReader` constructor stops keeping links at the first one with another channel count or no audio page (`src/audio/opus_reader.cpp`, lines 222-236). An unusable header also marks its link -1 (line 184). Later links are dropped, and the code comment says so. The plan's Task 2 body (line 289) says "stop at the end of the first link (rare; document it)", but that is an agent's spec, and the ADRs were not updated. It was shown by reading.

*Candidates: D5, N2-12.*

#### R7.40 ADR 0020 gives 260 as the only edge, and lists one fewer fallback than the code has

ADR 0020 says a short path comes back unchanged and a long one is made full first, and its only number is 260. `win32_path` treats 248 and up as long (R7.12). The ADR also says that with no short name, the report page is copied to `%TEMP%\Hydra`. `open_in_browser` (`src/app/report_files.cpp`, lines 118-121) also copies when a short name exists but `launch_html_viewer` fails: no .html association, or `CreateProcessW` fails. The ADR does not list that fallback. It was shown by reading.

*Candidates: D7, D9, N2-3, R7-B-6.*

#### R7.41 "1e-3 per sample after the first 20 ms" does not hold for odd rate pairs

ADR 0019 (lines 66-67) says this. When a rate pair lines up less often than every 4096 output frames, `StreamMix::Stem::seek_to` restarts at the target "within one input frame" (`src/audio/stream_mix.cpp`, lines 152-161). The test then allows 0.013 after 960 frames (`tests/test_stream_mix.cpp`, lines 272-290). Common rates line up much sooner: every 160 output frames for 44.1 kHz, 320 for 22.05, 3 for 32, 1 for 96, 80 for 88.2 and 640 for 11.025. All of those stay under the ADR's tolerance, so only odd pairs differ. It was shown by reading.

*Candidates: D2.*

#### R7.42 "The MP3 reader checks every seek" is true only inside the scanned frames, and the huge-file half of the finder's claim is wrong

ADR 0019 (line 51) says it checks every seek. `Mp3Reader::landed` returns true unchecked when the target lies past the scanned frames (`src/audio/ma_reader.cpp`, line 473), and `scan_frames` stops at the first frame that breaks the chain (lines 288-292). So a table seek past a resync point is not checked. The finder also said a huge MP3 goes unchecked. That is wrong. Over 4 GiB, `scan_frames` returns nothing (line 296), `build_seek_points` builds no table (line 423), and every seek takes `restart_and_skip`, the exact decode-from-start path (lines 404, 410 and 490-493). Huge files are slow but exact. It was shown by reading.

*Candidates: D3.*

#### R7.43 "The 3.1-hour overflow is gone" is true, and the new limit it hints at is not new

ADR 0019 (line 43) says the Vorbis overflow is gone. A Vorbis stem over 2 GiB now throws at open (`src/audio/vorbis_reader.cpp`, lines 33-35), and the ADR does not mention that. But the finder's "a new limit appeared" is wrong. At 707c285, `decode_ogg_vorbis` already passed `static_cast<int>(size)` to `stb_vorbis_decode_memory`, so files over 2 GiB already failed or misread. Today's code makes the same limit explicit. At typical bitrates 2 GiB is about 30 hours or more, so it is not a real-world case. The doc gap is the missing sentence. It was shown by reading.

*Candidates: D6.*

#### R7.44 ADR 0020's source-scan test is described as stronger than it is

ADR 0020 (lines 41-42) says the test "fails on any std::filesystem call ... whose line doesn't call os_path". The test checks a fixed list of 15 names (`tests/test_long_paths.cpp`, lines 282-286) and matches each as `"::" + name` (line 333). So `recursive_directory_iterator(` slips past, because the test looks for `::directory_iterator(`. `absolute(`, `status(`, `equivalent(`, `space(` and `current_path(` are not listed either. A grep of `src` and `tools` found only `std::filesystem::absolute` (`src/cli/report.cpp`, line 100 and `src/cli/fillcompare.cpp`, line 112) and `temp_directory_path` (`tools/replay.cpp`, line 427, which takes no path). Neither breaks on a long path today. It was shown by reading and a grep.

*Candidates: D8.*

### What today's commits did to existing findings

These are notes, not new findings. They record what the commits of 2026-10-03 did to findings that already exist. The commits that matter are 55049e7, 3686dd6, 8528da9, 8bc214f, 42f37e0 and 0d0bfe9, plus the long-path commits 11db329 and 02976c1 and the Preview loading and audio commits. Two finders read every file those commits touched and matched all 352 findings to them, by full path and by file name.

#### R7.45 Finding 269 is fixed

`report_file_exists` (`src/app/report_files.cpp`, line 132) now calls `file_exists_utf8`. The `is_dir` copy left `analysis.cpp` and became `is_directory_utf8` in `core/winstr` (line 98), from 11db329. One leftover: inside `winstr`, `is_directory_utf8` repeats the `GetFileAttributesW` existence test beside `file_exists_utf8`. Both live in the owner module.

*Candidates: newwork_A1 section 3, newwork_A2 section 3.*

#### R7.46 Finding 227 is half fixed and half extended

`decode_ogg_opus` no longer holds these numbers. `src/audio/decode.cpp` has no 48000 and no `kMaxFrame` at HEAD. They now sit once in `src/audio/opus_reader.cpp`, lines 48-50: `kRate = 48000`, `kMaxFrame = 5760` and `kPreRoll = 19200`. So 227's "48000 typed twice" half is fixed. Its "worked out by hand" half remains, and `kPreRoll` adds a second hand-worked value, 400 ms at 48 kHz. The 400 ms value itself is decided, in ADR 0019 (lines 35-38) and ADR 0006 (line 28), with the reason 80 ms was not enough. Only its spelling as 19200 is the hand-worked copy. `kPreRoll` came in with a0913d0 (2026-10-03). The other 48000 in `src`, `kOutRate` in `preview_load_job.h` (line 70), is the mixer's output rate, a different fact, as finding 227 already says.

*Candidates: Extension of finding 227, angle B; newwork_A2 section 3.*

#### R7.47 Finding 69 changed behaviour

Finding 69 says progress bars disagree about how full a bar is when the total is 0. For the Preview column, the bar at 0 of 0 is now 0.08, not 0.10. While the audio opens it is 0.08 plus 0.82 times done over total. When done it moves to Building (0.90) and then Highway (0.945). It still disagrees with the scan window (1.0) and the batch strip (0.0). This note comes from a finder's reading, and no verifier re-checked it.

*Candidates: newwork_A2 section 3.*

#### R7.48 Findings that today's commits extended or moved

Some findings now have more copies, or their copies moved. The new entries above carry the detail. Finding 30 is extended by R7.10, finding 54 by R7.26, finding 74 by R7.23, finding 76 by R7.14, findings 164 and 278 by R7.30, finding 182 by R7.21, finding 184 by R7.27, finding 187 by R7.17, finding 193 by R7.20, finding 278 by R7.28, finding 346 by R7.35, finding 351 by R7.1, and findings 149 and 159 (and 32 and 57) by R7.6.

A few more findings changed in ways the entries do not carry. Finding 74's copies changed: `decode_and_mix` is gone, and a stem that fails to open is now skipped in `PreviewLoadJob::open_audio`, with its bytes still counted as done. Finding 9's "audio end" input now comes from `StreamMix`'s promised length, which can differ from the old decoded length (R7.8). Findings 20 and 126 keep their gap: the parse moved from `resolve_preview_source` into `PreviewLoadJob::run` (`src/ui/preview_load_job.cpp`, lines 167-168), still with no settings key and no md5 check. Finding 217 is unchanged in substance: `report_files.cpp` now wraps the ShellExecute success test in a private `shell_open` (lines 66-70), and `win32_dialogs.cpp` keeps its own, so there are still two copies.

*Candidates: newwork_A1 section 3, newwork_A2 section 3.*

#### R7.49 Findings whose copies changed shape but still give the same answers

For positive tempos, none of today's commits changed what a cited copy outputs. Several findings now point at new code. Finding 32 and finding 1: the membership loop became the open-window walk (`src/core/replay.cpp`, lines 134-167). The gate still repeats `backend_row_value`'s first two checks, now as `pays` (lines 154-156), and the squeezed-out chord still sets `in_sp`. Finding 149: the replay copy moved to `replay.cpp` lines 149-151 and still clamps to 0 up to the deactivation node. Findings 57 and 159: the Preview time box scans became binary searches, and the gauge's tempo and meter splits became moving indexes, both with the same edges (`src/app/preview_view.cpp`, lines 513-527, 537-548 and 149-162). Finding 175: the scene's copy is now a moving index in `apply_preview_overlay` (lines 382-397), and the second copy is in `TrackState::set_overlay_intervals` (`src/render/track_state.cpp`, lines 200-228). Finding 157: the floor's on/off is now `SpanSweep::at` (R7.13), and the drain box copy (`preview_view.cpp`, line 598) is unchanged. Finding 52: the nearest-n guess is a backwards walk from `next_fill` (lines 389-395), still a guess. Finding 189: `find_song_ini` (`src/app/preview_source.cpp`, lines 122-127) now walks with the same `list_dir` that `discover_charts` uses, but the first-match versus last-match choice is still written twice. Findings 10 and 11: pitch 95 is now `mop_note(Kick, vel_dyn, true)` in `MidiParser::optype`, and the disco regexes became the hand matchers `is_disco_on_marker` and `is_disco_off_marker` (`src/parse/song.cpp`, lines 72-99), still hard-coding "3". Findings 9, 191, 329, 115, 135 and 314 moved from `build_preview_scene` into `build_preview_base`, and findings 97, 347, 216 and 352 into `apply_preview_overlay`, with no change in behaviour. Findings 348 and 339 still share `kOnActivationMs` (line 639). Findings 337 and 276 still share `kSpanEndTicks`, now used in `span_interval` (`track_state.cpp`, lines 176-185).

*Candidates: newwork_A1 section 3.*

#### R7.50 Finding 344 now has a live example

Finding 344 says the results stamp's bump rule leaves out the parser. Today's "Leaner MIDI and .chart parsing, identical results" (55049e7) is a parser-only change with no `kResultsStamp` bump. That is the exact case 344 describes. It is safe only because of the plan's identical-results rule and its equality tests. The stamp rule itself is still unrecorded.

*Candidates: newwork_A1 section 3.*

#### R7.51 Findings whose cited code was not touched

For these findings, today's change in the cited files did not touch the cited code, though line numbers may have shifted. In `src/core/replay.cpp` that covers `sqout_candidates`, `windows_for_path`, `resolve_sqout_note` and `ambiguous_window_warnings`: findings 29, 30, 38, 90, 110, 127, 128, 145 and 156. Findings 162, 163, 164, 166, 255, 266, 281 and 282 cite `replay_path`'s scoring lines, which are unchanged and moved by about 34 lines. In `tools/replay.cpp` today only swapped `u8path` for `os_path` and the sqlite opens for `store::open_sqlite`, which leaves findings 41, 68, 98, 103, 107, 130, 134, 139, 181, 198 and 246 as they were. In `preview_view.cpp`, findings 5, 37, 58, 89, 92, 93, 147 and 158, and findings 15, 39, 183, 185, 318 and 249, are as they were. In `song.cpp` the cited logic only moved, for findings 21, 43, 44, 53, 64, 96, 100, 178, 250, 258, 315, 320 and 331. In `analysis.cpp`, findings 56, 60, 63, 71, 111, 125, 142, 192, 256 and 345 only moved by about 30 lines. The second finder matched every finding that cites a file in the diff. Most of them, among them 2, 8, 14, 23, 31, 36, 47, 55, 65, 66, 72, 73, 86 and 103, came out as unchanged, meaning today's change did not touch their code. Findings whose copies are only in tests, docs or files outside the first finder's scope were not re-checked by it: 16, 102, 117, 118, 119, 122, 220, 221, 224, 226, 241, 279, 283, 285, 286, 300, 307 and 311.

*Candidates: newwork_A1 section 3, newwork_A2 section 3.*

### Refuted in round 7

No candidate was refuted as a whole. These are the parts that verifiers cut back or corrected. The findings above already use the corrected versions.

- **R7-B-10, the worker cap.** The finder said a comment is the only justification for `kBatchMaxWorkers = 8`. Decision 22 of the 2026-09-24 plan decides it, and c89596b added the comment. R7.22 keeps only the floor of 1 and the three parts the decision does not cover.
- **N2-9, what you would see.** The finder said a `StreamMix` failure would show "Something went wrong". The Preview prints the raw text instead, so no user sees that sentence on this path. The stale matcher stays in R7.20.
- **R7-A2, the first throw.** The `file_size_bytes` throw never reaches you, because its one caller catches it and uses a size of 0.
- **R7-A5, the second table row and the effect.** The row with the note at 10,350 ms cannot happen. Rows already stored would not change if the display check were removed. Only later records would.
- **R7-B-8, where the 10 ms default came from.** The finder said it began in the port or in d49c624. It came from the upstream author's b73597b.
- **R7-C2, the order in history.** The finder said the batch-only hint came after the coupling. It is older (21ddf53).
- **N2-2, MP3 in the table.** MP3 seldom goes silent at damage at all, because dr_mp3 skips a bad frame.
- **D3 and D6, one half each.** A huge MP3 goes unchecked is wrong (slow but exact). A new Vorbis limit is wrong (it already existed at 707c285).
- **Smaller history corrections.** R7-B-2, R7-B-7, R7-B-9 and R7-B-11 each had a commit or origin corrected by V4, and R7-B-7 had two wording corrections. Those are in R7.18, R7.32, R7.33 and R7.29.
- **Dropped by the finder, not a verifier.** Finder A2 looked at `same_file` in `src/cli/batch.cpp`, which now canonicalises `os_path(p)`. A long path would carry `\\?\` and a short one would not. MSVC's `_Canonical` strips the `\\?\X:` prefix from its result, so both sides come out unprefixed. It is not a finding.

## Candidates the verifiers refuted (8)

These were raised by a finder and refuted by the verifier who checked them against the code. They are not findings. They are listed so you can see what was checked and dropped.

- **store-28**: CONTEXT.md lines 34-37 already split lookups (report not analyzed, stale or ready) from listings (return only ready records). The candidate quoted only the second sentence. RecordStore::get_summaries is a lookup and reports Stale for the library chip, and RecordStore::list_records is a listing and drops stale rows. That is exactly the documented design, not drift.
- **display-13**: The report's Early fill column is computed in one place only: collect_rows in src/app/report.cpp (lines 289-293) takes the maximum of the model's e_difficulty over the path's activations. No second copy exists in src. store::summarize_path has a similar loop over difficulty(), which includes squeezes, so it answers a different question. The worry that another view might copy it later is not a duplicate today (introduced 9f1e2d2, 2026-08-31).
- **store-16**: Not an unrecorded assumption. ADR 0003 (lines 57-59) records that the leaderboard comparison is pinned to 4 bars and refuses to run otherwise. The toolbar in src/ui/library_toolbar.cpp disables the button at any cap other than kCloneHeroSpCap, and the picker is a modal popup, so the 'SP cap 8 reads 4-bar results' case cannot happen. The leftover point, that the cap-4 rule is applied in two places, is folded into the screenB-14 finding.
- **store-15**: run_batch and Settings::cap_query both call the owner, CapQuery::at, on the same Settings.sp_cap value. That is two calls to the owner, not a re-derivation, and prepare_row enforces that the key's cap matches the record.
- **store-29**: has_record and analyzed_hashes ask whether a Ready row exists. That is identical to 'the WinnerPicker's winner is Ready' for every input, because a Ready row always outranks a non-Ready one. They choose no winner, and their readiness SQL is the ADR 0018 twin covered under store-8.
- **sweep-probe2-c13**: The test literals [3840, 4051, 11731, 11761] and 3840.0 are hand-worked expected answers, not a second copy of the layout rule. Replacing them with a call to probe_note_ticks would make the test compare the function with itself. No production code re-derives the 2-bar lead-in or 4-bar pad: plan_inputs and build_probe_chart_text both call probe_chart.probe_note_ticks. 73 tests pass.
- **sweep-tests1-c3**: Nothing in src decides SP-phrase chord membership twice. The parser owns both phrase edges as ticks (it keeps no per-chord phrase flag; mark_sp_phrase_end sets flag_sp and sp_phrase_start only on the end chord). build_preview_scene only copies that span, and build_track_state is the only place that turns it into 'draw as energy', using the same ms function for the start edge as for notes and a half-tick display edge at the end. By reading, chords at the start tick, inside, at the end tick and at end+1 (even across a tempo change) all draw as the parser's tick span says. The proposed owner does not exist.
- **sweep-tests2-c11**: Nothing is re-derived. The library's Best path cell reads stored summary columns and the song panel reads the live record, but both trace to Path::totalscore and Path::pathstring on best_path(), cached through summarize_path and store::prepare_row, and both group digits with the one group_thousands. The only local piece is the two-space joiner in the table cell, which is layout. A stale summary would be a cache-invalidation bug, and none was shown.

## 1. Calibration result

The calibration agent caught both canaries.

Its brief named the file, the commit and methods 1 to 3. It did not say what was wrong. It was told to work from the code alone and not to read any doc, notes or memory file.

**Canary one: the multiplier decision written twice.** The agent's Finding 5 says that "which frontend direction governs a note at offset o from the SP end" is answered by five separate branches. Four are the backend-row branches in `rate_activation`. The fifth is the note loop's rule, which works from squeeze kind and the sign of the difficulty. Its truth table shows every branch reducing to "inside the SP end, early scale; outside, late scale". It also found the two edge differences at exactly 0 ms: SqIn and SqOut notes split there, and plain rows in [0, leeway) get no scale.

**Canary two: the squeezed-out note rated twice.** The agent's Finding 1 says a SqOut is judged twice. The backend loop rates the squeezed-out row against `scales.post`. The note loop rates the SqOut entry against `scales.pre`. It traced both numbers to one subtraction in `ScoreGraph::add_deact_edge`, which writes `sqinout_timing`, so the two offsets are the same stored number. It also gave the input where the two disagree: an activation with a SqIn and then a SqOut, with a tempo change between the two ends.

It reported nine more findings on its own. Among them: the pre-SqIn SP end is re-derived by stepping back two measures from the stored end; the transfer scale ignores `clamp_tick`; the materiality test is written three times; and the 2.0 ms difficulty edge has two different sides. All of them reappear as verified findings above.

Three caveats on this result:

- **A first calibration run was thrown away.** My first brief gave example questions and patterns that described the canaries almost word for word. I stopped that run before it finished and did not use it. The run reported here used a clean brief.
- **The project memory may have hinted.** Agents in this setup may load the project's memory index. One line in that index says the 2026-10-03 recurrence in `squeeze_rating` was "the same rule written twice in different words". I could not stop that from loading, and I can't confirm whether the agent saw it. It gives the file and the shape of the problem, not either canary's details.
- **A final checker re-scored the report independently.** It saw the canary list, which no finder did, and checked the report against the code at 707c285. Its verdict is in the completeness section.

## 2. Coverage

The ledger lists 2,688 functions, and every one has at least one row. The list was built by a script, not by reading. It covers 1,622 C++ functions, 592 doctest test cases and 474 Python functions under `src/`, `tests/` and `tools/`. The 592 test cases match a plain grep for `TEST_CASE` exactly. There are 3,443 rows in all, because some functions were read by several agents. Sixteen more rows cover things that aren't functions, such as constants, a page script and doc paragraphs.

The ledger is proof that every function was read by someone. It is not proof that every reader went deep. The finders traced their families across the whole repo. The ten sweep agents each read 136 to 239 functions, so a sweep row records a lighter read.

These were not checked, or were only partly checked:

- **`third_party/`** was skipped, as the prompt says.
- **Files the function script doesn't parse**: one PowerShell file (`tools/find_cmake.ps1`), the `.rc` resource file, two CSVs and header-only declarations. Constants in headers were reached through method 1, not the function list. JavaScript inside C++ string literals, such as the report page scripts, was read as part of the function that holds it.
- **The Preview overlay boxes** (time, score, drain, next activation and gauge readout) are drawn graphics that `hydra_uitest` can't read. A probe built in scratch read them through `build_preview_scene` instead.
- **One Dynamics case could not be driven to the end.** `hydra_uitest`'s `wait-idle` does not wait for the Dynamics job, which is itself a finding above. The Jamie All Over Hard case stopped at "Reading chart...". The Car Bomb, The Sentinel Hard case did complete, and it showed the 2x-kick drift.
- **Some drifts were shown by reading only.** Each finding says whether its disagreeing input came from a run, a Python port or reading.
- **Your decisions were looked for in ADRs, CONTEXT.md, plan headers and commit messages.** Session transcripts were not searched. The 2026-10-03 plan and handoff were deliberately excluded so the calibration stayed blind. A decision recorded only in a transcript or in those two files would be missed, and the item would show here as undecided.
- **Two calibration caveats** are in section 1: the discarded first run, and the project memory line that may have loaded.

## 3. Completeness passes

**The stop rule was not met.** Your prompt says the audit is done only when a completeness pass finds nothing new. No pass or round came back empty. You chose to stop after round 6 and take delivery as is. So the strongest claim this report supports is narrower: none beyond those listed were found in the ledgered functions by methods 1 to 4, in the passes that ran.

Every pass and round worked from the list of findings already known. Every new candidate was verified by a different agent before it counted.

**Passes 1 and 2 ran one fresh agent each, one after the other,** as your prompt specified. Pass 1 found four candidates, and all four survived (two in full, two in part). They were two rules for whether a library row counts as analyzed, best-path notation and score read from both stored columns and the blob, the settings bar deciding "Expert only" for 2x Bass itself, and two definitions of an all-0 path. Pass 2 was aimed at the area pass 1 exposed. It also found four, and all survived (one in full, three in part). They were two rules for whether a library search is on, activation and note counts read from columns on one page and from the blob on another, the DM and fill pages using their own "has a result" rule, and two rules for whether the all-0 section is redundant.

**Rounds 3 to 6 ran eight fresh agents each, in parallel,** one per angle, at your request after pass 2. Round 3 found 26 new candidates, and all survived. Round 4 found 14, and all survived. Round 5 found 13; 11 survived and 2 were already known. Round 6 found 13; 12 survived and 1 was already known. Some of the round 3 to 6 finds are drifts you would see, for example:

- the Preview keeps the old chart's notes after a difficulty, Pro Drums or 2x Bass change;
- the `.chart` and `.mid` parsers disagree on when a Star Power phrase ends;
- a `.mid` with a zero-numerator time signature crashes the app;
- a tied variant shows its leader's leftover SP and skip count instead of its own;
- the song panel keeps an old result after a batch updates the same chart.

Of the 71 surviving completeness finds, four pairs described the same problem twice, and one repeated a main finding (the "Paths kept" count, `display-31`). That leaves 63 completeness findings, and they are folded into the sections above.

**What the trend says.** New finds per round fell from 26 to 14, then held near a dozen. The later ones lean toward docs, tests and smaller corners, but not entirely. Running eight agents per round made "a round finds nothing" harder to reach, because eight agents digging deep will almost always turn up something. About 10% of the round results overlapped each other. The bigger cost was reading: each agent started cold and re-read the same core files. A leaner design would run three or four agents per round, each given the previous round's notes on what had already been searched.

**The final checker** saw the canary list and checked the calibration report against the code at 707c285. It found both canaries caught, and the code confirms both. On canary one it added a note: the report gives the note loop's rule as "works from difficulty = -offset" and does not spell out that the loop also reasons from squeeze kind. The checker made only five tool calls and skipped the status-line rule.

**Round 7 ran on 2026-10-03, after delivery, with five finders and four verifiers.** Two finders read the code committed after the snapshot 707c285. Three ran completeness round 7 by new angles: lightly read functions, every literal, and the screens traced least. They raised 42 candidates and nine documentation checks. The verifiers refuted none as a whole, cut back parts of several, and folded duplicates into other items. 44 findings survived, and seven more entries record what today's commits did to findings that already exist. They are in the section "Round 7 and the 2026-10-03 code", before the list of refuted candidates. **The stop rule is still not met**, because round 7 found new items.

## 4. Proposed order of fixes

This is a proposal, not a plan in motion. Nothing will be edited until you say yes, and each step below would get its own plan first.

**First, let the engine stamp the facts the displays now guess.** This is the ADR 0011 pattern again. Five drifts come from a display or tool rebuilding something only the engine knows. The pre-SqIn SP end is rebuilt by stepping back two measures. The cap-clamp anchor is ignored by the transfer scale. The Preview gauge guesses phrase collection and gets late SqIns wrong. The replay decides on its own which chords Star Power pays. The backend table rates a SqOut on phrase notes the engine never squeezes out. The double rating inside `rate_activation` belongs in this step too. The untracked plan `docs/superpowers/plans/2026-10-03-one-squeeze-rating-rule.md` already covers part of it. These come first because they change numbers you check, and because the stored-field fixes may need a results-stamp bump.

**Second, fix the parser drifts you can see on the Dynamics tab and in scoring.** At Hard and below, the Dynamics tab reads Expert's 2x kicks, and disco flip follows Expert's markers. The tab also counts 2x kicks in one total and drops them in another, and a late dynamics tag leaves early notes plain while the tab says dynamics are on. The completeness rounds added two more here. The `.chart` and `.mid` parsers disagree on when a Star Power phrase ends, and a `.mid` with a zero-numerator time signature crashes the app. Each has one owner in `src/parse/song.cpp` or `src/app/dynamics_breakdown.cpp`. The disco-flip fix may change analysis results, so it needs your call on Clone Hero's behaviour first.

**Third, give display formatting one owner per rule.** The visible drifts here are small but they're on screen today. They are the whole-ms rounding of the hardest squeeze, the percent rounding, the count-with-noun helper ("1 bars"), the two SP golds and the teal, the two names for one drum note, the two rich-text strippers, the four Stale messages, and what "optimal" means in the report versus the Paths tab. Most of these become one function each in `src/app/display_format` or `src/ui/theme.h`. Two stale-copy drifts belong next to them. The Preview keeps the old chart mode's notes after a difficulty, Pro Drums or 2x Bass change. The song panel keeps an old result after a batch rewrites the same chart.

**Fourth, correct the docs.** The User Guide states the early-fill sign backwards, hard-codes 3 ms, and says untagged MIDI charts show dynamics counts. ADR 0011, ADR 0012 and the cap-clamped doc name deleted functions or state the SP end without the clamp. Several CLI messages still say results don't carry their fill rule. These are cheap and need no code change.

**Fifth, one decisions round with you on the undecided assumptions.** Each item in that section needs a yes, a different value, or "make it a setting". Examples are the 1 ms impact gate, the 0.005 scale tolerance, the 10 ms rating bands, exactly 3.0 ms counting as uncounted, the exact cap tie, the 60 ms early-fill window, the song length the Preview should use, and which SP colour is right. Once decided, each goes into `hydra_rules.ini`, an ADR or CONTEXT.md, so it stops being undecided.

**Added 2026-10-03, after the audit: one owner for "does the Windows shell take this path".** Today's long-path work (02976c1) promised one place for long paths. The conversion itself does live in one place, `win32_path`. But the separate question "is this path short enough for the shell" is written five times. `shell_path` in `src/core/winstr.cpp` checks it twice, `open_in_browser` and `copy_to_short_temp` in `src/app/report_files.cpp` check it again, and the long-path test recomputes it. All five agree today. The fix is one small function in `core/winstr` that every one of them calls, plus a rule in the single-owner scan test so a sixth copy fails the build. Nothing changes on screen. Round 7 candidates N2-3, R7-A1 and R7-B-3 describe it, and verifier V2 confirmed it (see R7.12). You put this on the fix list on 2026-10-03, and the scan rule comes from the review-gate plan `docs/superpowers/plans/2026-10-03-derive-once-review-gate.md`.

**Last, fold the duplicates that agree today, module by module.** These change nothing on screen. They stop the next drift. Group them by owner so each change touches one module. Tests that recompute production values become tests that call production code or pin literal values. The `tools/ch_probe` constants move into `constants.py`.
