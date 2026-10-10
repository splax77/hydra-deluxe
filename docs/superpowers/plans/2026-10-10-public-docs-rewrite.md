# Public docs rewrite (2026-10-10)

## What this does

Every page a player reads gets brought up to date with Hydra Deluxe 2.5.0 and rewritten in a plainer, more human voice. That means the README, the docs index, the User Guide, the "differences from the public Hydra" page, and the GitHub release notes for all 35 releases (v1.4.1 to v2.5.0). The voice rules live in `docs/agents/public-doc-style.md`, saved with this plan. In short: write to "you", no first person, no AI-writing tells, and check every label and number against the code.

The release notes are history. Each one is rewritten for voice and shape only, and keeps saying what that release shipped, even where the app has changed since.

## Where the findings come from

Four read-only scouts ran on 2026-10-10. Two searched the web for what makes writing read as machine-made and how editors fix it. Their results are now the style page. Two compared the public pages with the code at `acfef6a4` (v2.5.0) and with the release notes. The main session re-checked the biggest claims in the code before writing this plan: no code writes to `Documents` any more, the repo is public, `batch_worker_count` caps a batch at 8 workers, only `start_dm_report` opens the comparison window, and the library's second line under a title shows only during a word search (`two_lines` in `src/ui/library_table.cpp:330`). Every other finding below is a scout's claim. The writer checks it in the code before acting on it (rule 9).

The current pages already avoid the usual buzzwords. Their main tells are 29 bullets that open with a bold label (all in the User Guide), some long paragraphs that put edge cases ahead of the main point, and two headings in a row in the User Guide that both read "The path report".

## The work

Six writers (executors, on Opus) work at the same time. The four page writers each work in their own worktree, branched from `main` once this plan and the style page are committed. The release-note writers touch no repo files. They write into the session scratchpad. When a writer finishes, one fresh reviewer (Sonnet) reads that writer's output against the code and the style page, fixes what it finds, and signs off. Then the main session joins and publishes. Nothing goes back to a writer, and there is no second round (D107).

Every writer reads `docs/agents/brief-preamble.md` first, then `docs/agents/public-doc-style.md`. None of them builds or runs tests: these are documents, and the code is read, never changed. Each writer keeps every true fact the old text has, fixes the wrong ones, and adds the missing ones listed for it. When it can't confirm a claim in the code, it leaves the old text and reports it.

### Task D1: README, docs index and differences page

This writer owns `README.md`, `docs/README.md` and `docs/differences-from-public-hydra.md`.

The README has three wrong lines. It says Hydra builds a "sortable HTML report"; since 2.4.0 the path report opens in its own window. It says reports are saved in `Documents\Hydra`, which is no longer true; the guide's "Old report files" section covers the leftovers. And "Upgrading keeps everything" needs to say that a release which changes the analysis marks saved results `Stale` until the song is analyzed again (2.1.0 and 2.4.0 did this).

The README also misses everything since 2.2.0. In two or three sentences it should mention the `Analysis settings` button and its panel, Note Shuffle, the search filters (`artist:`, `charter:` and the rest), the Preview's star counter and next-activation notation, and `hydra_rules.ini` as the place for judgment-call rules. It gets one requirements line: 64-bit Windows, and an installer that asks for admin rights (`installer/hydra.iss` lines 46 to 48). The scout found no minimum Windows version in the code, so the line names none. The README keeps its getting-started steps, which the scout checked against the UI labels, and every link it has now.

On the differences page, "This version is private" (line 100) is wrong, because the repo is public. Say instead what the public Hydra has that this one doesn't, without inventing a wiki or community. The list of settings a saved result remembers (line 38) misses Note Shuffle and the 1.0 fills rule. Note Shuffle belongs in "Things only this version can do", with the facts from the 2.4.0 notes. The page should also mention the `Analysis settings` button with `Reset to defaults`, the 2x-kick merge and ghost-plus-accent rule (D105) under the chart rules matched to Clone Hero, the Preview's star counter and notation line, and the `All` chip beside not analyzed, stale and analyzed.

`docs/README.md` is current. It gets a voice pass only if it needs one.

### Tasks D2 and D3: the User Guide, in two halves

The guide is about 7,900 words, too much for one writer's 20 minutes, so it splits at the `## Analyzing the whole library` heading. D2 owns everything above that heading. D3 owns that heading and everything below it. Each writer edits only its own half of `docs/UserGuide.md`, so the two branches merge without touching the same lines. Neither renames a heading that the other half, the README or the differences page links to (`#the-first-start-after-updating`, `#moving-from-the-public-version` and any `#...` link inside the guide) unless it updates those links too and says so in its report.

