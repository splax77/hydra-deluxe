# TOOL-AUDIT-T8-R3 (agent a77a4de09093458db)
Paths: H = C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents ; G = C:\Users\Patrick\.claude ; M = G\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory
Note: I read the T6 TSV rows, then the live files. I did not open the T5 ledger sections or T7 reports.

### L-F14-12
**Verdict** NOT A SPLIT.
**Homes read** H\public-doc-style.md "Words to cut": cuts "It's worth noting", "Note that", "In this section", "Let's", "Simply", "In order to", "To summarize", "In conclusion" (for player pages). G\CLAUDE.md:5: `Cut throat-clearing ("It's worth noting", "In order to", "It's important to note")` (examples, for explanations to the user).
Different readers; the global list is three examples, not a closed list. Nothing says one overrides the other.

### L-F19-21
**Verdict** NOT A SPLIT. Same two homes as L-F14-12. The public page has a longer set for player pages; the global file gives three examples for talking to the user.

### L-F26-13
**Verdict** REAL.
**Homes read** M\comments-explain-current-code.md: "Code comments explain the current code: what a constant or rule means and who reads it. They never tell its history." H\brief-preamble.md:119: "A comment names where a rule lives; it never restates the rule... Comments may say why a rule exists, or what it deliberately leaves out, as long as they don't spell out what it matches." The only Confirmed line (H\rulings\code-and-engine.md:22) is "comments exist to explain the current code, not give a history lesson", which covers the history side only.
**Kind** memory-vs-other.
**What changes** A writer following the note adds a comment saying what a constant or rule means; one following the preamble writes only a pointer to the owner.

### L-F28-13
**Verdict** REAL (weak: an omission, not a contradiction).
**Homes read** H\brief-preamble.md:123 "Lead with the plain version. One idea per sentence. Gloss any jargon in one line right after it. No bullet walls..."; G\model-postures.md:13 "Write plain English: lead with the plain version, one idea per sentence, no bullet walls of file:line references." (no gloss, no pointer to the global file). G\CLAUDE.md "How to explain things" has the gloss rule.
**Kind** doc-vs-doc.
**What changes** An agent or session briefed from the posture file alone would not gloss jargon.

### L-F29-08
**Verdict** REAL (weak). Same two lines as L-F28-13. **Kind** doc-vs-doc. **What changes** the gloss rule is in one copy and missing from the other.

### L-F30-17
**Verdict** NOT A SPLIT.
**Homes read** brief-preamble.md:123; H\doc-review.md:81 "check that jargon is glossed on first use, sentences are short, and no wall of file:line bullets stands without sentences around it" (names G\CLAUDE.md as the one home); G\tools\session_cards\writer-brief.md:94 "short sentences, no jargon without a gloss... keep sentences around them". All three carry the same core points; none contradicts another.

### L-F09-13
**Verdict** REAL.
**Homes read** H\this-machine.md:81 "copy `hydra.db-wal` and `hydra.db-shm` along with `hydra.db`... The bare hydra.db alone can miss the newest writes." H\ui-testing.md:195 "A plain copy of a database another program is writing can catch it halfway through a write, so the helper copies hydra.db with SQLite's online backup instead... whenever a hydra.db-wal sits beside it." hydra-test\tools\compare_db.py:85-87 "through SQLite's backup API... leaves a single-file copy with no -wal".
**Kind** doc-vs-code (this-machine vs the helper and compare_db.py); doc-vs-doc (this-machine vs ui-testing).
**What changes** An agent reading the live library copies three files by hand; the tools use the backup call and make one file.

### L-F13-06
**Verdict** REAL.
**Homes read** H\brief-preamble.md:33 "run the file (`bash <file>`, `python -I <file>`)". H\this-machine.md:7 "Run Python as `py`"; :9 "A bare `python` reaches a different install... C:\Python314\python.exe". G\gates\ruling_rules.py:108 `CHECK_COMMAND = "python.exe -I ...ruling_rules.py --check"`; sync_settings.py:20 `python.exe -I ...`; G\gates\README.md:110,114,145 use the full `...\pythoncore-3.14-64\python.exe` path.
**Kind** doc-vs-doc (preamble vs this-machine); doc-vs-code (this-machine's "not bare python" vs the two scripts' bare python.exe).
**What changes** Bare `python` runs 3.14.7 from C:\Python314; `py` and the full path run the 3.14.5 install the hooks use.

### L-F13-08
**Verdict** REAL.
**Homes read** H\issue-tracker.md:14 "**Close**: `gh issue close <number> --comment "..."`". H\this-machine.md:15 "Text never goes to GitHub through a hand-typed `gh` command. It goes through publish.ps1". G\gates\settings.fragment.json:9-43 denies gh pr create, release create/edit, issue create/comment, pr comment, api (Bash, PowerShell, `& gh`); no `gh issue close`. publish.ps1's -Kind set has no close kind. I searched publish_gate.py and percall.py for close handling: nothing.
**Kind** doc-vs-doc (issue-tracker vs this-machine); doc-vs-code (the deny list and publish.ps1 have no route).
**What changes** Following issue-tracker, an agent types a close comment by hand; following this-machine it must not, and publish.ps1 has no way to do it.

### L-F13-17
**Verdict** REAL for `gh issue close --comment` (same as L-F13-08). NOT A SPLIT for `gh issue edit --add-label/--remove-label` (issue-tracker.md:12-13): that carries no prose, so this-machine.md:15 does not cover it.
**Kind** (close part) doc-vs-doc and doc-vs-code.

### L-F13-23
**Verdict** REAL for the hook-tests half; the pwsh half is NOT A SPLIT (see L-F14-02).
**Homes read** G\gates\README.md:17 "The hook tests on this page need no build, so agents still run them." H\brief-preamble.md:47 "Never run all the hook tests... Run only the tests your brief names."
**Kind** doc-vs-doc. **What changes** An agent following the gates README runs hook tests unprompted; following the preamble it runs only those its brief names.

### L-F14-02
**Verdict** NOT A SPLIT (borderline).
**Homes read** H\this-machine.md:51 "Run scripts with `pwsh`... Windows PowerShell (`powershell.exe`)... a plain call to a script there is refused". G\gates\README.md:152 `powershell -NoProfile -ExecutionPolicy Bypass -File ...`. The README passes Bypass itself, the thing a plain call lacks, so both run. They answer "plain script call" and "test script with an explicit policy flag".

### L-F14-03
**Verdict** REAL. Same split as L-F09-13. ui-testing.md:350 ("copy... while the installed Hydra is closed") agrees with this-machine (closed means no -wal), so it is a context, not a third side.
**Kind** doc-vs-code; doc-vs-doc. **What changes** as L-F09-13.

### L-F15-01
**Verdict** REAL (small).
**Homes read** H\brief-preamble.md:63 "Never `git add -A` or `git add .`". M\git-traps-in-shared-checkout.md:19 "Never `git add -A` or `git add -u` in the shared checkout." I did not search for a hook that checks either.
**Kind** memory-vs-other.
**What changes** `git add -u` is banned by the note and not named by the preamble (though "stage by name" implies it).

### L-F16-08
**Verdict** REAL. Duplicate of L-F13-08: issue-tracker.md:14 `gh issue close --comment` has no deny rule in settings.fragment.json:9-43, while `gh issue comment` and `gh pr comment` are denied. **Kind** doc-vs-doc; doc-vs-code. **What changes** as L-F13-08.

### L-F29-03
**Verdict** REAL. Same two lines as L-F13-23 (gates README.md:17 vs brief-preamble.md:47). **Kind** doc-vs-doc.

Ids not reached: none.
