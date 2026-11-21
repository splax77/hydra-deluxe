# SW5: question-3 sweep, delta re-send, records A2 A8 A22 A32 A39 B5 B6 B7 C1 C3

Agent a1b4940001876bcab, task T5-SW5. Read-only listing; no record changed. Result in one line: no new Rewrite record in these files is "Q3 only". Every one of the 32 new rows names another failing question.

How I found the records. I did not use `rewrites.tsv`. For each of the ten files I took every line holding "Verdict" followed by "Rewrite" (in any heading form, including the plain-title and inline forms) and tied it to its record by the title above it. That gave 134 Rewrite records in all. A record counts as already judged when `sweep/SW1.md` to `SW4.md` hold a line that names it by heading or by its source line. Seven of the ten files were fully covered (A2, A8, A32, B5, B6, B7, C1; for B5 and B6 the "Extra real Rewrite verdicts" section in `SW3.md` names each of their 7 and 16 verdicts by source line). In three files the first sweep judged only a section row or a vague summary, so 32 records have no line of their own: 18 in A22, 11 in A39, 3 in C3.

How I judged. As in `briefs/T5-sweep.md`: Q3 only means the record's reason for Rewrite is that the claim was not checked on the real thing and every other question holds. For each record I read its own Q3 line and its own Verdict line (the prescribed fix). Every fix below asks for a change beyond real input (a typed number or a copied rule replaced, a floor or a loop that can pass with nothing read, a wrong or missing assertion, a stand-in for the thing named). I opened no test code. I did not open the verify files for these 32; the records' own words are quoted.

Line form: `<heading, with the record's line in its records file> | class | the record's words | capture or real input named`.

## A22 (records/A22.md; 20 Rewrite records, 2 already had a section row in SW1: the load-progress record at :58 and the renderer bare-highway record at :394; 18 new)

Preview renderer, a gem at the strike line adds drawn pixels (test_preview_renderer.cpp:127; record line 426) | Other fault | Q3 says "claim not checked (screen B, a real chart's gems)"; fix: "Rewrite. Drop 155; check each of the three gems at its own projected centre, kick included." An extra assertion and a missing check per gem | n/a
Preview renderer, SP phrase energy gems and active SP floor change pixels (:168; record line 454) | Other fault | "Rewrite. Drop 202; keep the three colour checks, each at a located pixel." An extra assertion, and the checks are not at located pixels | n/a
Preview renderer, resize and a tall target keep the track at the bottom (:263; record line 502) | Other fault | "Rewrite. Compare against the projected strike line from project_to_image and a named margin, not a sample at row 292." Pinned sample row | n/a
Preview timeline, a point in the Fills row names a band it is on, else the nearest fill (test_preview_timeline.cpp:74; record line 669) | Other fault | "The comment says 'Pinned from one run'. State 87-90 as the midpoint of the two fill x's the layout already reports." Pinned numbers with no working | n/a
Preview timeline, the moment under an x stays inside the window (:140; record line 692) | Other fault | "Write each result as start + share * span from the window's named start and span (no pinned number)." | n/a
Preview timeline, a fill outside the window has no item, a half-outside band keeps its visible half (:156; record line 719) | Other fault | "Replace pinned x's with fraction(view, ms) * width from the named inputs." | n/a
Preview timeline, the window moves every x by the view's start (:191; record line 741) | Other fault | "State the property: every x in b equals the same x in a minus the shift (2000 ms in px), over every row." Pinned values, not the rule | n/a
Preview timeline, the axis labels the 30 s steps and thins them by span (:226; record line 766) | Other fault | "Express the labels as multiples of kTimelineAxisStepMs inside the window." Typed label values | n/a
Preview timeline, the saved line has a point at each segment start and end and a mark at kSpActivationBars (:247; record line 790) | Other fault | "y positions from the row's named inset and the cap; no pinned numbers." | n/a
Preview timeline, the insets move the bands, the fills, the saved line and the axis labels (:269; record line 813) | Other fault | "Write each y as row top plus the named inset." Pinned y values | n/a
Preview timeline, the saved row's enough stretches start after the last activation (:303; record line 838) | Other fault | "The engine's sp_ready_ms per activation is the owner of 'ready'; the test should take its expected side from it on a real chart. Finding: saved_enough works out an SP-ready fact for itself." A second owner of the ready moment, plus the expected side typed | none named (a real chart with a saved path, "screen C")
Preview timeline, with no activation every stretch at or above the line is shaded (:345; record line 885) | Other fault | "Same as :303: the engine's ready moment is the expected side. Finding: as :303." | none named
Preview timeline, the ghost path's windows are bands in their own row (:363; record line 911) | Other fault | "Pinned numbers become the sum of row heights; the claim that ghost bands take no click needs a capture." Pinned numbers beside the missing capture | none named
Preview timeline, every fill in best_ticks has a word under it, kept inside the timeline (:461; record line 978) | Other fault | "Express with named inputs: centre minus half the word, clamped to 0." | n/a
Preview timeline, the axis labels stay inside the timeline (:524; record line 1043) | Other fault | "Work text_x from x and the 40 px width; no pinned number." | n/a
Preview timeline, a label that would start left of the timeline starts at its left edge (:538; record line 1062) | Other fault | "Written as max(0, x - w/2) from named x and w." | n/a
Preview timeline, a huge song length gets labels by the width, not by the length (:550; record line 1084) | Other fault | "Check the labels never come closer than the gap, instead of counts of 19 and 20." Typed counts | n/a
Preview timeline, the scrollbar's thumb and readout come from the view (:568; record line 1107) | Other fault | "Express thumb as share arithmetic from the named inputs." | n/a

