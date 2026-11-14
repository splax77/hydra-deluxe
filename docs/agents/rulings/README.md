# The user's rulings, by topic

This folder holds the user's rulings, one topic file each. A ruling is filed here only after the user clicks "File it" for that exact line in a question box. Open only the topic file your task needs; nothing here loads on its own.

## Topic files

- agents-and-workflow.md: status lines, parallel work, agent counts, approvals before work, and how far an agent's or a handoff's word is trusted.
- review-and-hooks.md: the one-reviewer rule, the review and doc gates, and what the hooks may and may not do.
- ui-and-display.md: layouts, defaults and on-screen wording in Hydra, the Path Builder answers, and how a UI change is checked.
- code-and-engine.md: derive once, one Star Power end, scoring rules, stored data, error lines and comments.
- clone-hero.md: where Hydra follows the game, and how live runs of the game must work.
- questions-and-audits.md: how a question to the user is written, splits, and when an audit finding may close.
- docs-and-writing.md: plain English, docs that never cite scratch folders, and searching for prior art before a plan.
- tests-and-runs.md: which tests run at a merge, small runs before big ones, and the whole-library rule.

## Confirmed and Unconfirmed

Each topic file has two headings. `## Confirmed` holds lines the user approved. Such a line is the user's words, and a session or reviewer may cite it for what it literally says, and for nothing more.

`## Unconfirmed` holds the lines sessions copied from memory notes on 2026-10-31, untouched. They are not the user's rulings. Sessions keep following them as working rules. No plan, review or question may cite one as the user's ruling. A session that needs the user's words for one looks for the quote in a session card under `docs/sessions/`, or asks the user.

A ruling that uses a project word does not change what the word means. `CONTEXT.md` decides that.

## How a Confirmed line reads

A Confirmed line has one of two shapes and nothing else on it.

The first holds words the user typed: `- **2026-11-13** "change the display text to frontend and backend" (session 7d28ae9a)`.

The second holds a label the user clicked in a question box, with the whole question: `- **2026-11-13** chose "Keep frontend/backend" to "Keep the words?" (session 7d28ae9a)`. A clicked label means nothing without its question, so the question is always there.

One line holds one quote, copied letter for letter, typos included. The session is the first eight characters of its id, or the whole id. The line holds no reading, no context, no "So ...", no "As the note records it:", no note name and no pointer to the page it backs.

A session's reading of a ruling lives in the plan or page that applies it, never here. It starts with "Reading (the session's, not the user's):". It names the `CONTEXT.md` term for each project word it uses. If the reading and `CONTEXT.md` disagree, `CONTEXT.md` wins and the session asks the user.

## How to file a ruling: the approval step

Only the user files a ruling, by clicking in a question box. A session that wants to file one does these four things in order.

1. It writes the exact line into a scratch file and runs the by-hand check, `ruling_rules.py --check <file>` (in the hooks repo's `gates` folder). The check says whether the line has the right shape and whether its quoted words are in what the user typed in the session it names. It prints the question text to use.
2. It asks the user with the question box (the AskUserQuestion tool). The question reads `File this ruling in docs/agents/rulings/<topic file>?`, followed by the whole line. The two labels are `File it` and `Do not file`. One box may hold up to four lines, one question each.
3. Only on `File it` does it add the line under `## Confirmed` in that file, and commit.
4. Git's gate checks the commit. It refuses any added line that is not in the right shape, whose quote the user never typed, or that has no `File it` answer for that line and that file in a main session's transcript. An agent's transcript never counts.

A ruling counts only once it is committed. Removing a line is always allowed.

No agent can approve a line, because the gate reads only main-session transcripts. An agent that hears the user's words puts them in its report, and the main session decides whether to ask the user.

## The backfill route for old lines

An old Unconfirmed line can be replaced by the user's original words without a click, through one narrow route. The user approved it on 2026-11-13 ("Yes, no click"). Scouts search the transcripts for the words behind the old line. A new line built from what they find passes the gate only when three things hold. Its quote passes the words test. Its date is before 2026-11-14, the day the gate was installed. The same commit deletes an old line under `## Unconfirmed` in the same file. Any line dated on or after the install day needs the click.

## How an Unconfirmed line reads

Each old line starts with the date of the ruling in bold, then the user's words in quotes. Words a note states without quoting the user follow "As the note records it:" and are copied as the note words them. In brackets come the session, the note and the rule the ruling backs. Only the note's markdown was dropped: backticks, bold marks and line breaks inside a quote.

The session is the one the note names for that ruling. Where the note names none, "by date" marks the one session in the note's Sessions line that falls on the ruling's date, and "the note's only one" marks a note with a single session. Otherwise the session says "not stated", and nothing is guessed. A "card" id is a session card the note itself cites. The note is the memory file's name; "archive/" means it sat in the memory archive. "Backs" names the CLAUDE.md rule or docs/agents page that carries the ruling today, or says "no page yet".
