# Rulings: questions, splits and audits

How a question goes to the user, what to do when Hydra answers one question two ways, and when an audit finding may close. The line format is in README.md beside this file. Newest first.

Under Confirmed are the user's exact words, filed with the user's approval or through the backfill route; under Unconfirmed are older lines written by sessions, which guide sessions as working rules but are never cited as the user's ruling.

## Confirmed

- **2026-11-14** "i dont understand why you always try to invent insane edge cases, this one isnt even possible in clone hero. you cant just activate arbitrarily whenever you want" (session ae1c19f5)
- **2026-10-31** "i think this would cause more trouble than it would fix. agents are very very bad at remembering to update things and im sure there are tons of stale comments throughout the project that no longer accurately describe their code" (session a7d95f1c)
- **2026-10-29** "when did they diverge and why? did i ever ask for it?" (session caca5d75)
- **2026-10-29** "why are we even using two different systems? why are you still trying to convince me of more convoluted bullshit?" (session 61cee421)
- **2026-10-29** "yes everything should use one rule." (session 61cee421)
- **2026-10-29** "so what went wrong here, and how can i trust the results of any audit when this keeps happening?" (session b4409ae9)
- **2026-10-29** "what are we already doing and why are you asking me to re derive it" (session caca5d75)
- **2026-10-19** "when did I tell you it was acceptable to remove activations?? why is hydra activating on nothing? i never approved this design" (session 7bd3ac91)
- **2026-10-19** "Bring back the old look: light the lanes for the half measure before the chord, display only, engine timing unchanged." (session 7bd3ac91)
- **2026-10-31** chose "Yes, add it (Recommended)" to "Proposal 4. Should the reviewer check "was this asked for?" Today the one reviewer checks that a change is correct, then fixes and signs off itself. The added step: it lists every change you can see in the diff, next to your words that asked for it. Any change without your words goes to you as a question (rule 12) instead of being signed off. In D13 the reviewer counted a ruling about the rule as covering the new sentence. It changes nothing on Hydra's screen. Recommended: yes. Strongest case against: one more step for every reviewer, and it depends on the reviewer finding your words." (session a7d95f1c)
- **2026-10-31** chose "Yes, add it (Recommended)" to "Proposal 5. Should a commit or release never include edits nobody in that session made or was asked to make? In August an unexplained edit found in the working tree went into release 1.7.3 under "build to v1.7.3", and it hid the eff. figure. Rule 12 already covers stray edits you can see; this would also cover ones you can't, such as code-only edits. Found edits get reported and left out until you say what they are. Recommended: yes. Strongest case against: your own unsaved edits in the main checkout would be held back until you confirm them." (session a7d95f1c)
- **2026-10-31** chose "Yes, ban them (Recommended)" to "Proposal 2. Should opt-out lines be banned? Today a session can tell you about a change you didn't ask for with lines like "I'm treating it as fine unless you say otherwise" or "Say the word and I'll hide it", and the change stays if you don't answer. Five changes stayed that way (cases 4 to 8). The rule would be: any visible change your words didn't ask for becomes a question before it merges, and nothing merges while it is unanswered. It changes nothing on Hydra's screen. Recommended: yes. Strongest case against: more questions, and small work can wait on you." (session a7d95f1c)



## Unconfirmed

- **2026-10-31** "Yes, require it", to: "Should every rule question list each screen's before and after text?" (session a7d95f1c-a1ce-46e9-bf4d-2e7cfbd98d1e; note: questions-describe-the-real-rule; backs: docs/agents/agent-rules.md rule 11)
- **2026-10-29** "why are we even using two different systems? why are you still trying to convince me of more convoluted bullshit?" and ruled "yes everything should use one rule" for the Backend timings rows (M39/306/33). (session 2026-10-29-caca5d75, by date; note: never-keep-a-split-quietly; backs: docs/agents/audit-method.md)
- **2026-10-29** As the note records it: It closes only on the user's words that name both sides. The five rules the user approved on 2026-10-29 ("yes both"): a split is a question by default; "already decided" must quote the ruling, and the quote must name both values. And: The user asked "how can i trust the results of any audit when this keeps happening?" (session: not stated; note: audit-closures-need-quoted-ruling; backs: docs/agents/audit-method.md)
- **2026-10-07** As the note records it: The user approved wording ("Click Scan library first") that would have been false there. (session: not stated; note: questions-describe-the-real-rule; backs: no page yet)
- **2026-10-05** As the note records it: The user approved it in a bulk "all recommended", and E3 shipped the wider rule. (session: not stated; note: questions-describe-the-real-rule; backs: no page yet)