D2 fixes one wrong sentence. The folder shows under the title only while a word search is on and the Folder or Charter column is hidden. When it shows, it is either the folder or "charted by <charter>", and matched text is highlighted in gold. D2 also adds these missing points. Removing a folder and scanning deletes that folder's saved results, so adding it back brings the songs back as `Not analyzed`. The Song folders window has `Close` and, after a change, `Scan now`. `Scan library` and `Analyze library...` grey out during a batch. `Compare with dmleaderboards...` greys out outside Expert, SP cap 4, 1.0 fills off and Note Shuffle off, and its hover says why. The defaults of difficulty, Pro Drums, 2x Bass, Note Shuffle and score range go into a short table with the other settings' defaults. The best all-0 path section shows only when that path isn't already listed, and it can hold several tied paths. The Paths tab's `Multiplier squeeze`, `Score breakdown` and `Backend timings` are folds that start open. The guide should also say what the Best path cell shows for a result with no kept paths, and cover the panel's failed-analysis state with its `Continue` button. On the Preview, D2 adds the sound-device warning ("Hydra couldn't use your audio device; the preview is muted.") and the other states: loading, failed, and no render device. It also fixes the order of the Next activation box (number, then notation, then where and which chord). On the Stars tab, the `With full solo bonus` column and the solo note show only when the chart has a solo. The 29 bold-label bullets in this half become paragraphs, plain lists or tables.

D3 fixes two wrong sentences. A batch uses all but one CPU core, up to 8. And closing the dmleaderboards comparison doesn't keep it to reopen: to see it again you pick the player again, which fetches the scores again. It also tightens one: a batch skips songs with a current result, so `Stale` songs are re-run without ticking the box. D3 renames the first of the two "The path report" headings to "Report windows", since it introduces both. It adds a table of the path report's columns, explains the timing dropdown's entries (`All timing tiers`, `No squeezes`, `Beyond N ms`) and names all five tiles. For the comparison, it adds a table of its columns, the `No paths` status, and the `Avg % of optimal` and `Points left on table` tiles. It picks one set of status names, using the dropdown's wording because that's what the player picks from, and uses it everywhere. It also covers the right-click menu that hides a column, the "no songs could be analyzed" message after a batch, comments after a value in `hydra_rules.ini`, and the main window's place being remembered in `hydra_ui.ini`.

### Tasks N1, N2 and N3: the release notes

The current text of every release note and title is backed up in the session scratchpad under `docs-rewrite/release-notes-original/`. That copy is how any note can be put back exactly. N1 rewrites v2.0.0 to v2.5.0 (6 notes, about 3,650 words). N2 rewrites v1.8.0 to v1.9.7 (13 notes, about 2,870 words). N3 rewrites v1.4.1 to v1.7.10 (16 notes, about 3,380 words). The word counts come from `gh release view` on 2026-10-10.

Each writer writes one file per release, `docs-rewrite/release-notes-new/<tag>.md`, and never runs `gh release edit` itself. The release section of the style page applies: same facts, same history, new voice. A link to the repo's old owner (`patseguin1-commits`) still redirects, so it stays; the 2.0.0 note says the repo moved there, and that is history. Titles stay as they are. For each file, the writer computes the doc-review key with `doc_review_key.ps1` and declares its sources with `doc_review_sources.ps1`: the original note, and any code or decision it checked a fact against. That makes the writer the note's author, so a fresh reviewer can sign it off.

### Reviews

There is one fresh Sonnet reviewer per task, six in all. Each one starts as soon as its writer reports. It works from `docs/agents/doc-review.md` and checks the shapes listed on the style page, not whether the text "reads like AI". It fixes its own findings, commits them (for D1 to D3) or saves them (for N1 to N3), and signs off on the final key. A reviewer that runs out of time signs off what it fixed and lists the rest under Notes. The main session passes those Notes to the user and doesn't hold the publish for them.

## Join and publish

After the D-reviews sign off, the main session merges D2, D3 and D1 into a join branch. It checks that every `#anchor` link in the four pages matches a heading, and that the guide has no duplicate headings. It renders the four pages through GitHub's markdown API (`gh api markdown`), which shows the result without publishing anything, and looks at them in the browser pane. Then it merges to `main`, pushes, and reports done (rule 10).

For the release notes, the main session posts each signed-off file with `gh release edit <tag> --notes-file <file>`, one release at a time, newest first. The user's yes to this plan is the permission for those 35 edits. If a post is refused or fails, the main session stops posting and reports which tags went out. Every tag can be put back from the backup.

## Out of scope

This plan changes no code, so nothing gets built, tested or released. The comparison window has no way to reopen without fetching again; the guide will describe that as it is. Adding a reopen button would be a separate code change for the user to decide on.
