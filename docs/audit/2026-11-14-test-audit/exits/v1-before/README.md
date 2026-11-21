# V1: each screen's text before the visible-group fixes

The main session ran these on 2026-11-21 (session e3d99f2b). The build was main at `465c33065f2b8de5a6d6595fd90a0926e5810bdb`, made in `worktrees\tf-r1-build`: `hydra_uitest`, `hydra_pathbuilder_uitest`, `hydra_batch` and `hydra_fillcompare`. Every chart is from the 98-chart corpus in `testdata/input`, never the user's library. A scout picked the charts and playheads from the chart files and the audit's saved R1 dumps. The command files are in `scripts/`, the printed output in `out/` and the screenshots in `shots/`.

The Preview's overlay boxes (time, score, drain, next activation, solo) are painted onto the highway with `draw_text_box`, so the `text` verb cannot read them. Their before text was read off full-frame screenshots.

The V0 stops were settled before this run (see the top of `../owner-map-V0.md`). Items 1, 6 and the fills spelling of item 14 have no visible change left, so they have no before text here.

## Item 2: the drain box

One tempo is read at the playhead and multiplied. Each playhead sits just before a tempo or meter change, so a time that follows the change would differ.

| Chart | Playhead | What changes next | Drain box before | Shot |
|---|---|---|---|---|
| Brandon Burkhalter - Someday | 2.000 s (m2.1.0) | 120 to 180 bpm at 4.000 s | SP drain (if activated) · 1 bar / 4.0 s · full meter 16.0 s | `shots/drain_someday_2s.png` |
| Nova Charisma - The Greater Cause | 128.000 s (m81.1.0, 3/4) | 5/4 at tick 155040 | SP drain (if activated) · 1 bar / 2.4 s · full meter 9.6 s | `shots/drain_greater_cause_128s.png` |
| Car Bomb - The Sentinel | 147.366 s (m104.1.0, 84 bpm, 5/16) | 324 tempo changes in the song | SP drain (if activated) · 1 bar / 1.8 s · full meter 7.1 s | `shots/drain_sentinel_147s.png` |

## Item 3 (B9): the solo box

On Mike Orlando - Tapped Out, at 246.86 s (mid-solo), the box reads "48%" and "68 / 140 notes" (`shots/solo_tapped_out_mid.png`). After the solo, at 256.23 s, it reads "+14,000" (`shots/solo_tapped_out_after.png`). The R1 dump's `score.solo` for that chart is 14000, so the box's own sum agrees with the engine on this chart. The other playheads, and Ladies Talk and Levitating, are in `shots/` as `v1_b9_solo_<n>.png` in script order.

## Item 4: "enough SP saved"

No before text. The shaded "saved" row is on the Path Builder's timeline, not on Hydra's Preview tab, whose top strip is the scrub bar. `hydra_pathbuilder_uitest` opens only the chart a test names (`h.chart_arg`, tests/pb_ui/pbtest_harness.cpp), and no command file can choose one. The scout also found that no corpus best path saves two or more phrases after its last activation. So no corpus chart shows the shading after the last activation, and the item has no before to compare.

## Item 5: the Path Builder's phrase counts

`hydra_pathbuilder_uitest --test build-a-path-by-clicking --test cant-activate-reason --test build-a-path-on-a-long-song` passed (`out/v1_item5_pb.out`). Those tests assert the exact text, so today's text is what they pin. Examples: "You have 0 phrases saved here. You need 2 to activate." and "You have 1 phrase saved here. You need 2 to activate." on Burnout; candidate rows such as "m58 → m68 4 phrases best" on Symphony X - Set the World on Fire. No corpus chart shows the engine's per-fill count and the gauge's count differing, because the engine keeps no per-fill count (O5 is `carry out`).

## Item 8: the hardest figure

A fresh analysis fills the figure for every path, so the gap (rows an older build left empty, `RecordStore::rows_missing_hardest`) cannot show here, and no store on disk holds such rows. Today's text, Paths tab, best path first:

- Evans Blue - Beg, "1- 0 0 E0": 0.0 ms (squeeze out 0.0 ms, early fill 0.0 ms).
- Mutemath - Allies, "0- 1- 1 0": squeeze outs -157.9 ms.
- Green Day - Burnout, "3- 1 2": 163.0 ms.
- Stone Temple Pilots - Trippin' on a Hole in a Paper Heart, "0 0 E2 E0": 0.0 ms.

The library search `squeeze>=0` showed 3 of the 4 analyzed charts and `squeeze<=1000` showed 4. The path report's "Hardest ms" column read 0.0 ms for the Trippin' row. Full output: `out/v1_item8_squeeze.out`.

## Item 9 (B9): the fill comparison page's count

No before text. `hydra_fillcompare` refused: "This database has no chart library. Run hydra_batch --whole-library, or scan in Hydra, to build one." A whole-library run needs the user's yes and is not asked for here. The scout also found that the 116 corpus chart files have 116 distinct hashes, so the page's two counts cannot differ on the corpus.

## Item 13: the all-0 name

On Beg the Preview's closed "Showing" combo reads `1- 0 0 E0  (optimal)`, and on Someday `1+ 1 0+ 2  (optimal)`. The Paths tab heads the section "Best all-0 path" and the best path "Optimal path · 7 stars". The open list, where `preview_path_label` writes "  (best all-0)", could not be opened from a command file: `**/##previewpath` finds no combo (tests/ui/uitest_preview.cpp), and the keyboard route did not open it. The list item's text is `<notation>  (best all-0)` by `src/app/preview_view.cpp` near line 824. Beg's best all-0 path is "0 0 0" at 319,890 (audit capture R4a/beg.txt), so its item would read `0 0 0  (best all-0)`. That last line is read from the code, not off the screen.
