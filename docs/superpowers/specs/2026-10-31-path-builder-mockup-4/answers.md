# The fourth Path Builder mockup: the step panel (2026-10-31)

Session 711858a7. `step-panel-mockup.html` is the page the user clicked through. It covers only the "Your activations" panel. `reaper-m48-m83-target.json` is what `hydra_replay target --ticks "90240,157440" --prodrums 1 --bass2x 1` returned for "(Don't Fear) The Reaper (Rh)" by Blue Öyster Cult: every way to end activations at m48 and m83. The m83 activation has two endings: "2-" (SqOut, offset 0.0 ms, Star Power ends at m89) and "2+" (SqIn, offset 0.0 ms, Star Power ends at m91).

## The problems, in the user's words

On the opened step 2 (m83 → m89, with "End it at:" buttons "m91 · no timing needed · −10,380" and "m89 · 0.0 ms · best"): "way too much yapping and no explanation for why starpower ends twice".

After clicking the next fill (m103): "i clicked to progress to the next activation and there is no answer for which option i chose or why it was chosen".

## How the page got here

The main session first proposed a panel with its own explanations. The user answered: "you dont need to explain what a squeeze is. but the user needs to know if theres a squeeze there." On a revised preview the user chose "Mock it first".

On the first page the m91 ending read "no squeeze". The user: "no squeeze is wrong there. it would be squeeze in". The main session changed it to "squeeze-in" without data. The user: "your mock is still inventing its own text instead of using what the engine already knows". The main session then read the engine's endings (the JSON above) and used the builder's short form for squeezes (`squeeze_short_text` in `src/pathbuilder/pb_text.cpp` at the time, as the "Where to activate next" rows printed it), which gave "squeeze-in, no timing" for m91. The user: "why are you still lying?? 0ms is not the same thing as no timing".

That wording comes from D13 (2026-10-04): `SPSqueeze::needs_no_timing` in `src/core/model.h` calls a SqIn at offset 0 needing no timing, while the SqOut on the same note reads 0.0 ms. Asked where that came from, the user said: "who came up with this rule and when was it approved by me? i never asked for suqeeze ins to be treated differently for no reasons", then "why was this presented to me in the first place? why did that session think different squeezes needed their own rules?". The main session explained the origin (two copies of one check disagreed at exactly 0; the fix sided with where a perfectly timed hit lands) and asked: "Reverse D13 everywhere, planned with the step panel?" The user answered "Yes, plan it".

On the Paths tab, a squeeze-in at exactly 0.0 ms had a sentence saying to hit the note "no more than 0.0 ms late". The user (2026-10-31, with a screenshot): "why does the squeeze in text say "no more than 0.0ms late" instead of "more than 0.0ms early" ??? who approved this text?? this makes no sense".

The builder's short form still had its own words. After seeing "squeeze-in, no timing" in it again, the user said: "squeeze in no timing again??? why do you keep inventing your own text instead of using what already exists?" Hydra already words every squeeze and early fill: the Paths tab's activation badges (`activation_badges` in `src/app/path_view.cpp`, D265), "squeeze in 0.0 ms", "squeeze out 130.4 ms", "squeeze in -130.4 ms", "early fill 30.0 ms". So the builder's own short form goes, and every place the builder names a squeeze (step rows, ending lines, the "Where to activate next" rows and the fill card's "Hydra would end it at" line) uses the badge words, joined by " · ". An early fill the badges list is kept, as the Paths tab shows it. The page below uses the badge words.

## What the page shows

- A step's row: its span, its phrases and its timings in the Paths tab's badge words ("m83 → m89 · 3 phrases · squeeze out 0.0 ms"), and its cost. A step with no timing to name shows no badge words.
- The step being built, opened, when it has more than one ending: the builder's existing label "End it at:", then one line per ending ("ends m89 · squeeze out 0.0 ms" with "best", "ends m91 · squeeze in 0.0 ms" with "−10,380"), the kept one marked, the others clickable.
- An earlier step, opened after the player has moved on: the same lines, greyed and not clickable, with the kept one still marked.
- A step with one ending: no lines, only Remove.
- Gone from the opened step: "Activate at … with … saved. Star Power runs until …", "This choice costs …", the before/after sentence, "You still have Star Power to use …" and the Cost/Squeeze glossary.

The user did not approve the page itself. "Yes, plan it" approved planning the D13 reversal together with this panel; the plan goes to the user for its own yes.

Two figures on the page are not the engine's: the step's cost of −12,440 when m91 is picked (the main session's sum of −2,060 and −10,380), and the m91 squeeze-in's figure, which the page shows as the engine's raw offset, 0.0 ms, as the user asked.
