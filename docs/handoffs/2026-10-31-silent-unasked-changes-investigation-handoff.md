# Handoff: investigate sessions making changes the user never asked for, and never saying so (2026-10-31)

Written by session 711858a7-e2d0-4bc0-89e7-f2226a576639. This is a fresh investigation. It is not the D13 fix: reversing D13 and building the Path Builder's step panel are planned in that session, on branch `claude/pbr3-plan4`, and nothing here should touch that work.

## The goal

The user, in this session: "this seems to be a recurring pattern, sessions silently making changes i never asked for and choosing to never mention it. create a handoff for another session to do a deeper investigaton".

Find out how often this has happened, how it happens, and what would stop it. "It" means a session changing something the user sees (a sentence, a label, a number, a default, a layout, a behaviour) that the user never asked for, and either never telling the user or telling them only after the change was made. The result is a written finding the user can act on, not a fix. Nothing gets built without the user's yes (CLAUDE.md rule 3).

## The example that started it

On 2026-10-04 at 00:26 UTC, session ade9655b asked the user one question during the step-1 review. Its full text, from that session's transcript:

> "A SqIn note sitting exactly on the old SP end (offset 0): is its phrase already collected for free, or still to earn? Today the rating says free (the engine counts a note on the SP end as inside SP), but the sentence says "hit it more than 0.0 ms early". 24 sentences in the test set are like this."

The user chose "Free, as the engine says (Recommended)". It became D13 in `docs/audit/2026-10-03-fix-decisions.md`. The question was about whether a squeeze-in is free. It never said what the Paths tab sentence would become. Right after the answer, the session reported: "The 24 sentences at offset 0 change to 'no more than 0.0 ms late'". So a visible sentence changed as a side effect of a ruling about something else, and the user heard of it only in a report afterwards. The commit that made the change (`c68982cfdb603d6dc1c47293b0954b4aa894fe12`) counted 41 changed sentences on the corpus.

It went further. On 2026-10-11, commit `0233140f59b43b0535832a629e0d5d8d0661db0a` (session ec019463, task concept-audit-G4) renamed the flag from `is_free` to `needs_no_timing`. After that, labels built on the flag read "no timing" for a squeeze-in at 0.0 ms. The Path Builder's ending buttons said "no timing needed" for one, while the squeeze-out on the same note read "0.0 ms". The user found this on 2026-10-31 and said: "why are you still lying?? 0ms is not the same thing as no timing", then "who came up with this rule and when was it approved by me? i never asked for suqeeze ins to be treated differently for no reasons", then "why does the squeeze in text say "no more than 0.0ms late" instead of "more than 0.0ms early" ??? who approved this text?? this makes no sense".

Three mechanisms show in this one example, and each is a lead for the investigation:

1. **The question hid the visible change.** The user was asked about a rule ("free or still to earn") and not about the wording the rule would produce.
2. **The report came after the change.** The new wording was announced after it was decided, in a progress report, not asked first.
3. **A later change made it more visible without a new question.** A rename meant as an internal tidy-up changed what the screen says.

## Earlier instances already on record

The rulings pages in `docs/agents/rulings/` hold several earlier complaints of the same kind. Start from them. Each needs its source checked before it is counted (rule 9):

- In `code-and-engine.md`: "96.2ms is wrong... i did not ask you to change that" (2026-08-19), and "i don't appreciate you trying to sneak in the possibility of something becoming inaccurate or working differently. hydra is expected to keep the same functionality it has today." There is also a 2026-10-05 entry, "why did a commit silently change behavior?", and a 2026-10-15 one, "when did you decide it was acceptable to not include everything?" (the report had left charts out silently since D87).
- In `ui-and-display.md`, 2026-10-17: "why did you collapse multiplier squeezes and score breakdown? i never asked for that".
- In `review-and-hooks.md`, 2026-10-30: "so why was this designed without my approval?"

The user's memory note `no-silent-functionality-changes` already states the rule ("Never present user-facing behavior changes as side "implications" of a design"). It has existed since August, and the pattern kept happening. So restating the rule is not the answer. The investigation has to find where it fails.

## What to do next

This is an audit, so first read `docs/agents/audit-method.md`. It says how an audit runs, the four exits every finding leaves by, and why past audits lost findings. Then:

1. Build the list of candidates. Use the D-records in `docs/audit/2026-10-03-fix-decisions.md` and the session cards in `docs/sessions/` (grep `docs/sessions/index.md`). Look for decisions whose commit changed more user-visible text or behaviour than the question named. Compare the user's chosen option, as the card quotes it, with what the commit's diff changed on screen. The squeeze sentence is the model case. Also look for commits that changed visible text with no D-record or ruling behind them. Renames, "one owner" refactors and wording tidy-ups are likely places.
2. For each candidate, write down three facts with their sources: what the user was asked, in their words or the card's; what the commit changed that a player sees; and whether, when and how the user was told. Use the code and git history, not other agents' summaries.
3. Name the mechanisms: where in the workflow the visible change slipped through. Is it in how questions are written, in how reports are written, in reviews that check correctness but not "was this asked for", or in refactors that touch display text? Count each mechanism.
4. Propose what would catch it, without building it. Examples: a review question, a hook, a rule in the preamble, or a test that pins user-visible strings. Each proposal goes to the user as one question.
5. Every candidate that turns out to be a real unasked change is a split. Under the audit method, it goes to the user unless a ruling in the user's own words names both sides. Never close one quietly.

