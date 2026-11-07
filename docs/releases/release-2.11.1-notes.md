Hydra Deluxe 2.11.1 draws characters its own fonts don't have. Before, many of them showed as `?`.

**Your saved results carry over.** Nothing reads `Stale` after the update, and nothing is analyzed again. Scores and paths don't change.

**Update from the app.** If you run 2.11.0, a line under the toolbar offers this release; click `Update`. The line appears after the daily check. To check right away, click `Check now` in `Analysis settings`. From 2.10.2 or earlier, download the `-setup.exe` file below and run it.

**Song names in every script read correctly.** Hydra Deluxe ships two fonts, one for text and one for fixed-width text such as paths and numbers. Each lacked characters that song names use, and drew them as `?`. The text font has no Korean, Thai, Greek or Cyrillic letters, no letters like `ł` or `š`, and no `‐` hyphen. The fixed-width font has no Japanese, Korean, Cyrillic or Thai letters. Only the fixed-width font could fall back to a Windows Japanese font, and the text font had no fallback at all. Hydra Deluxe now borrows any character a font lacks from the fonts Windows already has, in both fonts, so every part of the window gets it. Emoji draw too, in one color. A character still shows as `?` when no font on your Windows has it.
