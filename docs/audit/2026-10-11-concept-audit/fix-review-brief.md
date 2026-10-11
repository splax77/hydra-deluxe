# Reviewer brief for the concept audit's fix wave (phase 6)

Every fix-wave reviewer reads `docs/agents/brief-preamble.md` first, then main's `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\derive-once-review.md` (main's copy has kind F and step 8; a worktree's copy may be older), then this page, then its dispatch. You are the one fresh reviewer of one task's change (D107): you fix what you find yourself, commit, and sign off. Nothing goes back to the author and there is no second round.

## Where things are

You work in the task's worktree, on its branch, which your dispatch names. Commit there only; never touch main, never push, never merge or rebase. The branch forked from `origin/main` (`0997bbac`), which is older than local main; nothing under `src/` or `tests/` differed at the fork. Read the audit's documents from the main checkout by absolute path:

- The task's row and section: `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\audit\2026-10-11-concept-audit\fix-wave.md`.
- The findings it carries: `...\2026-10-11-concept-audit\findings.md` (one block per id).
- The decisions: D114 in `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\audit\2026-10-03-fix-decisions.md`. A displayed or stored change must match its item word for word.
- The executor brief it worked under: `...\2026-10-11-concept-audit\fix-brief.md`.

## What to check, beyond the review page's own steps

Each finding's fix leaves one owner, and the other homes call it. Grep for the inputs each fix reads, not only for the old names, and check nothing else still answers the question. A scan row the task added should flag the removed home if it came back, and match nothing it shouldn't; run `hydra_tests.exe -tc="single-owner*"` after any row change. Where a commit says no text search can catch the copy, judge whether that holds. Where the task changed something the user sees, confirm the D114 item it rests on, and that the tests pinning the old text changed with it.

## Building and testing

A build tree already exists in the worktree. Rebuild with `.\build_cpp.ps1 -Target hydra_tests` (and `-Target hydra_uitest` separately if your dispatch names uitest scripts; the script takes one target per call). Run only the filters and uitest names in the task's row. Never the whole suite, never `--all`, never anything over the song library.

## Sign-off

Your review file needs the line "Scope: full". Commit your fixes on the branch first, then submit with `& "C:/Users/Patrick/.claude/hooks/derive_once_submit.ps1" <key> "<review file>"`, using the key your dispatch gives, as the review page says. Keep the review file in your scratch folder. At either time or call warning, stop fixing, sign off what you fixed and list the rest in your Notes.

## Your final reply

Verdict, your fix commit hashes, the test commands you ran with pass counts, and your Notes. Under 20 lines.
