# The Preview golden image

`tests/test_preview_golden.cpp` renders the chart in this folder with Hydra's
Preview and compares the frame against `golden_onyx.png`, a screenshot of Onyx
showing the same chart at the same moment. ADR 0008 says why: the Preview is a
port of Onyx's previewer, and this image is how "faithful" gets checked.

## What is here

`golden.json` describes the capture, and its `_comment` is the record of how it
was taken. Today that record says:

1. The Onyx build was 20251011.
2. The chart is "blink-182 - What's My Age Again" (the Hoph2o sync chart). Its
   `notes.mid` sits beside this file.
3. The track shown was Expert Pro Drums.
4. Onyx was paused at 0:36.913, which is the `time_ms` the test renders at.
5. `golden_onyx.png` is the whole Onyx window (1372 by 881 pixels). The `crop`
   key names the highway area inside it, 1372 by 714 starting 126 pixels down.
   That area is what Hydra renders.
6. The `mask` rectangles cover Onyx's time box and track label, which Hydra does
   not draw.

The test reads every key in `golden.json` and fails if one is missing.

## Recapturing

Recapture only when the look must change on purpose, after changing Onyx's
inputs as ADR 0008 describes. The `crop` size must equal `width` and `height`
in `golden.json` (the test checks this), or the comparison no longer lines up.

1. Install the Onyx build you are capturing from and note its version for the
   `_comment`.
2. Open the chart in this folder in Onyx's previewer and pick Expert Pro Drums.
3. Size the Onyx window so the screenshot is as wide as `golden.json`'s `width`
   and the highway area is as tall as its `height`. *To confirm at the next recapture:* the window size
   Onyx was set to, and whether the screenshot includes the title bar, are not
   recorded anywhere.
4. Pause at exactly 0:36.913. *To confirm at the next recapture:* how the pause
   landed on that exact time is not recorded anywhere.
5. Screenshot the whole window and save it over `golden_onyx.png`.
6. Find the highway area in the screenshot. Set `crop` to it; its width and
   height must equal `width` and `height`.
7. Check that the `mask` rectangles still cover the time box and track label.
8. Update `_comment` with the build, chart, track, pause time and anything new
   you learned about steps 3 and 4.
9. Run the golden test alone
   (`build-cpp\Release\hydra_tests.exe -sf=*test_preview_golden*`). With
   `HYDRA_PREVIEW_GOLDEN_DUMP` set, it also writes the two frames as BMPs, the
   per-tile error map and the per-pixel delta histogram into the working
   folder, which help line up the crop.

## The thresholds

The test file owns the measurement: `max_channel_deltas` finds each pixel's
difference, `over_share` finds the worst tile and the whole frame, and `kTile`
is the tile size. This page only says what each key is for. Four keys in
`golden.json` set how close the frames must be, all in units of that code:

1. `tolerance` caps the mean difference, out of 255.
2. `pixel_delta` is the line between a matching pixel and a wrong one, out of
   255.
3. `tile_percent` caps the share of wrong pixels in the worst tile. This catches
   a small wrong piece, like a missing note, that the mean hides.
4. `frame_percent` caps the share of wrong pixels in the whole frame.

The user chose these values on 2026-10-10 from a measurement of two renders.
Today's render had a worst tile of 6.25 percent, a whole frame of 0.0525
percent and a mean of 5.688. A render with Pro Drums off, which draws some
notes differently from the screenshot, had a worst tile of 32.8 percent, a whole frame of 0.125
percent and a mean of 5.727. The mean barely moved, which is why the tile and
frame checks exist. The decision and its reasoning are under "User decisions",
item 3, in `docs/superpowers/plans/2026-10-10-test-fidelity-fixes.md`.

After a recapture, measure again before changing any of the four keys: run the
golden test with `HYDRA_PREVIEW_GOLDEN_DUMP` set, read the printed numbers and
the CSVs it writes, and take new values to the user.
