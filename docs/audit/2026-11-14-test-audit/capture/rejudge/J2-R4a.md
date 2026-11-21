J2 R4a re-judge, agent a4d085f736cf34568. Runs in AUD/capture/runs/R4a (beg, believer_path4, believer_path0_moments, flammarker_ingame, folded_sqin, late_sqin_twice, path_reduction_test0, pitch_dark_moments). Each opens one already-analyzed chart, reads the Paths tab text, then takes Preview highway screenshots at activation moments. No run varies panel width, picks a path in the Preview, changes a setting, opens an unanalyzed chart, or uses the legacy fill rule.

| Record | Test | Run | Verdict | Evidence |
|---|---|---|---|---|
| A44 7 | uitest_paths.cpp:540 | R4a | NOT SHOWN | No capture sets the panel width (0.01 / 0.99) and no chart is The Spiraling Void. Needs a hydra_uitest run at both widths. |
| A20 2 | test_preview_scene.cpp:70 | R4a | NOT SHOWN | Round trip of an internal function. pitch_dark_moments_a1_start_85963.1ms.png and beg_a1_start_74079.9ms.png show red and yellow notes in matching lanes, not all five colours, and a picture cannot show the round trip. |
| A20 3 | test_preview_scene.cpp:80 | R4a | NOT SHOWN | Song length is never shown on screen (the time box shows the playhead measure, "of m146.2.80"). |
| A20 7 | test_preview_scene.cpp:174 | R4a | NOT SHOWN | No chart has a solo: every Stars tab says "Solo Bonus: 0" (beg.txt:80, flammarker_ingame.txt:37). |
| A20 10 | test_preview_scene.cpp:227 | R4a | NOT SHOWN | All charts were analyzed before the Preview opened ("Analyzed (1)"). No unanalyzed capture. |
| A20 14 | test_preview_scene.cpp:289 | R4a | NOT SHOWN | No R4a run uses the legacy (1.0) fill rule. |
| A20 15 | test_preview_scene.cpp:306 | R4a | NOT SHOWN | Highway shots show faint lines on 4/4 charts (header "4/4"), but tick spacing (1920, 480, half) cannot be read from a picture. |
| A20 17 | test_preview_scene.cpp:351 | R4a | NOT SHOWN | No screenshot header read shows 6/8 (the two read both say 4/4). |
| A20 18 | test_preview_scene.cpp:373 | R4a | NOT SHOWN | No 4/4 to 6/8 change chart in the run. |
| A20 20 | test_preview_scene.cpp:411 | R4a | NOT SHOWN | No screenshot near the song length (224642 ms for Pitch Dark). pitch_dark_moments_a3_end_195426.9ms.png is the last activation end, 29 s before it. |
| A18 2 | test_model.cpp:43 | R4a | NOT SHOWN | Tie order inside a chord's pricing is on no screen or text. |
| A18 31 | test_model.cpp:941 | R4a | SHOWN (squeeze-out at 0 only) | beg.txt:38 "1 1- m31.1.0 3 bars squeeze out 0.0 ms"; beg.txt:40 "Hit the [K Y  ] note more than 0.0 ms late"; beg_a1_start_74079.9ms.png shows "1- (0.0 ms)". Matches "exactly 0 needs 0.0 ms" for a squeeze-out. A squeeze-in at exactly 0 and the "needs none" side are not shown (late_sqin_twice.txt:45,49,53 show 225.0, 30.0, 195.0 ms). |
| A18 33 | test_model.cpp:962 | R4a | SHOWN (zero-slack side only) | beg.txt:65 "4 E0 m83.1.0 2 bars early fill 0.0 ms"; beg.txt:67 "Early fill: 0.0 ms (required)". "Time to spare needs none" side not shown. |
| A18 37 | test_model.cpp:1034 | R4a | NOT SHOWN | No run reads a settings file with odd-case difficulty names. |
| A18 54 | test_model.cpp:1404 | R4a | SHOWN | The Stars tab lists six categories and a total; they sum on all six charts: beg.txt:77-84 66,050+196,500+65,200+0+200+200 = 328,150; believer_path4.txt:82-89 71,640+211,935+68,260+0+0+100 = 351,935; flammarker_ingame.txt:34-41 39,660+115,635 = 155,295; folded_sqin.txt:50-57 2,150+3,600+1,200 = 6,950; late_sqin_twice.txt:74-81 3,000+6,150+4,000 = 13,150; path_reduction_test0.txt:59-66 6,600+16,950+21,900 = 45,450. Matches "the total is the sum of the six". |
| A29 13 no SP no path | test_sp_meter_curve.cpp:406 | R4a | NOT SHOWN | flammarker_ingame has a path record ("(No activations.)", flammarker_ingame.txt:21), and its only capture is song start with gauge "0.0/4" (flammarker_ingame_song_start_0.0ms.png). Whether it has an SP phrase is not shown, and any chart's gauge starts empty. Not a no-phrase-no-path chart. |
| WN5 2 | uitest_preview.cpp:151 | R4a | NOT SHOWN | No run picks another path inside the Preview list. believer_path4.cmd.txt clicks ##path4 on the Paths tab, then opens Preview; no second pick, no "first path again". |
| A45 2 | uitest_preview.cpp:90 | R4a | NOT SHOWN | No run finishes an analysis while Preview is showing; charts were already analyzed. |
| A45 3 | uitest_preview.cpp:151 | R4a | NOT SHOWN | Same as WN5 2. |
| A45 7 | uitest_preview.cpp:534 | R4a | NOT SHOWN | Every screenshot is one panel width (804x424 highway image); none at the narrowest library or 640 px. |
| A45 15 | uitest_preview.cpp:849 | R4a | NOT SHOWN | No run changes difficulty or Pro Drums. |
| A45 16 | uitest_preview.cpp:922 | R4a | NOT SHOWN | No run changes SP cap or Pro Drums during analysis; no before/after score. |

## Side notes
- window-WN5.md and A45.md both record test_preview_path_overlay (uitest_preview.cpp:151); CLEAN.md lists it twice (WN5 2, A45 3). One test counted twice.
- CLEAN.md line 81 says the output is capture/out/R4a; it is in capture/runs/R4a.
- R4a fits only 3 of the 22 records. The other 19 need runs that vary panel width, path pick, settings, meter, solo, legacy fills, or an unanalyzed chart.
- A18 31/33 pin typed numbers (0.001, -0.001) per their records; A18 54 pins typed 21 and 1110 (test_model.cpp:1315, 1324).
- pitch_dark_moments.cmd.txt has no `text` step, so Pitch Dark has no Paths tab text in R4a.

Count: SHOWN 3, CONTRADICTED 0, NOT SHOWN 19.
