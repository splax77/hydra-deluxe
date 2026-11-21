Agent ad2c0dd7aaa5b5442 (T5-V-RESEND-C6, session 16892967-2cf8-4437-a7cd-e3dad250f621)

# Verify C6, re-send: the nine records the first verifier did not reach

Records checked: 9 of 9 on the first verifier's "Not reached" list (NW15, OBJ1, OBJ6, OBJ7, OBJ8, OL19, OL20, OL21, OL22). I agree with all 9 verdicts. Disputes: 0. Records not audited: none.

What I opened: `tests/test_note_window.cpp` 14-70 and 370-394; `tests/test_obj_loader.cpp` 1-186 (all); `tests/test_overlay_layout.cpp` 380-516; `tests/fixed_pitch.h` (ten_per_char); `src/core/note_window.cpp` (all); `src/core/model.cpp` 477-480 and 702-706; `src/core/model.h` 327; `src/render/obj_loader.cpp` 50-213; `src/render/overlay_layout.cpp` 141-285; the shipped models `assets/preview/models/drum-kick.obj`, `drum-tom.obj`, `drum-cymbal.obj` (counted with awk); `records-screen/pitch_dark_preview_1_act.png`; the records in `records/C6.md` lines 454-481, 485-516, 616-691, 1115-1216. I ran no `hydra_*` program. There is no run output in the records folders for these tests, so every mark below is "by reading".

Callers (question 2). By grep over `src/` and `tools/`: `ellipsize` is called at `ui/widgets.h:285`, `ui/library_table.cpp:534`, `ui/report_window.cpp:40`, `ui/settings_bar.cpp:271`, `ui/preview_tab.cpp:61`, `ui/preview_timeline.cpp:457`, and nowhere in `tools/`; the record's list is complete. `wrap_right_box` is called only at `ui/preview_panel.cpp:560`, `overlay_scale` at `:607`; none in `tools/`. `make_flat_quad` and `make_box` are called only at `render/preview_renderer.cpp:391-392`; `MeshId::Box` is used at `render/highway_draw.cpp:298` (a rail). The records' callers hold.

## NW15 (Rewrite): agree

By reading. `early_fill_window_end` (`note_window.cpp:132-140`) takes the SP-ready tick from `sp_ready_tick` (`model.cpp:702-706`: `bank_rise_ticks[1]`, tick 768 in the fixture at test line 55) and subtracts `*act.e_difficulty(true)`, which is `-e_offset + 0.0` (`model.h:327`). With `e_offset = -10` the figure is +10, so 85.7156 - 10 = 75.7156, and the line-386 literal is that; with `e_offset = 10` the figure is -10 and the line-393 literal is 85.7156 + 10. The record's working is right. Lines 383-385 (role, tick 768, side == `kCapSide`) check real fields. Line 392 is a repeat of 383. Lines 386 and 393 are raw results of `kCapSide` plus or minus the figure, so Rewrite as `kCapSide - 10.0` and `kCapSide + 10.0` is right, and `kCapSide` is defined in the file (line 38). Line 386 would go red if `early_fill_safe_ms` added the figure instead of subtracting it; line 393 for the same bug on the other sign. Question 3: the record's own read of `beg.txt:65-70` (figure 0.0, side 84.4, safe 84.4) agrees with the rule at figure 0; the sign cases are not on a real row, as it says. Verdict stands.

## OBJ1 (Investigate, claim not checked only): agree

By reading. `load_obj` (`obj_loader.cpp:83-134`) fans each face with `push_fan` (`:64-70`), so each of the four kick quads gives two triangles: 8 triangles, 24 vertices (test 44-45). Lines 47-54 check every vertex's normal is unit and uv is in [0,1]; the kick text's own uvs are all in [0,1] and each corner has a `vn`, so a face read with no normal (zero vector) turns line 50 red; a wrong uv index would turn 51-54 red only if it picked a value outside [0,1], which none of this file's values are, so 51-54 catch a missing uv, not a wrong index (the record's "off by one (reasoned)" is too strong for that line). Lines 58-63 compare bounds against the inline vertex list: a wrong vertex index would move a bound.

The record's Question 3 gap is "nothing compares the inline copy with the shipped file". I compared them: the inline `kKickObj` (test 22-33) is the same vertices, uvs, normals and faces as `assets/preview/models/drum-kick.obj` (the shipped file only adds blank lines). So the claim holds on the real file today, by my reading, but the test does not read the shipped file and nothing keeps the two copies equal. The shipped file's face count and 8 triangles are checked at OBJ8 line 167. The verdict stays Investigate, "claim not checked only"; the real input it needs is the test loading the shipped `drum-kick.obj`, which `HYDRA_ASSET_DIR` already gives OBJ8. Not a Keep, because the test's input is a hand copy.

