# The third Path Builder mockup: what the user decided (2026-10-31)

Session 711858a7. This mockup answers four problems the user found in the real app after the second mockup was built. The spec is `mockup_template.html` in this folder. `path-builder-mockup-3.html` is the same page with its data inside, so it opens on its own. Everything not named below stays as the second mockup has it (`../2026-10-31-path-builder-mockup-2/answers.md`).

## The problems, in the user's words

- On the fill's hover tooltip: "this is a giant block of white text that is difficult to read".
- On the 42-minute album squeezed into one width: "we need to plan out how to handle longer tracks. maybe some kind of scroll?"
- "moving across the timeline should not autoplay audio. the dots are small and difficult to hover oer".

## The four answers, word for word

1. **Hover.** Asked: "Hovering a fill: today it pauses, jumps the Preview to the fill, and on leave jumps back and resumes playing if it was playing. What should hover do?" Answered "Show, never play (Rec.)", whose text was "Hover pauses and shows the fill on the highway; moving off jumps back but stays paused. Only Play or Space starts sound."
2. **The hover target.** Asked: "The fill dots are hard to hit (1.15 rem wide). How should we make them easier?" Answered "Bigger hit area (Rec.)", whose text was "The pointer picks the nearest fill anywhere in the Fills row's height, out to halfway to the next fill. Dots look the same."
3. **The tooltip.** Asked: "The fill tooltip is six white sentences. How should it read?" Answered "Short card (Rec.)". The preview the user saw was a bright heading ("m87 · 2:25.655"), then three rows ("Phrases saved 4", "Best finish here 840,645", "Cost −67,440" in red, the tier colour), then two grey lines ("Best choice now is m28 (908,085)." and "Hydra would end it at m97.").
4. **Long songs.** Asked: "Long songs (this album is 42 minutes) squeeze the whole timeline into one width. Which way should I mock up for you to click through?" Answered "Zoomed window + scroll (Rec.)", whose text was "The timeline shows a few minutes at a time, follows the playhead, and scrolls sideways; a thin overview strip above shows the whole song and where you are."

Answers 1 to 3 are built as answered. When a fill costs nothing, the card's Cost row reads "best" (the list's word) and its first grey line reads "This is the best choice now." A locked, refused or selected fill gets the heading and one grey line.

## The open questions in the purple bar

Each has its recommendation selected; the other options are one click away. The 3 minutes and 2.5 rem per measure are stand-ins so the window can be drawn. They are not settled numbers.

1. How much song the window shows. Recommended: fixed minutes. Other: a fixed width per measure.
2. While playing. Recommended: the window follows the playhead (it turns a page when the playhead passes three quarters of the way), and the user's own scroll stops that until Play is pressed again. Others: always follows; never follows.
3. What the strip shows. Recommended: sections, fills tinted by cost, your activations and the window's frame. Others: the same without fills; the frame only.
4. A click or drag on the strip. Recommended: a click centres the window there and dragging the frame scrolls it. Other: the same, and a click also moves the Preview's clock there.
5. A list row whose fill is outside the window. Recommended: hovering or clicking the row scrolls the timeline to it. Other: only a click scrolls.
6. The mouse wheel over the timeline (added, because a scrolling timeline forces it). Recommended: it scrolls sideways. Other: it does nothing, and the strip is the only way to scroll.
7. List row click. Recommended: it moves the Preview's clock to the fill, as a fill click does. Other: it leaves the clock where it was, as the build does today.
8. Empty list. Recommended: the side column stays 27 rem, as the build has it. Other: it narrows to 24 rem, as the second mockup did.
9. Section stripes. Recommended: they alternate over the sections shown, as the build does. Other: they count the hidden "End" section too, as the second mockup did.
10. Saved-row caption. Recommended: "2 phrases: needed to activate" wraps onto a second line if it must. Other: it stays on one line and is cut with "…". The bar's "Font size to view at" switch (15 px or 20 px) shows the wrap; it is a viewing aid, not a question.

## The user's answers to these questions

(To be filled in after the user clicks through.)

## The data

The song is "Divine Inner Tension (Album)" by Hail The Sun, the 42-minute chart from the real-app check. The options are the user's, read from one row of a copy of their database: Expert Pro Drums, 2x kick, SP cap 4, 10 ms Path limit. `build_data.py` says how to rebuild the data. The data file (`pb-data.js`, about 9 MB) is not committed; the inlined page holds it.

The deep dump finished in about 8 seconds, well inside the 10-minute cap, so the album was used and The Last Baron was not needed. But it held 27,020 paths and 369,784 distinct window prefixes, and the mockup prices each prefix with one engine run. So the mockup keeps only the paths within 20,000 points of the best: 878 paths and 12,260 prefixes, priced in about 76 seconds with no errors. The second mockup's gap-filling step ran past a 200-second cap on this chart and was dropped. A fill that no kept path activates at shows grey, with "no engine numbers" on its card. Because of that cut, red fills (cost of 1% of the best or more, about 49,900 points here) are rare in this mockup.

The check: the path with two activations, at tick 74,880 (m40, ending at tick 90,240) and at tick 122,880 (m65, ending at m69), scores 4,140,915 in `hydra_replay score` with that path, and the mockup's "Your score" for the same two activations is 4,140,915.
