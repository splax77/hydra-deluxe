# Public doc style

This page says how to write anything a Hydra Deluxe player reads: `README.md`, `docs/README.md`, `docs/UserGuide.md`, `docs/differences-from-public-hydra.md`, and the GitHub release notes. Internal docs follow the user's "How to explain things" rules in `~/.claude/CLAUDE.md` instead; this page adds to those, it doesn't replace them.

The goal is a page that sounds like someone who knows the app explaining it to another Clone Hero drummer. Accuracy comes first. A wrong label, default or number does more harm than any stock phrase, so check every name and number against the code before you write it.

The rules come from two web searches on 2026-10-10. The main sources were [Wikipedia: Signs of AI writing](https://en.wikipedia.org/wiki/Wikipedia:Signs_of_AI_writing), [tropes.fyi](https://tropes.fyi/directory), the [Google developer style guide](https://developers.google.com/style/highlights), the [Microsoft style guide](https://learn.microsoft.com/en-us/style-guide/brand-voice-above-all-simple-human), [digital.gov plain language](https://digital.gov/guides/plain-language/writing), [Write the Docs](https://www.writethedocs.org/guide/writing/docs-principles/), [Diátaxis](https://diataxis.fr/), and the READMEs of [ripgrep](https://github.com/BurntSushi/ripgrep), [YARG](https://github.com/YARC-Official/YARG) and [Moonscraper](https://github.com/FireFox2000000/Moonscraper-Chart-Editor). Every source treats these patterns as habits to fix, not proof that a machine wrote something. One on its own means little. A page full of them reads as generated.

## Voice

Write to the reader as "you", in the present tense and the active voice. "Hydra Deluxe analyzes the song when you click it", not "The song will be analyzed once it is selected."

Don't write in the first person. No "I" or "we". The app is "Hydra Deluxe" (or "Hydra" in release notes from before 2.0.0, when that was its name).

Lead each section, and each paragraph, with its main point. The edge cases come after it.

Where a player would ask "why?", give the reason in one plain sentence. "Rows above optimal are expected: many leaderboard scores were set on older Clone Hero versions whose fill rules allowed totals that are impossible now."

Say plainly what the app can't do, and what is a what-if rather than a playable path.

Use specifics: the real label, a real number, a real example. Every number says where it comes from, such as a setting, a default, the test charts or a measured run. Never invent a number, a timing or an example song.

Be friendly and direct. No hype and no false enthusiasm.

## Words to cut

Cut puffery and significance words: seamless, robust, powerful, comprehensive, effortless, crucial, pivotal, key (as an adjective), vibrant, showcase, leverage, empower, enhance, foster, delve, intricate, landscape, testament, streamline. Replace each with the plain fact, or delete it.

Write "is" where "serves as", "stands as", "acts as" or "represents" would go.

Cut throat-clearing and signposts: "It's worth noting", "Note that", "In this section", "Let's", "Simply", "just" as filler, "In order to", "To summarize", "In conclusion".

Don't hedge to hide a missing fact. If something is unknown, say it's unknown.

Use one name per thing, and use the app's own label for every control. Don't cycle synonyms: if the window is the "path report", don't also call it the index or the table page.

## Sentence shapes to avoid

Avoid "not X, but Y" and "it's not X, it's Y". State the positive claim.

Don't group things in threes by reflex. List as many items as there really are.

Don't end a sentence with an "-ing" clause that adds vague weight, like "..., making it easy to find your songs." End on the point.

No self-asked questions ("The result?") and no teaser fragments.

No em dashes in prose. Use a period, comma, colon or parentheses. Keep a dash only where it's part of text the app shows, such as a window title.

Keep sentences short, one idea each, but vary the length a little so the page doesn't read as a drumbeat of eight-word lines.

## Formatting

Write headings in sentence case, and make them say what's in the section ("Compare with dmleaderboards", not "Overview"). Never give two headings in one page the same text. Keep a heading's text when other pages link to it, or update every link.

Don't start bullets with a bold label and a colon. Write short paragraphs instead, or a plain list, or a table when the content is real reference material (settings and their defaults, report columns, keys).

Use a list only for real steps or real parallel items. Use prose otherwise.

Follow each page's existing convention for UI names, and keep it consistent: labels the reader clicks or reads on screen go in `code` style, as the User Guide does now.

No emoji. Link text says where the link goes ("the User Guide"), never "here".

Put the condition before the instruction: "To keep it, click `Save`."

## Structure

The README is the front door. It says who the app is for, what it does, how to install it, how to get a first path and where to read more. It stays short.

The User Guide follows the screen, in the order you meet things. Within a section, say what it is and what you do with it first, then the details, then the edge cases.

Game mechanics are explained once. Link to the Hydra wiki or to the differences page instead of repeating them.

If a page covers only part of something, say so at the start.

## Release notes

A release note describes the app as it was at that release. A rewrite changes the voice and the shape, never the history. It keeps every fact the note states, including its numbers, its song lists and the names of things at the time, even ones that changed later. It adds nothing that the release didn't ship. If a note looks wrong about its own release, leave the sentence and report it.

A note opens with one or two plain sentences saying what the release does. Short bold lead sentences that open a paragraph are fine, because each one is a full sentence and not a label.

## Things not to do

Don't chase an AI-detector score. Those tools are unreliable, and writing for them makes text worse.

Don't drop facts to make text shorter. A rewrite keeps every true fact the old text had, unless it's wrong or said twice.

Don't add a claim you can't point to in the code, the release notes or a measured run.