## OBJ6 (Investigate): agree, with one correction to Question 8

By reading `make_flat_quad` (`obj_loader.cpp:176-182`). It builds one quad through `push_quad` with the corners (-x,+z), (+x,+z), (+x,-z), (-x,-z) at y 0, normal +Y, uvs (0,0), (1,0), (1,1), (0,1). Test lines 114-119 (bounds) and 111 (2 triangles) hold. Lines 123-124 (u = x + 0.5, v = 0.5 - z) hold at all four corners. Line 121 holds. The winding at 128-132: first triangle (a, b, c) gives e1 = (1,0,0), e2 = (1,0,-1), y of the cross product = e1.z*e2.x - e1.x*e2.z = 1 > 0, so it holds. Correction: line 132 checks only the first triangle; the record says each line catches "the winding changed", but the second triangle (a, c, d) is never checked for winding, so a flipped second triangle would not turn anything red. `make_flat_quad` does not sort, so vertices 0-2 are that first triangle. The "Onyx UV layout" in the name has no source opened in the test or in `obj_loader.cpp` (the comment at `:178` gives only the winding); the uv lines 123-124 restate the corner table in the production code rather than a rule from elsewhere. Question 3 needs the Preview's flat lanes, as the record says; the `pitch_dark_preview_1_act.png` capture shows the lane pads, but nothing in it shows the uv direction. Verdict stands.

## OBJ7 (Investigate, claim not checked only): agree

By reading `make_box` (`obj_loader.cpp:184-200`). I worked the winding for two faces: top (a (-h,h,h), b (h,h,h), c (h,h,-h)) gives a cross product of +Y, outward; left (a (-h,-h,-h), b (-h,-h,h), c (-h,h,h)) gives -X, outward. The loop at 139-155 covers all 8 triangles, so a face wound backward turns line 148 red on each of its two triangles. Lines 153-154 look at the stored normal of each triangle's first vertex, so they catch an added bottom or back face only if that face carries a bottom or back normal; combined with 148 (normal agrees with winding) that is enough. 156-159 catch a missing face. The record is right. The box is used at `highway_draw.cpp:298` as a rail, so the capture needed is a Preview with the rail visible; the screen capture I read shows the highway edges but nothing named as that mesh, so the claim stays unchecked on screen. No fault in the test itself. Verdict stands.

## OBJ8 (Rewrite): agree

By reading, and checked on the real files. The question 3 form for a claim about files is real input, and I counted the shipped files independently with awk: `drum-kick.obj` has 4 quads (8 triangles, test 167); `drum-tom.obj` has 126 four-corner faces and 2 fourteen-corner faces, so 126*2 + 2*12 = 276 = 252 + 24 (test 171); `drum-cymbal.obj` has 352 four-corner, 32 three-corner and 1 thirty-two-corner face, so 704 + 32 + 30 (test 181). The vertex extents agree with the typed ones: tom x +-0.49097, y -0.000322 to 0.30859, z +-0.232598; cymbal x +-0.505644, y 0.044534 to 0.26993. So the claim holds on the shipped files today. It is still a snapshot of the asset: the counts and extents were read once from the files, and no owner other than the file itself gives them. The tolerances are loose: line 183 (`0.045f` with epsilon 0.05) passes for a minimum y anywhere from about -0.007 to 0.097, and 184 (`0.270f`, epsilon 0.02) accepts about +-0.027, so the extents lines catch a swapped model only roughly. Line 175 (`0.0f`, epsilon 0.01) accepts the tom's true minimum -0.000322. Rewrite stands: state what the renderer needs as properties, with no typed triangle counts.

## OL19 (Rewrite): agree

