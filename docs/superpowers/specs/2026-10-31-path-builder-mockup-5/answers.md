# The fifth Path Builder mockup: a slimmer top (2026-10-31)

Session 711858a7. At the RA-7 real-app check the user opened The Reaper in the check build of the Path Builder and sent `builder-top-before.webp`, with: "getting better but still way too much clutter in the interface, especially up top".

Between the song line and the highway, that screen stacks: the song line with three scores and three buttons; a two-line paragraph ("Build your own Star Power path, left to right. Click a glowing fill…"); a coach line ("Your Star Power runs until m72. Save 2 more phrases, then activate again. The next glowing fill is m83. Your best next choice is m83."); a legend row of seven colour keys with "Click anywhere on the timeline to move the highway there." and the "Show Hydra's best path" box; then five timeline rows (Section, Star Power phrases, Star Power saved with its own two-line caption, Your activations, Fills) and the scroll row.

Asked "Mock up a cleaner top area before changing any code?", the user chose "Yes, mock it".

`top-mockup.html` is the page the user clicked through. It is the third mockup's page (the one the builder was built from) with the top changed. Its data file, `pb-data.js` (Divine Inner Tension by Hail The Sun, about 45 MB of engine output), is not committed; the page needs it beside it to run. The panels below the timeline are the third mockup's and were not part of the question.

## What the page shows

- Gone: the paragraph, the coach line, the legend row, and the "Star Power phrases" row.
- "Show Hydra's best path" sits beside Undo, Start over and Finish the rest for me.
- The stars sit along the top of the "Star Power saved" graph, which keeps its dashed 2-phrase line and its shading.
- Your activations share one row with the fills, labelled "Your activations"; each bar starts at the fill it was made from.
- The legend's words, unchanged, are the hover text of the row labels: "Star Power saved" shows "★ Star Power phrases", "- - - 2 phrases: needed to activate" and "Shaded: enough to activate"; "Your activations" shows the six fill keys.

Asked "Does this top look right after you click through it?", the user answered "Looks right".

The page did not cover what the coach line also carried: the refusal shown when a player clicks a fill they can't activate at, and the left-to-right sentence for a fill before the last activation. Where those go is for the plan to ask.