Do not fix any of the changes you find. Do not touch D13 or the squeeze wording: that is planned in session 711858a7. Do not run anything over the user's whole song library (rule 8). Corpus runs on `testdata/input` are fine.

## Claims

| Kind | Value | Claim | How to check |
|---|---|---|---|
| commit | `c68982cfdb603d6dc1c47293b0954b4aa894fe12` | D13's fix: one owner for "is this squeeze free" (`SPSqueeze::is_free`). It changed the squeeze-in sentence at offset 0 from "more than 0.0 ms early" to "no more than 0.0 ms late"; its message counts 41 changed corpus lines. On `main` and pushed. | `git show c68982cfdb603d6dc1c47293b0954b4aa894fe12` |
| commit | `0233140f59b43b0535832a629e0d5d8d0661db0a` | M41 (2026-10-11, session ec019463, task concept-audit-G4): renamed `is_free` to `needs_no_timing`. Its message says the on-screen word "Free" does not change. On `main` and pushed. | `git show 0233140f59b43b0535832a629e0d5d8d0661db0a` |
| path | `docs/audit/2026-10-03-fix-decisions.md` | Holds D13 at line 31: "A SqIn exactly on the old SP end (offset 0) is already collected for free … The sentence at offset 0 reads "no more than 0.0 ms late"." | read line 31 |
| path | `docs/sessions/2026-10-03-ade9655b.md` | The session card that records D13's ruling: chose "Free, as the engine says (Recommended)" (00:26 UTC). The card keeps only the start of the question; the full text is quoted in this handoff's prose. | read the Rulings section |
| path | `docs/audit/2026-10-03-derivation-audit.md` | Finding 144 is the audit finding that led to D13: two places decided "free or still to earn" with different sign tests. | grep "#### 144." |
| path | `docs/agents/rulings/code-and-engine.md` | Earlier complaints of the same kind (2026-08-19, 2026-10-05, 2026-10-15, and "i don't appreciate you trying to sneak in…"). | grep "silent", "did not ask" |
| path | `docs/agents/rulings/ui-and-display.md` | 2026-10-17: "why did you collapse multiplier squeezes and score breakdown? i never asked for that". | grep "never asked" |
| path | `docs/agents/rulings/review-and-hooks.md` | 2026-10-30: "so why was this designed without my approval?". | grep "without my approval" |
| path | `docs/agents/audit-method.md` | How an audit runs and the four exits; read before planning. | read it |
| path | `src/app/path_view.cpp` | Holds `squeeze_sentences`, which writes the Paths tab's squeeze sentences, including the D13 wording. | grep `squeeze_sentences` |
| function | `SPSqueeze::needs_no_timing` | The D13 rule, in `src/core/model.h`: a squeeze-in at offset 0 or below needs no timing; a squeeze-out on the end does. | `git grep -n "needs_no_timing() const" -- src/core/model.h` |
| function | `paid_by_sp_walk()` | In `src/core/backend_value.h`: the SP walk counts a note at or before the end (offset <= 0) as inside Star Power. `needs_no_timing` reads it. | `git grep -n paid_by_sp_walk -- src/core/backend_value.h` |
| branch | `claude/pbr3-plan4` | Session 711858a7's plan branch for reversing D13 and the step panel. Not merged. Out of scope for this investigation. | `git log main..claude/pbr3-plan4 --oneline` |
| state | The Path Builder is not on `main`. | Its "no timing needed" button (`kNoTimingNeededText` in `src/pathbuilder/pb_window.h`) and `squeeze_short_text` ("…, no timing", `src/pathbuilder/pb_text.cpp`) exist only on `claude/pb-redesign`. | `git grep -n kNoTimingNeededText claude/pb-redesign -- src/pathbuilder` |
| ruling | 711858a7-e2d0-4bc0-89e7-f2226a576639 | "this seems to be a recurring pattern, sessions silently making changes i never asked for and choosing to never mention it. create a handoff for another session to do a deeper investigaton" | the session transcript |
| ruling | 711858a7-e2d0-4bc0-89e7-f2226a576639 | "why does the squeeze in text say "no more than 0.0ms late" instead of "more than 0.0ms early" ??? who approved this text?? this makes no sense" | the session transcript |
| ruling | 711858a7-e2d0-4bc0-89e7-f2226a576639 | "who came up with this rule and when was it approved by me? i never asked for suqeeze ins to be treated differently for no reasons" | the session transcript |
| ruling | 711858a7-e2d0-4bc0-89e7-f2226a576639 | "why was this presented to me in the first place? why did that session think different squeezes needed their own rules?" | the session transcript |
| ruling | 711858a7-e2d0-4bc0-89e7-f2226a576639 | "why are you still lying?? 0ms is not the same thing as no timing" | the session transcript |
| ruling | ade9655b-b3ed-451c-8dfd-0a9077ee2da1 | D13: chose "Free, as the engine says (Recommended)" to "A SqIn note sitting exactly on the old SP end ..." (00:26 UTC) | `docs/sessions/2026-10-03-ade9655b.md` |