By reading the test (422-469) and `wrap_right_box` (`overlay_layout.cpp:212-239`), `overlay_scale` (`:141-158`) and `right_room` (`:160-163`). The test builds the largest box at scale 1 (441-446), gets the scale, and wraps each box again at its own lowest row. Line 456 checks the wrapped width against `right_room` at the box's bottom; `wrap_right_box` itself aims at the same `right_room` (`:232`), so the line is not true by construction (the first word of a line always goes in, even too wide, `:200`), but both sides use `right_room`, so an error inside `right_room` or `highway_span_at` would not turn it red. Line 457 (box no larger than reserved) and 459 (the notation stays one line: the notation piece is `kKeepWhole`, `:387`) are properties. Lines 461, 465 and 467 depend on the string lengths in `kBurnout` and the widths 800 and 620; they stand in for "the loop reached a wrapped case and a shrunk case", as the record says. The strings in `kBurnout` (397-399) say "GreenCym" and "YellowCym"; the screen prints "[Kick - Yellow cymbal]" (`pitch_dark_preview_1_act.png`, the Next box: "at m57.1.0 · [Kick - Yellow / cymbal]"). That capture, at 1280 wide, shows the next box wrapping its "at ..." line, keeping "3+ (94.3 ms)" on one line and staying clear of the highway edge, which matches the claim at one width; 800 and 620 are not captured. Verdict stands. This is the same stale-label split as split 2 in `verify/C6.md`.

## OL20 (Investigate, claim not checked only): agree

By reading `ellipsize` (`overlay_layout.cpp:241-277`) against `ten_per_char` (`tests/fixed_pitch.h:9-14`: 10 px per non-continuation byte). "3- 1 2  (optimal)" is 17 characters = 170 px. At 170 and 500 the whole text returns (`whole_w <= max_w`, `:245`). At 169 the cut that fits is 15 kept characters plus "…" = 160 px, and the cut after "l)" would be 17 > 169, so the result is "3- 1 2  (optima…" as line 477 says. At 80: the candidate cuts at 7 and 8 characters both strip to "3- 1 2" (spaces dropped by `kept`, `:256-260`), 70 px fits, and a cut at 9 characters is 100 px and does not, so line 480 holds. At 15: the first cut is "3…" = 20 px > 15, so `lo == 0` and the result is "…" alone (line 482). The empty text fits (line 483). Each line is its own branch: 473-474 the fits branch, 477-478 the binary search, 480 the space strip, 482 the `lo == 0` branch. Callers are real screens (library column, report, settings bar, Preview), and no capture shows a cut label, so question 3 is "claim not checked"; nothing else is wrong with the test. Verdict stands.

## OL21 (Investigate, claim not checked only): agree

By reading. The built label is 14 groups of "2- " (3 characters) and 26 of "1 " (2), plus " (optimal)" (10): 42 + 52 + 10 = 104 characters, as the record says. At each of 600, 420, 333 and 95 px the shown text must fit (493), end in "…" (494-495) and be a prefix of the label (496); all hold by the binary search and `kept` (spaces stripped from the end of a prefix, so the prefix test still holds). Lines 500-501: "Café été" is 8 characters; at 50 px the 5-character cut "Café…" fits, and at 40 px "Caf…"; the continuation-byte test at `:254` is what keeps a cut off the middle of "é". Removing that test would turn those two lines red. "40 activations" stays a comment's claim: nothing in `records-engine/` shows a corpus chart with that many activations; the seven captured charts hold at most 4. Verdict stands: claim not checked only, needing a corpus chart with the longest best path.

## OL22 (Investigate, claim not checked only): agree

By reading. With the same inputs as OL20: 170 px whole gives `kept_w` 170 (`:246`); the cut at 80 px keeps "3- 1 2" (60 px, spaces not counted, `:274-275`); "…" alone gives 0 (`:271`); "Caf…" gives 30. The single real caller of the four-argument form is `ui/library_table.cpp:534`, where `kept_w` stops the title highlight; no capture shows a highlight running to a cut label, so Question 3 stays "claim not checked". Lines 509, 511 and 514 repeat OL20's 480, 482 and OL21's 501 strings; that is overlap, not a fault. If `kept_w` were computed on the text with the ellipsis, 510, 512 and 515 would turn red. Verdict stands.

## Splits

No new split beyond `verify/C6.md`. Notes from the nine:

1. OBJ1: the test's `kKickObj` and the shipped `assets/preview/models/drum-kick.obj` agree today; nothing ties them together, so they can drift.
2. OBJ6: line 132 checks the winding of the first triangle only; the quad's second triangle is unchecked.
3. OBJ8: the extents lines use tolerances wide enough (183, 184) to pass a model whose minimum y is off by about 0.05.
4. OL19: `kBurnout`'s "GreenCym" and "YellowCym" against the screen's "Yellow cymbal" (the stale-label split already listed as split 2 in `verify/C6.md`).

## Golden-file flags

None. NW15's rewrite uses `kCapSide - 10.0` and `kCapSide + 10.0`; OBJ8's rewrite uses properties. No record proposes a saved expected-output file.

## Not audited

None.