(The A22 rows with Q3 text "Real chart: claim not checked (screen C)" all carry the same Q3 fault as a second fault; the fixes above are about typed numbers.)

## A39 (records/A39.md; 11 Rewrite records, all new)

test_dynamics, Dynamics tab on Acid Romance, counts, colours and widths, 2x Bass on and off (uitest_details.cpp:249; record line 83) | Other fault | Q3: "Acid Romance is not among the seven charts and the Dynamics tab is not captured." Fix: "the 2x-kick sentence is built from those counts, never typed; the width check takes the width from the table's own placed width ... (no re-run of the width rule)." Typed counts and a re-run rule | none named (a Dynamics capture on Acid Romance is wanted)
test_dynamics_reopen (:346; record line 103) | Other fault | "take the ghost count from viewed.dynamics in both scenarios; drop the claim 'never the store' or state it as a property that can still fail." A claim in the name the test cannot fail | none named
test_dynamics_hard (:421; record line 123) | Other fault | "State D20 as a property ... No literal count." Typed counts | none named (Dynamics at Hard on Pathfinder)
test_stars (:467; record line 143) | Other fault | Q3 names a real capture (Believer Stars tab, base score 71,740, seven cutoffs) that agrees. Fix: "Check each of the seven rows as a row ... against star_cutoffs, cite SC-24 at the factor, and drop the width self-check." Typed values and a self-check | the Believer Stars-tab capture
test_details_close_teardown (:539; record line 163) | Other fault | Q9: "the test writes the button's two statements by hand instead of using the button (a copy of the button's body)"; fix: "Reach the route the way a player does ... check the audio is released on the real device." Stand-in, and the claim is not checked on the real device either | none named
test_preview_load_bar (:617; record line 203) | Other fault | "The comment itself says the charts load in a few frames, so the loop may see no frame where loading() is true." Fix: "Hold the load at a gate so at least one poll lands, require polls above zero." A loop that can pass with nothing read | none named
test_panel_open_close (:655; record line 223) | Other fault | "check the widths against library_split_width(room, share) as test_panel_split does, and drop the typed fractions." Typed fractions | none named
test_layout_sweep (:879; record line 323) | Other fault | "Require the Preview to have loaded (use the result of the wait) and that the Expand all control existed when expected." A sweep that can pass with nothing measured, and a side note that an error stops the sweep early | none named
test_panel_prev_next (:999; record line 363) | Other fault | "Add the last row's end ('next' disabled or inert) or cut the comment's promise." The comment promises more than the test checks | none named
test_panel_headline, Burnout before and after analysis (:1019; record line 383) | Other fault | Q3: Burnout "is not among the seven"; fix: "The headline shows the best path's total, string, stars and hardest timing as viewed.record names them." The expected values are typed instead of read from the owner | none named
test_view_rules_broken (:1134; record line 443) | Other fault | "the broken file is a one-line fixture ... and there is no capture of the panel." Fix: "The panel shows exactly what the toolbar shows for the same state (read from the owner), the Dynamics counts are read from viewed.dynamics." Typed expected text | none named

## C3 (records/C3.md; 11 Rewrite records, 8 already named in `SW3.md` rows or its "Extra real Rewrite verdicts" section; 3 new)

label layout: 39 crowded numbers never overlap, first and last show (test_label_layout.cpp:40; record line 892) | Other fault | Q3: "the claim is about what the Paths tab shows ... Claim not checked." Fix: "Delete line 56; say 'no label that would fit between its neighbours is left out' as a property if more is wanted." An assertion that has to go | none named (a Paths-tab strip capture on a chart with many close activations)
label layout: labels at the ends stay on the strip (test_label_layout.cpp:59; record line 905) | Other fault | "Write 288 as the strip's right edge minus the label's width." A typed number | none named (same capture)
installer: the builder has a Start Menu shortcut (test_launch.cpp:83; record line 981) | Other fault | "Drop the exact-name pin (line 100) and ask the owner of the taskbar behaviour ... Needs the taskbar capture too." An exact-name copy of the rule beside the missing capture | none named (a taskbar capture)

Not new but worth the main session's eye, because SW3 and SW1 recorded them as section rows: C3 :953 (launch quotes) is the record at line 953 and has Q3 plus "drop the typed spellings at 45-49", so Other fault holds. C3 line 679 (decode_image PNG, test_image_decode.cpp:31) is named in SW3's extras: "Check alpha on all four pixels", Other fault.

## Count table

Columns: Rewrite records in the file in all, new to me (no line in SW1 to SW4), Q3 only, Other fault, Unclear (the last three count the new ones only).

| records file | Rewrite in all | new to me | Q3 only | Other fault | Unclear |
|---|---|---|---|---|---|
| A2 | 10 | 0 | 0 | 0 | 0 |
| A8 | 23 | 0 | 0 | 0 | 0 |
| A22 | 20 | 18 | 0 | 18 | 0 |
| A32 | 8 | 0 | 0 | 0 | 0 |
| A39 | 11 | 11 | 0 | 11 | 0 |
| B5 | 7 | 0 | 0 | 0 | 0 |
| B6 | 16 | 0 | 0 | 0 | 0 |
| B7 | 5 | 0 | 0 | 0 | 0 |
| C1 | 23 | 0 | 0 | 0 | 0 |
| C3 | 11 | 3 | 0 | 3 | 0 |
| total | 134 | 32 | 0 | 32 | 0 |

A2, A8, A32, B7 and C1 match their `rewrites.tsv` row counts (10, 23, 8, 5, 23). B5 (7), B6 (16) are named by source line in SW3's extras section, so I counted none as new, though their tsv rows are one each. A22 holds 20 (its own count line says Rewrite 20).