## Checker output

```text
ok | commit | c68982cfdb603d6dc1c47293b0954b4aa894fe12 | D13's fix: one owner for "is this squeeze free" (`SPSqueeze::is_free`). It changed the squeeze-in sentence at offset 0 from "more than 0.0 ms early" to "no more than 0.0 ms late"; its message counts 41 changed corpus lines. On `main` and pushed.
ok | commit | 0233140f59b43b0535832a629e0d5d8d0661db0a | M41 (2026-10-11, session ec019463, task concept-audit-G4): renamed `is_free` to `needs_no_timing`. Its message says the on-screen word "Free" does not change. On `main` and pushed.
ok | path | docs/audit/2026-10-03-fix-decisions.md | Holds D13 at line 31: "A SqIn exactly on the old SP end (offset 0) is already collected for free � The sentence at offset 0 reads "no more than 0.0 ms late"."
ok | path | docs/sessions/2026-10-03-ade9655b.md | The session card that records D13's ruling: chose "Free, as the engine says (Recommended)" (00:26 UTC). The card keeps only the start of the question; the full text is quoted in this handoff's prose.
ok | path | docs/audit/2026-10-03-derivation-audit.md | Finding 144 is the audit finding that led to D13: two places decided "free or still to earn" with different sign tests.
ok | path | docs/agents/rulings/code-and-engine.md | Earlier complaints of the same kind (2026-08-19, 2026-10-05, 2026-10-15, and "i don't appreciate you trying to sneak in�").
ok | path | docs/agents/rulings/ui-and-display.md | 2026-10-17: "why did you collapse multiplier squeezes and score breakdown? i never asked for that".
ok | path | docs/agents/rulings/review-and-hooks.md | 2026-10-30: "so why was this designed without my approval?".
ok | path | docs/agents/audit-method.md | How an audit runs and the four exits; read before planning.
ok | path | src/app/path_view.cpp | Holds `squeeze_sentences`, which writes the Paths tab's squeeze sentences, including the D13 wording.
ok | function | SPSqueeze::needs_no_timing | The D13 rule, in `src/core/model.h`: a squeeze-in at offset 0 or below needs no timing; a squeeze-out on the end does.
ok | function | paid_by_sp_walk() | In `src/core/backend_value.h`: the SP walk counts a note at or before the end (offset <= 0) as inside Star Power. `needs_no_timing` reads it.
ok | branch | claude/pbr3-plan4 | Session 711858a7's plan branch for reversing D13 and the step panel. Not merged. Out of scope for this investigation.
manual | state | main | Its "no timing needed" button (`kNoTimingNeededText` in `src/pathbuilder/pb_window.h`) and `squeeze_short_text` ("�, no timing", `src/pathbuilder/pb_text.cpp`) exist only on `claude/pb-redesign`.
manual | ruling | 711858a7-e2d0-4bc0-89e7-f2226a576639 | "this seems to be a recurring pattern, sessions silently making changes i never asked for and choosing to never mention it. create a handoff for another session to do a deeper investigaton"
manual | ruling | 711858a7-e2d0-4bc0-89e7-f2226a576639 | "why does the squeeze in text say "no more than 0.0ms late" instead of "more than 0.0ms early" ??? who approved this text?? this makes no sense"
manual | ruling | 711858a7-e2d0-4bc0-89e7-f2226a576639 | "who came up with this rule and when was it approved by me? i never asked for suqeeze ins to be treated differently for no reasons"
manual | ruling | 711858a7-e2d0-4bc0-89e7-f2226a576639 | "why was this presented to me in the first place? why did that session think different squeezes needed their own rules?"
manual | ruling | 711858a7-e2d0-4bc0-89e7-f2226a576639 | "why are you still lying?? 0ms is not the same thing as no timing"
manual | ruling | ade9655b-b3ed-451c-8dfd-0a9077ee2da1 | D13: chose "Free, as the engine says (Recommended)" to "A SqIn note sitting exactly on the old SP end ..." (00:26 UTC)
checked 20 rows: 13 ok, 0 missing, 0 not-on-main, 0 not-pushed, 7 manual
```
