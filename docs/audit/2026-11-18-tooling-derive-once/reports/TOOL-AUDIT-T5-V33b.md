# TOOL-AUDIT-T5-V33b (verifier of F33b). All claims "by reading"; nothing run.

Verdicts for the 12 rows. G = C:\Users\Patrick\.claude\gates, H = C:\Users\Patrick\Downloads\Hydra\hydra-test, SC = C:\Users\Patrick\.claude\tools\session_cards.

## I7-096 CONFIRMED (K2 split)
- H\tools\ch_mcp\README.md:83: "copies ... only for a song `ch_start` was given a folder for, which the bot plays from the command line. When a person plays the song instead, the game's own ... scorestats.json still holds that run after `ch_stop`".
- H\docs\agents\clone-hero-mcp.md:60 (one line, found by grep) says the copy is for a command-line song "whether the bot or a human player (human=True) plays it".
- Code: H\tools\ch_mcp\bot_run.py:755-761 sets cap_s whenever song_dir is given; :799 refuses only when run.cap_s is None. Line 752 human_refusal(human, song_dir) implies a human launch carries a folder. So code agrees with clone-hero-mcp.md; the README sentence reads as if a human run is never copied. Situation: human=True with a song folder.
- Split lines: (a) README:83 vs clone-hero-mcp.md:60 on whether a human-played folder launch is copied; (b) path literal with the user name appears at clone-hero-mcp.md:15 and :60 while game_files.py derives it from _HOME (K1).
- Ruling: docs\agents\rulings\clone-hero.md:9 is under `## Confirmed` (heading verified at line 7): "stop messing with the game's save file. just let it stay." It does not name either side of the split (who copies); it covers leaving the file alone.

## I1-012 CONFIRMED (K6)
- G\percall.py:102 `if data.get("tool_name") == "SubagentHandback"` saves the report; G\progress.py:19 `REPORT_TOOLS = ("SubagentHandback", "StructuredOutput")`; progress.py:119 and percall.py:168 use REPORT_TOOLS. So two places name the report tools and a StructuredOutput report is exempt from the budget but never saved.
- Tests: test_progress.py:209 and :370 loop over both tool names for the exemption; :389-412 test the save for SubagentHandback only. Nothing ties the two lists. Situation: a subagent that hands back through StructuredOutput. Whether a live subagent ever uses StructuredOutput stays open (needs a live run). Ruling: none seen.

## I4-078 CONFIRMED (K6 candidate)
- G\ruling_rules.py:214-215 default `cards_roots = [cards.CARDS_ROOT, cards.local_cards_dir(cards.DATA_ROOT)]`. SC\check.py:225 builds the same two folders in default_cards (plus staged, minus index.md, replace-by-name). Two places choose which card folders count. Split only if both claim "the cards": ruling_rules reads no staging folder, check.py does. Situation: a card in staging only. No test ties them. No ruling.

## I4-108 CONFIRMED-CHANGED (weak)
- prune_list.py:23-24 holds PROTECTED_FOLDERS and the full-UUID SESSION_ID, used at :69 and :123. check.py:38 CARD_FILE is an 8-hex prefix of a card file name, a different question (card name, not transcript name), so the "K6 weak" is thin. The K1 claim about common.py path pieces I did not re-open; marked unverified. No ruling.

## I10-066 CONFIRMED (K3)
- H\tools\hydra_ui_mcp.py:42 comment "how much longer than the timeout in force to wait for a reply"; used at :459 (`s.proc.wait(timeout=self.reply_margin_s)` for process exit in _stop_session) and :566, :636 (reply wait `s.timeout + reply_margin_s`). One number meaning two waits. Situation: slow Hydra shutdown. No value copy. No ruling.

## I10b-113 CONFIRMED-CHANGED (K1)
- Copy is real: H\tools\ci_build_folder.ps1:51 `$script:OldTime = [datetime]::new(2000, 1, 1, ...)` (finder said 21-32; that is BuildFiles) and H\tools\test_ci_build_folder.ps1:61 same literal. Grep over tools\ found only these two. The test fails if the script's value drifts, so the pair is pinned by behaviour. No ruling.

## I6-143 CONFIRMED (K7, K1) by finder's quotes; not re-opened (audit-method.md:110, :168-169). Reading of "nothing checks it" rests on the finder's searched list; I did not widen it.
## I7-139 CONFIRMED-in-part: K7 removal rule has no check; not re-opened.
## I7-162 CONFIRMED (K1/K3): this-machine.md:3 says "Fix the line in the same commit as the work that found it"; H\CLAUDE.md:9 says a session or agent updates the Clone Hero page in the same commit; brief-preamble.md:105 limits it to an agent whose brief owns the page, otherwise report it. Different actors, same duty. Different questions though (this-machine lines vs Clone Hero pages), so the preamble difference is a K3-style wording gap only. No ruling.
## I2-061, I7-137, I7-138 not re-opened; finder says single home. Unverified by me.

## Extra splits seen
- None beyond those above. Hook gap: none noticed.
