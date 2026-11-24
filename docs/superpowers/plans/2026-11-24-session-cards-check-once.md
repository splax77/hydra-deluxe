# Session cards: check once, then freeze

Status: a plan, waiting for the user's yes. Nothing here is built. The user said yes to having this plan written, not to building it.

## The short version

Today every run of the card checker judges all 603 cards again, against a world that keeps moving. So a card that was right when it was written can fail months later, and the public index then marks it `[failed]`. That is what happened on the "Hit window" rename: 21 good cards failed because a word in another file changed.

The fix is to judge a card once, when it is written, and then freeze it. A card that passes gets a stamp: one line in a git-tracked file, holding a hash of the card's text (a hash is a short fingerprint that changes if any character changes). After that, a run asks one question about a published card: does it still match its stamp? Nothing outside the card can change that answer.

When the world does change (a commit disappears, a term is renamed, a decision entry is removed, a session grows after its card), the card gets a note in a separate file. A note fails nothing.

## What the user asked for

The user's words, as the brief for this plan quotes them: "the whole design of the tag and card system seems very fragile. just fundamentally broken. do a critical review of the system and tell me a better design that will hold up against this kind of thing". They also called the design "insanely fragile".

## What is wrong today, checked against the code

All the code below is in the hooks repo, `C:\Users\Patrick\.claude\tools\session_cards\`. Each fact has a row in the Premise map.

**Every run re-judges every card.** A run with no card paths checks every published card, every local card and every staged card (`default_cards` in check.py). The start-of-session hook's note tells the main session to run exactly that (P2, P3).

**The judging reads six moving inputs.** For each card, `check_card` reads today's tag list, today's decision log, today's git repos and the session's live transcript through today's transcript reader (P4 to P7). Any of the four can change after the card was written.

**The tag list is thrown away and rebuilt at every session start.** `prepass.run` overwrites `tags.txt` from the bold terms in Hydra's `CONTEXT.md` plus `hand_tags.txt` (P1). A renamed term drops its old tag. That is the "Hit window" failure.

**A local file decides what the public index says.** The checker writes `checked` or `failed` into `manifest.json`, which sits outside git, and `index.py` builds the `[failed]` and `[stale]` marks from it (P8, P9).

**Three scripts edit published cards in place.** They are `check.py --refresh-agents`, `reclass_clicks.py --write` and `number_rulings.py assign` (P10 to P12).

**One thing the brief said does not match the code.** The brief said migrate.py rewrites old cards for the "Rulings" to "The user's words" rename. It does not. Old cards keep the "Rulings" heading and the parser accepts either name (P13). migrate.py was a one-time copy that removed the dropped "Sources" block (P14). The point still stands: the code has no record of which format a card was written in.

**What already exists to build on.** The checker already hashes each passing card and stores it as `card_sha` in the manifest, and `prune_list.py` already reads it to ask "is this card still the one that was checked" (P15, P16). That is the stamp idea, kept in the wrong place (a local file) and never used by the checker itself. The plan moves that one answer into git and makes the checker read it. There is no card format number today; `prepass.FORMAT` is the format of the condensed evidence, not of the card (P17). A staged card already replaces the published card of the same name (P18), so "a fix is a new version that goes through the check" needs no new mechanism.

## The design

### 1. The stamp

A card that passes the full check gets one line appended to a stamp file. The line holds the card's file name, the session's start time, the card format, the checker version and the hash of the card's text.

Public cards' stamps go in `docs/sessions/stamps.tsv` in Hydra, beside the cards, in git. Local cards' stamps go in `stamps.tsv` in the local-cards folder, because a local card's file name carries its session id and must never be public.

The file only gets new lines. When a new version of a card passes, a new line is appended, and the last line for a card is the one that counts.

The hash is taken over the card's text with Windows line ends turned into plain ones. Hydra's `.gitattributes` lets git convert line ends on checkout (P19), and a hash of the raw bytes would then change on a fresh clone with no edit to the card. That would be the same class of bug this plan removes.

One new file, `stamps.py`, owns the stamp: reading the file, hashing a card, writing a line and answering "does this card match its stamp". The manifest's `card_sha` and `common.file_sha` are removed, so the question has one owner (agent rule 15).

### 2. What a run does

A staged card gets the full check, as today. If it passes, it is published and stamped. If it fails, it stays in staging, as today.

A published or local card with a stamp is only compared with its stamp. The tag list, the decision log, git and the transcripts are not opened for it. One check stays: a card in the public folder whose session is kept locally still fails, because that is a privacy rule about where the card sits, not about the card's words (P40).

A published card with no stamp gets the full check, because nobody has judged it yet. This is also how the 603 existing cards get their stamps: the first run after the change finds them all unstamped.

A published card whose text no longer matches its stamp fails, with a message that says it was edited after its check and how to fix it: put the edited text in staging and run the check.

### 3. A new version is judged only on what it adds

When a staged card replaces a stamped card, every item the stamped version already holds is carried over without being judged again. An item is a tag, a D-number, a commit hash, or a line of the user's words. Only items the new version adds are judged.

Without this, fixing a typo in an old card would put the whole card back in front of today's world, and the fragility would come back through the side door.

The old version counts only if it still matches its stamp. If it was edited after its stamp, nothing is carried over and the new version gets the full check. The rules that read only the card's own text (line limit, headings, roadblock shape, catcher) are judged on the whole new version, as today.

### 4. The tag list

The tag list becomes `docs/sessions/tags.txt` in Hydra, in git. It only gets new lines. It has three kinds of line:

```
bre
hooks (hand)
hit-window => timing-step
```

A bare tag comes from a `CONTEXT.md` term. A tag marked `(hand)` was added by hand. A line with `=>` says the tag on the left was replaced by the tag on the right. Lines are read in order, so a later `=>` line can turn a replacement around.

Every tag ever listed stays valid for the cards that carry it. A new card may not use a replaced tag; the checker's message names the tag to use.

A new script, `tags.py`, adds a line for each `CONTEXT.md` term the list lacks. It never removes or changes a line. When a listed term is gone from `CONTEXT.md` and has no `=>` line, it prints one line saying so and stops there. It does not guess what the term was renamed to; a person adds the `=>` line.

`hand_tags.txt`, `build_tags`, `read_tags` and the local `tags.txt` are removed. The list has one home.

**This works whichever way the "Timing step" question goes.** The plan adds `hit-window` back and the line `hit-window => timing-step`, which matches `CONTEXT.md` today (P20). If the term goes back to "Hit window", one more line, `timing-step => hit-window`, makes `hit-window` the current tag again. No card is touched either way. No card carries `timing-step` today (P27).

### 5. Card formats

The checker records in the stamp which format a card matched. Format 1 is the six headings with "Rulings". Format 2 is the six headings with "The user's words". `common.py` gets a small table of formats, and `parse_card` reads its two accepted heading orders from it.

Nothing about which cards are accepted changes: the checker takes either format today (P13) and still will.

The protection is a test with frozen sample cards, one per format, kept under `fixtures\formats\`. Every reader of a card (the parser, the words reader, the index builder) must read each sample. A future format change that breaks an old format turns that test red before it reaches a real card.

migrate.py and its tests are removed. It ran once and has no job left.

### 6. Notes

A new script, `notes.py`, looks at the stamped cards and the world and writes a note for each thing that no longer holds. There are four kinds:

- `stale`: the session's transcript grew after the card was written. Today this is the manifest's `stale` flag (P21).
- `commit-missing`: a commit the card cites is in no repo now.
- `d-entry-missing`: a D-number the card cites has no entry in the decision log now.
- `tag-unlisted`: a tag the card carries is not in the tag list now.

Public cards' notes go in `docs/sessions/notes.tsv`, in git. Local cards' notes go in the local-cards folder. Each note carries the hash of the card version it was written about. A note counts only while that hash is the card's last stamp, so a card that is rewritten and stamped again starts with no notes. Each kind is written once per card version. `notes.py` always exits 0, and no note changes a card, a stamp or a `[failed]` mark.

There is no note for a changed transcript reader. A stamped card is never read against a transcript again, so the reader cannot affect it.

### 7. The index

`index.py` stops reading the manifest. It lists every card file in the cards folder. A card with no matching stamp is marked `[failed]`. A card with a `stale` note is marked `[stale]`. The order and the date on each row come from the start time in the stamp, or from the date in the card's file name when the stamp's start time is empty, as the index does today (P39). `[write-back]` already comes from the card's own text.

One line still depends on this machine: "N sessions are kept locally". Local cards exist only here, so the count is read from the local stamp file. See Out of scope.

### 8. No script edits a published card

`--refresh-agents` is removed from check.py. `reclass_clicks.py --write` and `number_rulings.py assign` write the changed card into staging instead of over the published file. The next check run judges what they added (design 3) and stamps the new version.

### 9. Proof of each quote

When the full check matches a quote to a message, it writes down which message: the message's time and a hash of its text. These records go in a local `proofs` folder, one file per card. They are never public: a hash of a whole message would put a fingerprint of unquoted words in the public repo.

Nothing in the pipeline reads a proof. It is there for a person who wants to trace a quote after the transcript is gone. To find the message, `proofs.py` asks `common.is_users_words` once per typed message, the same call `_holders` already makes (P41). It never matches a quote by a rule of its own, and `common.py` does not change, because the ruling gate asks the same function (P38).

### 10. What stays as it is

Staging, exact typed quotes, chose lines for clicks, the local and public split, the public-content rules in the writer brief, the 60-line limit, the roadblock shape and the manifest lock all stay. The manifest stays too, as the local work queue: `batch.py`, `link_memory.py` and `number_rulings.py` read its `status` to pick sessions (P22 to P24). It no longer decides anything public.

## Prior art

The design follows three known patterns. An append-only log where each entry carries the version it was written under, so old entries stay readable without rewriting them: the Go `ledger` library does this (https://pkg.go.dev/github.com/rbaliyan/ledger). Records addressed by a hash of their content, so a silent change is visible: the `model-ledger` project's snapshots work this way (https://block.github.io/model-ledger/concepts/snapshot/). And the distinction Pat Helland draws in "Immutability Changes Everything" between a schema that describes data as it was written and one that governs current data (https://cacm.acm.org/practice/immutability-changes-everything): a card's format is the first kind. The search found nothing I could cite on frozen-fixture tests, so design 5's test rests on its own argument, not on a source.

## Global constraints

These hold for every task.

- G1. No task edits, moves or re-encodes an existing card. The 21 cards pass without being edited.
- G2. No file under `docs/sessions/` ever holds a local session's id, file name, title or tag.
- G3. Tests use made-up folders only, the way `keep_off_the_real_folders` already does (P25). No agent runs a script on the real card data. Only the main session does, in steps M1 and M2.
- G4. An executor runs only the test files its task names. Nobody runs the whole hook suite.
- G5. A new test goes in the new file its task names. `test_session_cards.py` is edited by one task per wave, so parallel tasks never touch the same file.

## The tasks

All code tasks run in one worktree of the hooks repo, on one branch, with one fresh reviewer at the end. Each is sized for under 100 tool calls and 20 minutes. A Python test file is run as `py -I <worktree>\tools\session_cards\<file>`.

### Wave 1 (three tasks in parallel)

### T1: the stamp owner

Owns `stamps.py` and `test_stamps.py`, both new.

It builds design 1: read a stamp file, hash a card's text, append a line, and answer whether a card matches its last stamp line.

Acceptance criteria:
- A card saved with Windows line ends and the same card saved with plain line ends give the same hash. Test: `test_hash_ignores_line_end_style`.
- Changing one character of a card makes it stop matching its stamp. Test: `test_changed_text_does_not_match`.
- Appending a stamp never changes or removes an earlier line; the file before is a prefix of the file after. Test: `test_append_keeps_every_earlier_line`.
- With two lines for one card, the later one counts. Test: `test_last_line_wins`.
- A local card's stamp is written to the local folder and the public file does not change. Test: `test_local_stamp_stays_local` (G2).

Runs: `test_stamps.py`.

### T2: the tag list owner

Owns `tags.py` and `test_tags.py`, both new.

It builds design 4: read the three kinds of line, answer "is this tag listed" and "is it replaced, and by what", and add missing `CONTEXT.md` terms. It reads terms with the same pattern and slug rule `build_tags` uses today (P1). That code exists in two places only until T10 removes `build_tags`; after that `tags.py` is the one owner.

Acceptance criteria:
- **The rename test.** Start with a list that has `hit-window`. Rename the term in a made-up `CONTEXT.md` to "Timing step". Run the sync. `hit-window` is still listed and `timing-step` is added. Test: `test_renamed_term_keeps_its_old_tag`.
- The sync never removes or changes a line; the file before is a prefix of the file after. Test: `test_sync_only_appends`.
- A listed `CONTEXT.md` tag whose term is gone and that has no `=>` line gets one printed line, and no `=>` line is written. Test: `test_gone_term_is_reported_not_guessed`.
- After `a => b`, `a` is replaced and `b` is current. After a later `b => a`, `a` is current and `b` is replaced. Test: `test_later_pointer_turns_it_around`.
- A `(hand)` tag is never reported as gone. Test: `test_hand_tags_are_not_context_tags`.

Runs: `test_tags.py`.

### T3: card formats and frozen samples

Owns `common.py`, `fixtures\formats\` (new) and `test_formats.py` (new).

It builds design 5: the format table in `common.py`, `parse_card` reading its heading orders from the table, a function that says which format a parsed card is, and one frozen sample card per format.

Acceptance criteria:
- Each sample parses with no errors, its words section is read by `words_lines`, and the format function names its number. Test: `test_every_format_still_reads`, one case per sample.
- A card with both headings, or with headings in another order, still gives the error it gives today. Test: `test_bad_headings_still_refused`.
- The existing `CardTests` still pass unchanged, which shows no card is newly accepted or refused.

Runs: `test_formats.py`, and `py -I -m unittest test_session_cards.CardTests` from the worktree's tools folder.

### Wave 2 (two tasks in parallel, after wave 1)

### T4: the checker checks once

Owns `check.py`, `test_check_once.py` (new) and `test_session_cards.py`.

It builds designs 2 and 8 for check.py. The full check runs for staged cards and unstamped cards. Stamped cards are compared with their stamp through T1; their session id comes from the card's own header, and a stamped card that matches gets manifest status `checked`, so `batch.py`, `link_memory.py` and `number_rulings.py` still see it (P22 to P24). A pass writes a stamp with the format from T3 and a new `CHECK_VERSION` constant. Tags are judged through T2. `--refresh-agents` is removed, with `AgentCountTests`. The manifest's `card_sha` is no longer written. The manifest `status` is still written, for the work queue.

The commit rule for new cards stays as it is today (P26). A commit that later disappears gets a `commit-missing` note, which fails nothing.

Acceptance criteria. Each test first publishes and stamps a passing card, then changes the world, then runs the check with no card paths:
- **Renamed term.** The tag list loses the card's tag entirely. The card passes. Test: `test_stamped_card_survives_a_lost_tag`.
- **Removed D-number.** The decision log loses the card's D entry. The card passes. Test: `test_stamped_card_survives_a_removed_d_entry`.
- **Missing commit.** The repos are replaced by empty ones. The card passes. Test: `test_stamped_card_survives_a_missing_commit`.
- **Changed transcript reader.** `prepass.condense` is patched to raise, and the stored `user_text.json` is deleted. The card passes, and the patched reader was never called. Test: `test_stamped_card_never_reads_a_transcript`.
- Each of the four tests also runs the same world against an unstamped copy of the card and shows it fails. That proves the stamp is what made the difference.
- A stamped card edited by one character fails, and the message names staging. Test: `test_edited_card_fails_its_stamp`.
- A staged card that fails gets no stamp line and is not published. Test: `test_failed_card_gets_no_stamp`.
- A new card with a replaced tag fails, and the message names the tag to use. Test: `test_new_card_may_not_use_a_replaced_tag`.
- A stamped public card whose session is kept locally still fails the "must not sit in the cards root" check. Test: `test_stamped_card_in_public_folder_of_local_session_fails`.
- `check.py --refresh-agents` is refused as an unknown option. Test: `test_refresh_agents_is_gone`.
- A search of `check.py` for `card_sha`, `file_sha`, `read_tags` and `refresh_agents` finds nothing, docstring and comments included. The executor's report quotes the search.

Runs: `test_check_once.py`, and the `CardTests` and `LocalCardTests` classes of `test_session_cards.py`.

### T5: notes

Owns `notes.py` and `test_notes.py`, both new.

It builds design 6. It asks the existing owners: `common.existing_commits` for commits, `check.decision_numbers` for the log, T2 for tags, and the manifest's `stale` flag for growth.

Acceptance criteria:
- Each of the four kinds is written when its cause is set up. Test: one case per kind in `test_notes.py`.
- Running twice writes each note once. Test: `test_a_note_is_written_once`.
- A note written about an earlier version of a card is not counted once the card has a newer stamp line. Test: `test_note_for_an_old_version_is_ignored`.
- A note never changes a card, a stamp or the exit code: after a run with all four causes, the cards and stamp files are byte for byte the same and the exit code is 0. Test: `test_notes_fail_nothing`.
- A local card's note goes to the local folder and the public notes file does not change. Test: `test_local_note_stays_local` (G2).

Runs: `test_notes.py`.

### Wave 3 (four tasks in parallel, after wave 2)

### T4b: a new version is judged only on what it adds

Owns `check.py` and `test_check_carry.py` (new). No other wave 3 task touches check.py.

It builds design 3 on top of T4: read the items of the stamped version, subtract them from the new version's items, and judge only what is left.

Acceptance criteria:
- A staged new version of a stamped card that adds one D-number passes in a world where the card's old tag, old commit and old quote would all fail. Test: `test_new_version_is_judged_on_what_it_adds`.
- A staged new version that adds a quote the user never typed fails; the old version stays published and still matches its stamp. Test: `test_bad_new_version_leaves_the_old_one`.
- When the published version was edited after its stamp, nothing is carried over: the same world that passed above now fails the new version. Test: `test_edited_old_version_carries_nothing_over`.
- A new version that drops an item and keeps the rest passes, and its stamp line is a new last line. Test: `test_new_version_gets_its_own_stamp_line`.

Runs: `test_check_carry.py`, `test_check_once.py`.

### T6: the index and the prune list read stamps

Owns `index.py`, `prune_list.py`, `number_rulings.py` and `test_session_cards.py`.

It builds designs 7 and 8. `index.py` takes its rows from the card files, the stamp file and the notes file. `prune_list.py` asks T1 whether a card matches its stamp, in place of `card_sha`. `number_rulings.py assign` writes its changed card to staging.

Acceptance criteria:
- With the manifest deleted, the index's card lines are the same as with it present. Test: `test_index_needs_no_manifest`.
- A card with no stamp is marked `[failed]`; a card with a `stale` note for its last stamp is marked `[stale]`. Test: `test_index_marks_come_from_stamps_and_notes`.
- Rows keep today's order and dates, including a card whose stamp has no start time. Test: `test_index_order_and_dates_match_today`.
- The index never holds a local card's id or title, and still counts them. The existing `LocalCardTests` index cases, updated to the new inputs.
- A session whose card was edited after its stamp is held back from the prune list. The existing `PruneListTests` case for an edited card, moved from `card_sha` to the stamp.
- `number_rulings.py assign` leaves the published card byte for byte the same and puts the numbered version in staging. Test: `test_numbering_writes_to_staging`.

Runs: the `LocalCardTests`, `PruneListTests` and `NumberRulingsSafetyTests` classes of `test_session_cards.py`.

### T7: the click rewriter writes to staging

Owns `reclass_clicks.py` and `test_reclass_clicks.py`.

Acceptance criteria:
- `--write` leaves every published and local card byte for byte the same and puts each rewritten card in staging. Test: `test_write_goes_to_staging`.
- Without `--write` nothing is written anywhere, as today. The existing dry-run test.

Runs: `test_reclass_clicks.py`.

### T9: the hook, the writer brief and the README

Owns `gates\session_cards_start.ps1`, `gates\tests\test_h53_session_cards_start.ps1`, `tools\session_cards\writer-brief.md` and `gates\README.md`.

The hook itself keeps doing what it does: it runs `prepass.py`, `batch.py` and `prune_list.py`, and it never runs the check (P3). Its note to the main session changes. `tags.py` runs first, before the writers start, so they read an up-to-date list. `notes.py` runs after `check.py` and before `index.py`. The commit step names `tags.txt`, `stamps.tsv` and `notes.tsv` beside the cards and the index.

The writer brief changes in four places. The tag list's path becomes Hydra's `docs/sessions/tags.txt` (its line 28 and step 1 of "How to work"). Rule 3 says a tag on the left of `=>` is replaced and is never used. The paths section says a card that passes is stamped and is then never edited in place. The first pointer to the removed backfill plan stays as it is.

Acceptance criteria:
- The note names the commands in this order: `tags.py`, then the writers, then `check.py`, `notes.py`, `index.py`, `link_memory.py`. The order check in `test_h53_session_cards_start.ps1`, extended.
- The note's commit step names the three new files. A new check in the same test file.
- The hook still exits 0 and prints `{}` when no session is waiting. The existing check.

Runs: `powershell -NoProfile -ExecutionPolicy Bypass -File <worktree>\gates\tests\test_h53_session_cards_start.ps1`.

### Wave 4 (two tasks in parallel, after wave 3)

### T8: proof of each quote

Owns `proofs.py` (new), `test_proofs.py` (new) and `check.py`. No other wave 4 task touches check.py. Its one change there is the call that hands each matched quote to `proofs.py`. It is after T4b because both change check.py.

It builds design 9. The files are local, always.

Acceptance criteria:
- A passing card with two quotes gets a proof naming the right message time for each. Test: `test_proof_names_each_quotes_message`.
- A failing card gets no proof file. Test: `test_failed_card_gets_no_proof`.
- No file under the cards folder changes when a proof is written. Test: `test_proofs_stay_out_of_the_cards_folder`.
- The proof's message is found by calling `common.is_users_words`, not by a matching rule in `proofs.py`. The executor's report quotes the call.

Runs: `test_proofs.py`, `test_check_once.py`.

### T10: remove the old tag list and migrate.py

Owns `prepass.py`, `common.py`, `hand_tags.txt` (removed), `migrate.py` (removed) and `test_session_cards.py`.

`prepass.run` stops writing `tags.txt`. `build_tags`, `read_tags`, `HAND_TAGS` and `file_sha` leave `common.py`. `MigrateTests` and the prepass tag test leave the test file.

Acceptance criteria:
- A prepass run writes no `tags.txt`. Test: `test_prepass_writes_no_tag_list`, in `PrepassTests`.
- A search of the tools folder and the gates folder, leaving out `gates\state` (the agents' status files), for `build_tags`, `read_tags`, `hand_tags`, `file_sha`, `card_sha` and `migrate` finds nothing. The executor's report quotes the search. Every file the search can reach is owned by an earlier task, so a hit left in a file this task does not own is reported, not edited.

Runs: the `PrepassTests` and `StoredCopyTests` classes of `test_session_cards.py`.

### After the code is reviewed and merged: the main session's steps

These touch the real card data, so the main session runs them, not an agent. They are not Clone Hero live runs, so the plan has no live steps.

### M1: seed the tag list and stamp the 603 cards

M1 runs right after the code merge, before any card writer is launched, so no run ever sees the new checker without its tag list.

1. Write `docs/sessions/tags.txt` from the current local `tags.txt` and `hand_tags.txt` (read from git history of the hooks repo, since T10 removed it), with the hand tags marked `(hand)`, then add `hit-window` and `hit-window => timing-step`.
2. Run `check.py` with no card paths. Every card is unstamped, so every card gets the full check once.
3. Run `notes.py`, then `index.py`.

Acceptance criteria:
- check.py prints `cards=603 checked=603 failed=0` (P28, P29).
- `git status` shows no existing card file as changed (G1).
- The new index differs from the old one in exactly one way: the 21 `[failed]` marks are gone (P29, P30).
- The stamp files hold 592 public lines and 11 local lines (P28).

**A card that fails at stamping time** gets no stamp. It stays where it is, unedited, and the index marks it `[failed]`, as a failed card is marked today. The main session does not commit. It stops and tells the user which cards failed and why. Today's run shows no failure other than the 21 tag failures, so the expected number is zero; any other number is a finding, not something to patch around.

4. Commit the three new files and the index on Hydra's main, staged by name. This step uses row P31.

### M2: Hydra's CLAUDE.md

Owns `CLAUDE.md` in Hydra, on its own branch.

The "Past sessions" section gains three plain sentences. The tag list is `docs/sessions/tags.txt`, and a line `old => new` means older cards carry the old tag, so search for both. A `[failed]` mark means the card has no valid stamp, and `[stale]` means the session went on after its card was written. `stamps.tsv` and `notes.tsv` in that folder are the checker's records, not cards.

No page under `docs/agents/` describes the checker. A search of that folder for `session_cards`, `tags.txt`, `check.py` and `[failed]` found only `audit-method.md` and `this-machine.md`, and both only say that cards exist. So none changes.

Acceptance criterion: every label and path in the three sentences matches the merged code, checked by the plan's reviewer.

## Out of scope, for the user to approve

- The "N sessions are kept locally" line in the index is still counted on this machine. Local cards exist nowhere else.
- Nothing stops a person editing `stamps.tsv` or `tags.txt` by hand. The tools only append; git history shows any other edit.
- A session that grows after its card still gets no new card automatically. It gets a `stale` note, as it gets a `[stale]` mark today.
- Whether the `CONTEXT.md` term stays "Timing step". The plan works either way.
- Which commits a new card may cite. The rule stays as today: any commit found in either repo when the card is checked (P26). With stamps, a commit that later disappears can no longer fail a card, so nothing needs the stricter rule of "only commits on main or pushed". That stricter rule would drop the commits of unmerged branches from cards for good, because a card is written about two hours after its session ends. It is a change nobody has asked for, so it is not built.
- Proofs of quotes stay on this machine. Losing the card data loses them. Nothing reads them.

## Questions

None. Each choice the plan makes is a read of what the code does today, or follows from the user's words quoted above.

## Promise map

| Promise | Plan line | Task | Check that proves it |
|---|---|---|---|
| A stamped card cannot fail because the tag list changed | Design 2 | T4 | `test_stamped_card_survives_a_lost_tag`, with its unstamped twin failing |
| A stamped card cannot fail because a D entry was removed | Design 2 | T4 | `test_stamped_card_survives_a_removed_d_entry`, with its twin |
| A stamped card cannot fail because a commit is gone | Design 2 | T4 | `test_stamped_card_survives_a_missing_commit`, with its twin |
| A stamped card is never read against a transcript again | Design 2, 6 | T4 | `test_stamped_card_never_reads_a_transcript` |
| A renamed `CONTEXT.md` term keeps its old tag | Design 4 | T2 | `test_renamed_term_keeps_its_old_tag` |
| The tag list only gets new lines from the tool | Design 4 | T2 | `test_sync_only_appends` |
| The tool never guesses a rename | Design 4 | T2 | `test_gone_term_is_reported_not_guessed` |
| A later `=>` line turns a replacement around, so the plan works either way | Design 4 | T2 | `test_later_pointer_turns_it_around` |
| A new card may not use a replaced tag, and is told which to use | Design 4 | T4 | `test_new_card_may_not_use_a_replaced_tag` |
| The stamp file only gets new lines; the last line counts | Design 1 | T1 | `test_append_keeps_every_earlier_line`, `test_last_line_wins` |
| A line-end change is not a card change | Design 1 | T1 | `test_hash_ignores_line_end_style` |
| An edited card fails its stamp and is told to go through staging | Design 2 | T1, T4 | `test_changed_text_does_not_match`, `test_edited_card_fails_its_stamp` |
| A card that fails gets no stamp | Design 2 | T4 | `test_failed_card_gets_no_stamp` |
| A new version is judged only on what it adds | Design 3 | T4b | `test_new_version_is_judged_on_what_it_adds` |
| A bad new version leaves the old one published and stamped | Design 3 | T4b | `test_bad_new_version_leaves_the_old_one` |
| An old version edited after its stamp passes nothing on to the new one | Design 3 | T4b | `test_edited_old_version_carries_nothing_over` |
| A stamped card still fails when it sits in the public folder of a local session | Design 2 | T4 | `test_stamped_card_in_public_folder_of_local_session_fails` |
| A note about an old card version does not mark the new one | Design 6 | T5 | `test_note_for_an_old_version_is_ignored` |
| The index keeps today's order and dates | Design 7 | T6 | `test_index_order_and_dates_match_today` |
| Old formats stay readable by every reader | Design 5 | T3 | `test_every_format_still_reads` |
| No card is newly accepted or refused by the format table | Design 5 | T3 | `test_bad_headings_still_refused`, `CardTests` unchanged |
| Each kind of note is written, once | Design 6 | T5 | the four kind cases, `test_a_note_is_written_once` |
| A note fails nothing and changes no card or stamp | Design 6 | T5 | `test_notes_fail_nothing` |
| The index is decided by files in git, not the manifest | Design 7 | T6 | `test_index_needs_no_manifest`, `test_index_marks_come_from_stamps_and_notes` |
| The prune list asks the stamp owner | Design 1, 7 | T6 | the edited-card case of `PruneListTests` |
| No script edits a published card | Design 8 | T4, T6, T7 | `test_refresh_agents_is_gone`, `test_numbering_writes_to_staging`, `test_write_goes_to_staging` |
| A passing card's quotes each get a proof; a failing card gets none | Design 9 | T8 | `test_proof_names_each_quotes_message`, `test_failed_card_gets_no_proof` |
| Proofs stay out of the cards folder | Design 9 | T8 | `test_proofs_stay_out_of_the_cards_folder` |
| The proof finds its message through the one quote-matching owner | Design 9 | T8 | the quoted call to `common.is_users_words` |
| Nothing public holds a local session's details (G2) | Global constraints | T1, T5, T6 | `test_local_stamp_stays_local`, `test_local_note_stays_local`, the `LocalCardTests` index cases |
| The hook's note runs the commands in the new order and commits the new files | T9 | T9 | the order check and the commit-step check in `test_h53_session_cards_start.ps1` |
| The hook still exits 0 with `{}` when nothing waits | T9 | T9 | the existing check in `test_h53_session_cards_start.ps1` |
| The old tag list and migrate.py are gone, with one owner left | Design 4, 5 | T10 | `test_prepass_writes_no_tag_list`, the quoted search |
| The 21 cards pass without being edited (G1) | Global constraints | M1 | `failed=0`, and `git status` shows no card changed |
| All 603 cards are stamped once | M1 | M1 | `cards=603 checked=603 failed=0`; 592 public and 11 local stamp lines |
| A card that fails at stamping gets no stamp and stops the commit | M1 | T4, M1 | `test_failed_card_gets_no_stamp`; M1's stop rule |
| No leftover name of a removed owner in `check.py` | Design 1, 8 | T4 | the quoted search of `check.py` |
| The index changes only by losing the 21 `[failed]` marks | M1 | M1 | the diff of the index before and after |
| Tests never touch the real card data (G3) | Global constraints | every code task | each new test file builds on `Base` or calls `keep_off_the_real_folders`; the reviewer reads each `setUp` |
| CLAUDE.md's new sentences match the code | M2 | M2 | the reviewer's label-and-path check |

## Premise map

Paths of hooks-repo files are absolute. Paths of Hydra files are relative to this repo.

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: Every prepass run overwrites `tags.txt` from `build_tags`, which reads the bold `CONTEXT.md` terms plus `hand_tags.txt`. | code | `C:\Users\Patrick\.claude\tools\session_cards\prepass.py:517-519`, `C:\Users\Patrick\.claude\tools\session_cards\common.py:735-753` | checked 2026-11-24 |
| P2: A check run with no card paths judges every published, local and staged card. | code | `C:\Users\Patrick\.claude\tools\session_cards\check.py:257-270`, `C:\Users\Patrick\.claude\tools\session_cards\check.py:361` | checked 2026-11-24 |
| P3: The start hook runs prepass, batch and prune_list itself, and only tells the main session to run check.py, with no card paths. | code | `C:\Users\Patrick\.claude\gates\session_cards_start.ps1:326-352`, `C:\Users\Patrick\.claude\gates\session_cards_start.ps1:280-283` | checked 2026-11-24 |
| P4: The full check fails a card whose tag is not in today's tag list. | code | `C:\Users\Patrick\.claude\tools\session_cards\check.py:180-182`, `C:\Users\Patrick\.claude\tools\session_cards\check.py:315` | checked 2026-11-24 |
| P5: The full check fails a card whose D-number has no entry in today's decision log. | code | `C:\Users\Patrick\.claude\tools\session_cards\check.py:68-73`, `C:\Users\Patrick\.claude\tools\session_cards\check.py:215-217` | checked 2026-11-24 |
| P6: The full check fails a card whose commit hash is in no repo today. | code | `C:\Users\Patrick\.claude\tools\session_cards\check.py:218-228` | checked 2026-11-24 |
| P7: The full check reads the user's words from the live transcript through `prepass.condense` while the transcript exists, and from the stored copy only after. | code | `C:\Users\Patrick\.claude\tools\session_cards\check.py:76-104` | checked 2026-11-24 |
| P8: The checker writes `checked` or `failed` into the manifest, which lives in the data folder outside any repo. | code | `C:\Users\Patrick\.claude\tools\session_cards\check.py:338-339`, `C:\Users\Patrick\.claude\tools\session_cards\common.py:34`, `C:\Users\Patrick\.claude\tools\session_cards\common.py:156-157` | checked 2026-11-24 |
| P9: The index builds its rows, its `[failed]` and `[stale]` marks and its order from the manifest. | code | `C:\Users\Patrick\.claude\tools\session_cards\index.py:32-60` | checked 2026-11-24 |
| P10: `--refresh-agents` rewrites a card's file in place. | code | `C:\Users\Patrick\.claude\tools\session_cards\check.py:120-138`, `C:\Users\Patrick\.claude\tools\session_cards\check.py:317-319` | checked 2026-11-24 |
| P11: `reclass_clicks.py --write` rewrites published and local cards in place. | code | `C:\Users\Patrick\.claude\tools\session_cards\reclass_clicks.py:150-155` | checked 2026-11-24 |
| P12: `number_rulings.py assign` rewrites a published card in place. | code | `C:\Users\Patrick\.claude\tools\session_cards\number_rulings.py:116-137` | checked 2026-11-24 |
| P13: Old cards keep the "Rulings" heading, are not rewritten, and the parser accepts either heading but not both. | code | `C:\Users\Patrick\.claude\tools\session_cards\common.py:51-57`, `C:\Users\Patrick\.claude\tools\session_cards\common.py:780-791` | checked 2026-11-24 |
| P14: migrate.py is a one-time copy that removes the Sources block and never overwrites a card. | code | `C:\Users\Patrick\.claude\tools\session_cards\migrate.py:1-17`, `C:\Users\Patrick\.claude\tools\session_cards\migrate.py:30-65` | checked 2026-11-24 |
| P15: The checker already stores a hash of each passing card as `card_sha` in the manifest, a SHA-1 of the raw bytes. | code | `C:\Users\Patrick\.claude\tools\session_cards\check.py:340-344`, `C:\Users\Patrick\.claude\tools\session_cards\common.py:302-304` | checked 2026-11-24 |
| P16: `prune_list.py` treats a card whose bytes differ from `card_sha` as unchecked. | code | `C:\Users\Patrick\.claude\tools\session_cards\prune_list.py:93-105` | checked 2026-11-24 |
| P17: `prepass.FORMAT` is the condensed evidence's format; no constant numbers the card's format. | code | `C:\Users\Patrick\.claude\tools\session_cards\prepass.py:36-39`, `C:\Users\Patrick\.claude\tools\session_cards\common.py:48-59` | checked 2026-11-24 |
| P18: A staged card replaces the published or local card of the same name in a run, and a passing staged card is moved over it. | code | `C:\Users\Patrick\.claude\tools\session_cards\check.py:265-270`, `C:\Users\Patrick\.claude\tools\session_cards\check.py:286-304` | checked 2026-11-24 |
| P19: Hydra's `.gitattributes` sets `* text=auto`, so git may convert a card's line ends. | code | `.gitattributes:1` | checked 2026-11-24 |
| P20: `CONTEXT.md` names the term "Timing step" today. | code | `CONTEXT.md:676` | checked 2026-11-24 |
| P21: prepass sets the manifest's `stale` flag when a carded session's transcript changed. | code | `C:\Users\Patrick\.claude\tools\session_cards\prepass.py:465-471` | checked 2026-11-24 |
| P22: `batch.py` picks sessions by the manifest's `status`. | code | `C:\Users\Patrick\.claude\tools\session_cards\batch.py:49-60` | checked 2026-11-24 |
| P23: `link_memory.py` reads only sessions whose manifest status is `checked`. | code | `C:\Users\Patrick\.claude\tools\session_cards\link_memory.py:37-42` | checked 2026-11-24 |
| P24: `number_rulings.py` reads the manifest's `status` to pick cards and to refuse when any failed. | code | `C:\Users\Patrick\.claude\tools\session_cards\number_rulings.py:38-42`, `C:\Users\Patrick\.claude\tools\session_cards\number_rulings.py:69-73` | checked 2026-11-24 |
| P25: The card tests point every default folder into a temp folder. | code | `C:\Users\Patrick\.claude\tools\session_cards\test_session_cards.py:42-70` | checked 2026-11-24 |
| P26: A commit counts as existing when `git cat-file` finds the object in a repo, whether or not any branch reaches it; the metadata's commit list uses the same test. | code | `C:\Users\Patrick\.claude\tools\session_cards\common.py:706-724`, `C:\Users\Patrick\.claude\tools\session_cards\prepass.py:484-486` | checked 2026-11-24 |
| P27: No published card carries the tag `timing-step`. | unchecked | `docs/sessions/` (a search of every card file found 0 files holding the text; a search does not read the lines) | M1 |
| P28: There are 603 cards: 592 published and 11 local. | unchecked | `docs/sessions/` (a count of 592 card files beside `index.md`, and 11 files in the local-cards folder, from a directory listing) | M1 |
| P29: Today's check fails exactly 21 cards, all for the tag `hit-window`, and passes the other 582. | unchecked | `brief for task cards-check-once-plan` (the main session's run; this plan's author saw 21 index lines with `hit-window` and 21 `[failed]` marks but did not rerun the check) | M1 |
| P30: The index holds 216 `[stale]` marks, all from the manifest's flag. | unchecked | `docs/sessions/index.md` (a count from a search) | M1 |
| P31: A commit on Hydra's main that changes only files under `docs/sessions/` needs no code review and no document review. | code | `C:\Users\Patrick\.claude\gates\review_rules.py:22`, `C:\Users\Patrick\.claude\gates\review_rules.py:168-186` (Hydra's marker `.derive-once-gate:1-4` has no `gate:` line, so only the built-in code paths count, and `docs/` is not among them), `C:\Users\Patrick\.claude\gates\review_rules.py:24`, `C:\Users\Patrick\.claude\gates\review_rules.py:564-567` (only the four public pages and the document marker need a document review; `docs/sessions/` holds none). The hook file the marker names is not in the hooks repo's main checkout, so `review_rules.py` is the live rule. | checked 2026-11-24 |
| P32: The hook reads the writer brief from the tools folder, so the copy in the data folder is not the one writers get. | code | `C:\Users\Patrick\.claude\gates\session_cards_start.ps1:272-278` | checked 2026-11-24 |
| P33: The writer brief sends writers to `DATA\tags.txt` and tells them tags come only from it. | code | `C:\Users\Patrick\.claude\tools\session_cards\writer-brief.md:28`, `C:\Users\Patrick\.claude\tools\session_cards\writer-brief.md:77`, `C:\Users\Patrick\.claude\tools\session_cards\writer-brief.md:98` | checked 2026-11-24 |
| P34: Hydra's CLAUDE.md "Past sessions" section says nothing about the tag list, the marks' meaning or the checker's files. | code | `CLAUDE.md:23-29` | checked 2026-11-24 |
| P35: The user asked for a design that holds up, in the words quoted under "What the user asked for". | ruling | `brief for task cards-check-once-plan` (the main session's quote of the user's message) | checked 2026-11-24 |
| P36: Each message in the evidence carries its time, so a proof can name the message. | code | `C:\Users\Patrick\.claude\tools\session_cards\common.py:596-606` | checked 2026-11-24 |
| P37: `hand_tags.txt` is a file of 15 hand-added tags under a comment line, tracked in the hooks repo, so M1 can read it from history after T10 removes it. | code | `C:\Users\Patrick\.claude\tools\session_cards\hand_tags.txt:1-16` (a `git ls-files` run showed it tracked) | checked 2026-11-24 |
| P38: The ruling gate also asks `is_users_words` whether a quote is the user's typed words, so its answer must keep one owner. | code | `C:\Users\Patrick\.claude\gates\ruling_rules.py:226-231` | checked 2026-11-24 |
| P39: The index sorts its rows by the manifest's start time, then session id, and shows the date from the card's file name when there is no start time. | code | `C:\Users\Patrick\.claude\tools\session_cards\index.py:52-60` | checked 2026-11-24 |
| P40: A published card whose session is local fails check.py's "must not sit in the cards root" test, which asks `keeps_local` (the card's own header and the local-cards folder). | code | `C:\Users\Patrick\.claude\tools\session_cards\check.py:278-284`, `C:\Users\Patrick\.claude\tools\session_cards\check.py:323-326` | checked 2026-11-24 |
| P41: `common._holders` already finds the messages that hold a quote by calling `is_users_words` on each message alone. | code | `C:\Users\Patrick\.claude\tools\session_cards\common.py:568-570` | checked 2026-11-24 |
