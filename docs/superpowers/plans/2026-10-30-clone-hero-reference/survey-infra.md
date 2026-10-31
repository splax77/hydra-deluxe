# Survey: the knowledge-base pattern and Hydra's code owners

Slice of the Clone Hero reference plan (task `ch-reference-plan`, session 3d58c2ba). Everything below was read from the files named; no test, build or game run was made. Where a search found nothing, the search is named.

## 1. The docs-vs-code test (tests/test_docs_match_code.cpp)

### Which docs it reads

`docs()` at lines 70-89 builds the list by hand: every `.md` directly in `docs/adr/` and `docs/agents/` (a non-recursive `directory_iterator`, line 74), plus five fixed files: `README.md`, `CONTEXT.md`, `docs/UserGuide.md`, `docs/development.md` and `docs/cap-clamped-squeeze-frontend-anchor.md` (lines 79-80). Each must exist and be non-empty (`REQUIRE_MESSAGE(!d.lines.empty(), ...)`, line 85). `docs/clone-hero/` is not in the list today; the folder does not exist yet (Glob `docs/clone-hero/**` found nothing).

### What it checks

Four checks, each its own doctest case (the header comment, lines 1-16):

1. "every backticked code name exists in src/, tools/ or tests/" (line 339). Every inline backticked span outside a fenced block and outside a section whose heading holds "Superseded" or "Kept as history" (`first_heading_skip_reason`, lines 230-234) is passed to `code_name_parts`. If that returns parts, each part must be a word in `code_words()`, or the doc-prefix/name pair must be on the `allowed()` list (lines 200-228), which only shrinks: an unused entry fails the test (lines 394-397).
2. The User Guide's `hydra_rules.ini` sample loads to `core::Rules{}` (line 403).
3. "every marked default equals the code's value" (line 460). A `<!-- default: Name -->` marker must name a key in `known_defaults()` (lines 247-277; an unknown name is a failure, line 293), the last number before it on the line must equal the code's value (lines 313-327), or for a flag the last word must be "on"/"off" and match. Every known marker must appear in some doc (lines 467-469).
4. The pictures the public pages link, the shot list and `docs/images` are one set (line 532).

### How it decides a backticked span is a code name

`code_name_parts`, lines 161-191. Quoted:

```
if (span.find_first_of("/\\=") != std::string::npos) return {};
static const std::regex file_ext(R"(\.(cpp|h|c|md|ini|json|py|ps1|exe|db|txt|mid|chart|sng|srb|opus|ogg|mp3|wav|flac|png|jpg|cmake|hlsl|dll)$)", std::regex::icase);
if (std::regex_search(span, file_ext)) return {};
... // a `(` right after a name marks a call; "Kick (Ghost)" is prose
static const std::regex qualified(R"(^(?:[A-Za-z_]\w*::)*[A-Za-z_]\w*$)");
if (!std::regex_match(span, qualified)) return {};
... // split on "::"
const bool k_name = last.size() > 1 && last[0] == 'k' && std::isupper(last[1]);
const bool underscore = last.find('_') != std::string::npos;
if (!call && parts.size() == 1 && !k_name && !underscore) return {};
return parts;
```

So a span is code only when it is `name(`/`name()`/`name(args)`, `Type::name`, `kName`, or a single name with an underscore. Anything with a slash, backslash, `=`, a known file extension, a space, a dot, a comma or a non-ASCII byte is prose and is skipped.

`code_words()` (lines 126-155) walks `src`, `tools` and `tests` through `sourcetree::for_each_source_file` (tests/source_tree.h: `kWalkedFolders = {"src", "tools", "tests"}`, line 34, via `git ls-files --cached --others --exclude-standard`, line 61). Only files whose extension `codelex::language_of_extension` knows are read: `.cpp .h .c .hlsl .py .ps1 .psm1 .yml .yaml` (tests/code_lexer.h lines 350-356). Code tokens count everywhere; string literals count only outside `tests/` (`strings_count`, line 134); comments never count. `.js` files (the Frida agent scripts in tools/ch_mcp/agent/) and `.md` files are not read. CMake targets are added from `CMakeLists.txt` (lines 145-153).

### What would happen to docs/clone-hero/ pages as they would be written

Checked against the predicates above, not run:

- `GameManager`, `ScoreManager`, `DrumsPlayer`: one part, no underscore, not `kName` -> skipped. Safe.
- Obfuscated class names: `dump_facts.py` line 55 says the obfuscator's names are non-ASCII, so `qualified` never matches -> skipped. Safe.
- C#-style `GameManager.Update` or `Int32.TryParse`: the dot fails `qualified` -> skipped. Safe.
- An RVA like `0x20DF680`: starts with a digit, fails `qualified` -> skipped. Safe. (Also never caught by check 3, which only reads `<!-- default: -->` markers.)
- `re\f215C750.asm` or `docs/clone-hero/code/x.asm`: slash or backslash -> skipped. Safe.
- `scorestats.json`, `song.ini`, `notes.mid`: file extension -> skipped. Safe.
- Python names in tools/: `RVA_DRUMS_ENGINE_CTOR`, `pressed_input_hit`, `ch_start` have underscores, so they are checked, and they pass because `tools/` is walked (constants.py, engine.py, game_tools.py are `.py`). A name that lives only in a `.js` agent file would fail.
- `legacy_fills`: checked (underscore) and found (src/cli/batch.cpp line 73 declares a local `legacy_fills`). Passes today, but it hangs on one local variable.
- Game names that WOULD fail: anything written `Class::Method`, `Method()` or with an underscore that is not in Hydra's `.py`/`.cpp` code or non-test strings, for example `DrumsEngine::HitNote()` or an Il2Cpp field like `m_starPowerAmount`. Il2Cpp dumps spell members C#-style with a dot, so the clean rule is: game names are written as the dump writes them (`Class.member`), never with `::` or `()`. Then they are skipped by this test and checked by a game-side check instead.
- Memory-style words in prose like `drums0dnoflip` -> skipped (test line 504 pins this).
- Section headings do not matter; only "Superseded"/"Kept as history" skip a section.

So adding the folder to `docs()` would not fail on game names by itself. It would fail on any Hydra name on a page that the code no longer has, which is the point.

### The change that lets Hydra names be checked while game names are checked against their own home

Two parts, both in this one test file plus one data file:

1. Add `"clone-hero"` to the `for (const char* dir : {"adr", "agents"})` loop at line 73. `directory_iterator` is non-recursive, so `docs/clone-hero/code/` is not read. That is right: the `.asm` and annotated files there are evidence, not prose. If the writer puts `.md` annotations in `code/`, the loop needs a `recursive_directory_iterator` for that one folder, and the fenced-block skip (line 350) already keeps listings out of check 1.
2. A fifth check, "docs match code: every game fact on a Clone Hero page has a home". Game facts need a form the test can find. Proposal: RVAs are written as `0x` plus hex (already prose to check 1), and game members as `Class.member` (already prose). The new check reads only docs under `docs/clone-hero/`, pulls every backticked span matching `^0x[0-9A-Fa-f]{5,}$` and every span matching `^[A-Za-z_]\w*\.[A-Za-z_]\w*$`, and looks each up in one committed table. The table is the single home the request asks for. Candidate: `docs/clone-hero/code/facts-6142.tsv` and `facts-4045.tsv`, one line per `class<TAB>member<TAB>rva<TAB>kind`, written by `tools/ch_mcp/dump_facts.py` from the dumps in `hydra-data\ch-dumps\{4045,6142}` (that script already parses `// RVA:` lines, line 45, and writes `facts.json` beside the dump, which is outside the repo). RVAs that tools use stay in `tools/ch_probe/constants.py` (29 hex addresses there; `ch_mcp/bot_run.py`, `dump_facts.py` and `frida_tools.py` import them from there, so the tools already have one home); the fifth check also asserts every `RVA_*` value in constants.py appears in the 6142 table, so the two homes agree. A fact on a page that the table lacks fails with the page, line and span, in the same style as check 1 (lines 386-388).
   The dumps are not in the repo, so the test reads the committed table only; regenerating it is a tool step, not a test step. If the user does not want a generated table committed (two files, size unknown until generated; dump.cs for one build is large), the fallback is to check pages only against `constants.py`, which covers the handful of RVAs the tools touch and leaves the rest unchecked. The request says not to leave that unstated, so the plan must put this choice to the user.
   Field offsets (`0x20`, `0x94`) are short hex and would collide with ordinary numbers; they are left to the page's evidence file, not the test, unless written as `OFF_*` names from constants.py (underscore -> checked by check 1 against tools code, which already works).

### How it runs, and whether precheck and CI run it

Locally: `build-cpp\Release\hydra_tests.exe "-tc=docs match code*"`. `tools/precheck/test_pick.cpp` lines 786-791 push three `always:` lines on every pick, the first being `always: "-tc=docs match code*"`, so `hydra_precheck --tests` always prints it (CLAUDE.md rule 1 says the main session runs each). CI: `.github/workflows/ci.yml` line 124 runs `ctest ... -R hydra_tests -E audio_device`, which runs the whole `hydra_tests` suite by shard, so the docs test runs on every push to `main` and every pull request; there is no separate docs step (grep of `.github/` for "docs match code" found only the precheck comment at ci.yml:62).

## 2. CLAUDE.md, the preamble, and what reads them

### Where the lines go

`CLAUDE.md` has "## Agent skills" at line 3 with nine `###` subsections (Issue tracker 7, Triage labels 11, Domain docs 15, Past sessions 19, Audits 27, GUI testing 31, Public doc style 35, Derive-once review 39, Doc review 45), then "## Agent rules" at line 53. The file is 2,101 whitespace-separated words (PowerShell split count). The reference line fits as a new `### Clone Hero reference` subsection between "Domain docs" (line 15) and "Past sessions" (line 19), two or three sentences: read `docs/clone-hero/README.md` before any Clone Hero question and before any plan, test or probe that touches the game; a learned fact goes on its page in the same commit. "Domain docs" (line 15-17) is the nearest existing shape: one sentence plus a pointer to a docs/agents page. Budget proposal: at most 80 words in CLAUDE.md, since every session loads it.

The write-back rule in `docs/agents/brief-preamble.md` fits as one short paragraph in "## The code is the source of truth" (lines 81-87) or a new "## Facts you learn" section before "## When blocked" (line 89). Budget proposal: at most 60 words.

### What reads these files

- `C:\Users\Patrick\.claude\gates\tests\test_docs_numbers.py` reads `CLAUDE.md`, `docs/agents/brief-preamble.md`, `derive-once-review.md` and `doc-review.md` (`DOCS`, line 17). It fails when a line matches a retired word (`RETIRED`, lines 21-25: `\b110\b`, "every 10 tool calls", `hooks[/\\]status_append`, `derive_once_submit`, `doc_review_submit`, `reviewer_dispatch_gate`, `doc_review_key`, `doc_review_sources`, `main_move_check`, `hooks[/\\](state|tests)`), when a "N tool calls" count is not 100 or 150 (`CALL_COUNT`, line 36), when "every N minutes" is not 5 (line 37), and it requires the rule-7 phrases (lines 93-108). A new paragraph that avoids numbers of tool calls and those words passes. It also checks every `agents/*.md` in the hooks repo for `maxTurns` (line 71-79).
- `C:\Users\Patrick\.claude\gates\progress.py` line 1 and `agent_stop.py` line 19 mention CLAUDE.md only in comments.
- In Hydra `tests/`, grep for `CLAUDE\.md|brief-preamble` found nothing. The docs-vs-code test reads every `docs/agents/*.md`, so the preamble's backticked names are already checked (write `docs/clone-hero/README.md` with its extension and it is skipped as a file name).

### Are docs/clone-hero/ pages gated?

No. `review_rules.py` line 23: `PUBLIC_DOCS` is exactly `README.md`, `docs/README.md`, `docs/UserGuide.md`, `docs/differences-from-public-hydra.md`. `publish_gate.py` gates only `ExitPlanMode` plans, `Artifact` publishes and posts through `publish.ps1` (lines 110-120). A page under `docs/clone-hero/` is an internal doc: a move of `main` that only adds or edits it needs no doc review.

### What the derive-once ref gate needs

`review_rules.py` line 21: Hydra's `.derive-once-gate` holds only prose (its text names an old `hooks\lib` function; the gates code ignores that), so `BUILTIN_CODE` applies: `^((src|tests|tools|third_party|installer)/|(CMakeLists\.txt|CMakePresets\.json|build_cpp\.ps1)$)`. `explain_main_move` (lines 179-209): if any changed file matches, the new tip must be a key a reviewer signed CLEAN, or a clean merge of main with such a key ("carry"), or waived. So:

- The test change (`tests/test_docs_match_code.cpp`, and the `tools/ch_mcp/dump_facts.py` change if the table generator is extended) is code: its own branch, one reviewer dispatched with `subagent_type: reviewer`, the tip hash as key, then merge.
- A docs-only commit (pages under `docs/clone-hero/`, `CLAUDE.md`, `docs/agents/brief-preamble.md`, memory pointers, ADR 0023, `docs/audit/ch-evidence.md`) touches no `BUILTIN_CODE` path, so the ref gate says "no-review-needed" and `main` moves. The request still wants one fresh review of the pages for trust; that review is dispatched the same way but nothing refuses the merge without it. If the committed facts table lives under `docs/clone-hero/code/`, it is not code to the gate either; the test that reads it is.
- Every commit needs the three trailers (`Task:`, `Agent:`, `Session:`), or the ref gate refuses it (ref_judge.py line 225).

## 3. Changing C:\Users\Patrick\.claude\agents\card-writer.md

From `C:\Users\Patrick\.claude\gates\README.md` "Working on the hooks" (lines 76-84) and "Making a change live" (lines 104-108): the hooks repo's `main` is live in every session on the next tool call, so nobody edits it in place. Work goes in a worktree under `C:\Users\Patrick\.claude\worktrees\<task>` on a branch named for the task (line 82; the delete rule trusts only that folder). The repo's `main` holds a `.derive-once-gate` whose `gate:` lines name `gates/`, `agents/` and `settings.json` (line 84), so `agents/card-writer.md` is gated code there: one fresh reviewer before it reaches `main`, dispatched as `docs/agents/derive-once-review.md` says, with the brief naming the hooks repo root and the commit (CLAUDE.md "Derive-once review"). The main session then merges `main` into the branch in the worktree, runs the merge's tests there, and fast-forwards: `git -C C:\Users\Patrick\.claude merge --ff-only <branch>` (line 106). The commit carries the three trailers.

Today's worktrees there: `push-trusts-gated-main` and `review-gate-simplify-plan` (git worktree list); `main` is at f3ecafa.

Hook tests that touch `agents/card-writer.md` (grep of `gates\tests` for `card-writer`): one match, `test_h53_session_cards_start.ps1` line 135, which checks the start-of-session note's text names "card-writer" and "never to run check.py"; it reads `session_cards_start.ps1`, not the agent file. `test_docs_numbers.py` `test_no_agent_file_sets_a_turn_cap` (lines 71-79) reads every `agents/*.md` and fails on `maxTurns`. No test reads the agent file's instructions. The README's rule (line 102): a merge runs the test files for the scripts it changed, so a card-writer-only change runs `test_docs_numbers.py` (`-p test_docs_numbers.py`) and, to be safe, `test_h53_session_cards_start.ps1`.

The writer brief, `session-cards\writer-brief.md`, is untracked (its own line 5 and the request say so). It is where the card shape lives (lines 19-53): seven fixed `##` headings, 60-line cap, and "Rules a checker enforces" (lines 59-67). A "missed write-back" flag fits as one new optional line under "## Open threads", next to the existing `memory:` line (line 47), for example `clone-hero: <fact the session learned that docs/clone-hero/<page> lacks>` or `clone-hero: none`. The checker (`check.py`, which the brief says the main session runs after a wave) would need to accept the line; the brief says the shape is checked, so the plan must have the executor read `check.py` and add the line to what it allows. The agent file `card-writer.md` (10 lines) only points at the brief and sets the tools (`Read, Glob, Grep, Write, PowerShell`, no git); the change there is one sentence telling the writer to read `docs/clone-hero/README.md`'s topic list so it can tell a Clone Hero fact from anything else. Note the brief's line 84 still names `hooks/status_append.ps1`, a retired path; `test_docs_numbers.py` does not read the brief, so it does not fail, but the executor should fix it to `gates/status_append.ps1` while there.

## 4. Hydra's code owners for the seven areas

One line each, from grep of `src/` and the single-owner rules under `tests/single_owner/rules/`. "SO:" marks a question the rules already name an owner for (file and line of the rule).

Area 1, finding and reading a chart:
- Which notes file wins when a folder has several: `pick_notes_file` in src/parse/chart_files.cpp (SO: parse.json:126). Which files are notes files: `notes_file_format` and `chart_format_of` in chart_files.cpp (SO: parse.json:149, :339).
- `.mid` reading: `MidiParser::parse` in src/parse/song.cpp line 1064; the lean read `parse_mid_lean` line 1913; byte walk `walk_track` in src/parse/midi.cpp (SO: parse.json:687).
- `.chart` reading: `ChartParser::parse` song.cpp line 1804; what a line says and does: `classify_chart_line`, `ChartParser::optype` (SO: parse.json:626, :656); sections kept by name: `kSongSection`, `kSyncTrackSection`, `kEventsSection` (SO: parse.json:808).
- `.sng`: src/parse/sng.h (`kSngXorMaskOffset` line 34 and the following offsets), src/parse/sng.cpp. `.srb`: src/parse/srb.h (`kSrbHeaderSize` line 43), `srb_read_metadata` in srb.cpp (SO: parse.json:174), `srb_inflate_stream_reading` song.cpp line 1999.
- Track names: `kDrumsTrackNames = {"part drums", "part drum"}` and `kEventsTrackName = "events"` in src/parse/midi.h lines 97-98; `MidiFile::drums_track` / `is_drums_track_name` midi.cpp line 446 (SO: parse.json:722).
- `song.ini`: found by `find_song_ini` in chart_files.cpp line 40 (SO: parse.json:200); read by `ini_delay_ms` / `parse_delay_ms` in src/app/preview_source.cpp lines 301 and 113, stored through `app/analysis.cpp` line 155 into `timing.delay_ms`. The `.sng` metadata copy: `sng_metadata_delay_ms` preview_source.cpp line 182.
- Sections: `is_section_marker` song.cpp line 35; shown name `section_display_name` in src/parse/section_names.cpp (SO: parse-fills.json:4).
- Solos: `find_solo_sections` song.cpp line 63, into `Song::solo_sections` (SO: parse.json:248, :400).

Area 2, building the note list:
- Cymbals and toms: `cymbal_counts(pad, pro)` src/core/song.cpp line 73 (`return pro && allows_cymbals(pad)`); marker pitches `kMarkerPitches` / `is_midi_marker_pitch` song.cpp (SO: parse.json:224).
- Dynamics and the enabling tag: the tag test at song.cpp line 303 (`s == "ENABLE_CHART_DYNAMICS" || s == "[ENABLE_CHART_DYNAMICS]"`); `.chart` always on (comment at line 1891); `Chord::apply_ghost` / `apply_accent` src/core/model.h lines 257-258.
- Disco flip: `emit_chord_timestamp` song.cpp lines 418-427 (`if (pro && in_disco) chord.apply_disco_flip();`), `Chord::apply_disco_flip` model.h line 234; the marker's N at song.h line 53.
- 2x kick: `kick2x_pitch()` src/parse/song.h line 61 (`kick_pitch - 1`); the `bass2x` parse argument (song.cpp lines 767, 1382); `kDynamicsParseBass2x` src/app/dynamics_breakdown.h line 175.
- Note Shuffle: `apply_note_shuffle` src/parse/note_shuffle.h line 37, switched by `apply_shuffle_switch` song.cpp line 1931 and `ChartReading::noteshuffle` src/parse/chart_reading.h line 26; `NoteShuffleFreezes` src/core/error_kind.h line 32.
- What plain Drums drops: cymbals, by `cymbal_counts` above; dynamics are kept on purpose (memory `ch-plain-drums-drops-dynamics`, user ruling); labels by `note_label(note, pro)` model.h line 80.

Area 3, timing:
- Chart `Offset`: song.cpp lines 1820-1825 into `Song::chart_offset_s` (src/core/song.h line 130). `song.ini` `delay`: `ini_delay_ms` above; the "nonzero delay replaces Offset" rule (memory `ch-audio-delay-offset-rule`) must be located by the scout: grep of `src/` for `chart_offset_s` shows only song.cpp:1825 and song.h:130 as writers; the reader that combines them was not found in this survey (searched `delay|offset` in src/parse and `"delay"|delay_ms|chart_offset_s` across src/).
- Song clock: `Song::timecode(tick)` (song.cpp line 424 uses it); tempo map `SongTiming::tempo_at` src/core/timing.cpp (SO: core-measures.json:71).
- Hit window and cap: `kDefaultHitWindowMs = 85.0` src/core/model.h line 108; the setting `Settings::hit_window_ms` src/app/config.h line 167 (display-layer only, comment at line 158); the probe's formula and clamp facts live in tools/ch_probe/constants.py (`RVA_WINDOW_FORMULA`, `OFF_BACK_WINDOW`, `OFF_FRONT_WINDOW`).
- Calibration: no owner in `src/` (grep `calibrat` across src/ found nothing). Hydra does not model it.
- Playback speed: no owner in `src/` for game speed; `kBaseSpeedPercent = 100` src/net/dmbot_client.h line 35 is the leaderboard's speed field, and the Preview "always plays at 1x" (src/render/preview_renderer.cpp line 431).

Area 4, fills:
- Written fills, generated fills, and which generator: `Song::check_activations` src/core/song.cpp (SO: parse-fills.json:49, :94); `kNotesOnlyFills = FillDeadlineRule::Ch11` src/parse/song.h line 175.
- 1.0 vs 1.1 rule: `FillDeadlineRule`, `fill_rule_for(legacy_fills)`, `legacy_fills_for(rule)` src/search/graph.h lines 45-51; `kCh11FillLeadBeats = 4` line 71; `fill_rule_description` line 75 (SO: concept-F7.json:58).
- Fill deadline: `activation_fill_deadline_ms` graph.h line 85; stored per activation edge line 166.
- Early fill reach and slack: `kEarlyFillReachMs = 85.0`, `kEarlyFillSlackMs = 60.0` model.h lines 137-140 (D250).
- The legacy flag: `kLegacyFillsFlag = "--legacy-fills"` src/app/config.h line 53; `legacy_fills_help_text` src/ui/library_parts.h line 47.

Area 5, Star Power:
- Phrases: `close_sp_phrase` song.cpp line 401 (comment cites 1.1's `0x20D2440`), `SongTimestamp::sp_phrase_start` song.h line 42, `Song::sp_phrase_count` song.cpp line 83.
- Meter and full-bar size: `kCloneHeroSpCap = 4` model.h line 36; `max_sp_bars` graph.h line 93; `graph_build_cap` src/search/pather.h line 54.
- Activation: `kSpActivationBars = 2` src/core/timing.h line 51; engine checks at src/search/engine.cpp lines 818, 946, 965.
- Drain and where SP ends: `SpEndKind` model.h line 467-473 (Collected +2 measures, Clamped, SqIn); `activation_initial_end_times` graph.h line 167; `offset_from_sp_end` model.h line 118; `after_sp_end` src/core/backend_value.h line 36. Grep of src/ for `\bdrain` found no symbol; the drain is expressed as measures per bar in those end-time maps, which the scout must confirm in engine.cpp.
- Phrases collected during SP: `add_sp_points` engine.cpp (SO: d115-sp-points.json:4); `offered_phrase` (D36) and `banked_phrase_in_reach` named at graph.h lines 127-171.
- Squeezes: `kSqueezeWindowMs = 500.0` model.h line 114; `within_squeeze_window` line 126; `SqueezeKind` line ~402 (SO: concept-F7.json:79); rating in src/core/squeeze_rating.h (`rate_note` line 130, `squeeze_budget_ms` line 78); `activation_can_squeeze` src/core/sqout_chord.h line 51.
- The 3 ms leeway: `Rules::backend_leeway_ms = 3.0` src/core/rules.h line 32; applied by `counted_without_squeeze` backend_value.h line 41 (`paid_by_sp_walk(offset_ms) || offset_ms < leeway_ms`).

Area 6, scoring:
- Note values: `kNoteBasePoints = 50`, `kCymbalBonusPoints = 15` model.h lines 148-149; `ChordNote::basescore` is the per-note 1x owner (scoring.h header).
- Multiplier and combo: `to_multiplier(combo)` src/core/timing.h line 31; `kStarPowerMultiplier = 2` line 48; `category_scores` src/core/scoring.h line 58.
- Accents and ghosts: `CategoryScores::accent/ghost/dynamics_bonus` scoring.h lines 23-46; `ChordNote::is_accent/is_ghost` model.h lines 176-177.
- Solo bonus: `solo_bonus`, `solo_note_count` scoring.h lines 64-68; `kSoloBonusPerNote = 100` model.h line 151.
- Star cutoffs: `kStarMultipliers {0.1,0.5,1.0,2.0,2.8,3.6,4.4}`, `kMaxStars = 7`, `star_cutoff`, `star_cutoffs` src/core/stars.h lines 21-42 (header says decoded from 6142 on 2026-09-27).
- `scorestats.json`: no owner in `src/` (grep found none there); the tools own it: `tools/ch_mcp/game_files.py`, `tools/ch_mcp/compare_scorestats.py` and its test, `tools/ch_probe/experiments/sp_compare.py`. Score totals that mirror it: `score_total` model.h line 157 (SO: concept-F7.json:4) and `ReplayScore` / `kReplayScoreFields` src/core/replay.h lines 112-140.

Area 7, game internals the tools touch: no `src/` owner; the homes are `tools/ch_probe/constants.py` (RVAs, offsets, `PER_NOTE_DEFAULT_RVAS`), `tools/ch_probe/engine.py` (`is_precision`, `pressed_input_hit`; SO: probe.json:71, :256), `tools/ch_mcp/frida_tools.py`, `tools/ch_mcp/game_tools.py`, `tools/ch_mcp/bot_run.py`, `tools/ch_mcp/dump_facts.py` (`NAMED_SIGNATURES = ("GameManager", "ScoreManager", "DrumsPlayer")`, line 29) and the `.js` agents under `tools/ch_mcp/agent/`.

Single-owner rules that already name an owner for these topics are the `SO:` entries above; they come from tests/single_owner/rules/{parse,parse-fills,core-measures,probe}.json and rules/search/{concept-F7,d115-sp-points}.json. There is no rule yet for the hit window, the 3 ms leeway, the SP cap, star cutoffs, note values or the delay/Offset choice.

## 5. Sizing the four infrastructure tasks (one Opus executor each, 100 calls / 20 minutes)

(a) The CLAUDE.md reference line. Owns `CLAUDE.md` only. One subsection of two or three sentences under "## Agent skills". Tests it may run: none in Hydra; the hooks repo's `test_docs_numbers.py` reads CLAUDE.md, so the executor runs `python -I -m unittest discover -t . -s gates/tests -p test_docs_numbers.py` from `C:\Users\Patrick\.claude` with `GATES_HYDRA_ROOT` pointing at its worktree. Docs-only, so the ref gate needs no review; the request still wants a reviewer for trust. Needs the pages first: yes for the final wording (it names `docs/clone-hero/README.md`), but the line can be written in the same wave as the pages if it names the agreed path. About 10 calls. Could merge with (b).

(b) The write-back rule in CLAUDE.md and docs/agents/brief-preamble.md. Owns `CLAUDE.md` (the same subsection as (a), so (a) and (b) are one task in practice) and `docs/agents/brief-preamble.md`. Same test as (a), plus `hydra_tests.exe "-tc=docs match code*"` because the preamble is in `docs()`. Docs-only. Needs the pages first: no; it can name the folder. About 15 calls. Recommend folding (a) into (b) as one task.

(c) The docs-vs-code test change. Owns `tests/test_docs_match_code.cpp`, and, if the facts table is chosen, `tools/ch_mcp/dump_facts.py` (a `--tsv` output) plus the two generated tables under `docs/clone-hero/code/`. Tests it may run: `hydra_tests.exe "-tc=docs match code*"` (warm build of `hydra_tests` only), and `py -m pytest tools/ch_mcp/tests -q` if dump_facts.py changes. Gated code: its own branch and reviewer. Needs the pages first: yes to pass green (the loop `REQUIRE`s each listed doc non-empty, line 85, and the new check needs at least one page with a fact to prove it checks something, as check 1 does with `names_checked > 100`, line 398). Can start early with a placeholder `docs/clone-hero/README.md` and a failing-first unit case for the new predicate (same style as "what counts as a code name", line 494). Generating the tables needs the dumps on disk at `hydra-data\ch-dumps`, so that step reads outside the repo; the executor must not need the game. About 40-60 calls plus one warm build. The open user question (commit a generated table, or check RVAs against constants.py only) must be answered before this task starts.

(d) The card-writer change. Two owned files in two repos: `C:\Users\Patrick\.claude\projects\...\session-cards\writer-brief.md` (untracked, edited in place) and `C:\Users\Patrick\.claude\agents\card-writer.md` (in a hooks worktree under `C:\Users\Patrick\.claude\worktrees\<task>`, its own branch, one reviewer, ff-only merge). The executor also reads `session-cards\check.py` to accept the new card line and, if it changes it, owns that file too (untracked, so no gate). Tests: `test_docs_numbers.py` (`maxTurns` check over agents/*.md) and `test_h53_session_cards_start.ps1`. Needs the pages first: only the README's topic list, which the writer brief can name by path. About 25 calls. No Hydra build.

Order: (c) and (d) are independent of each other and of the page writing beyond a placeholder README, so they can run in the same wave as the writer, as long as (c)'s table question is answered. (a)+(b) is one small task that can go in that wave too.

## Promise map

| Promise in this survey | Where it sits | Task | Check that proves it |
|---|---|---|---|
| Hydra names on docs/clone-hero pages are checked at every merge | section 1, change part 1 | (c) | `-tc=docs match code*` fails on a page naming a removed Hydra symbol (executor proves with a red-then-green scratch page); precheck prints the `always:` line (test_pick.cpp line 789) |
| Game names written `Class.member` and RVAs are never mistaken for Hydra names | section 1 | (c) | new cases in "what counts as a code name" pin `code_name_parts("GameManager.Update").empty()` and `code_name_parts("0x20DF680").empty()` |
| Every RVA and game member on a page has a single home and is checked against it | section 1, change part 2 | (c) | the fifth test case fails on a page span absent from the facts table, and on a constants.py `RVA_*` absent from the 6142 table (unit case with a fake table) |
| docs/clone-hero pages need no doc-review gate; the test change needs a derive-once review | section 2 | main session | `review_rules.py` lines 21-23 quoted; a docs-only merge of main moves with "no-review-needed", the test branch needs a CLEAN key |
| CLAUDE.md and the preamble keep passing the hooks' doc test | section 2 | (a)+(b) | `test_docs_numbers.py` run against the worktree with `GATES_HYDRA_ROOT` |
| The card writer flags a missed write-back | section 3 | (d) | one card written on a fixture session shows the `clone-hero:` line; `check.py` accepts it; `test_docs_numbers.py` and `test_h53` pass |
| The card-writer agent file change is reviewed before it is live | section 3 | (d) + main session | hooks repo `.derive-once-gate` names `agents/` (README line 84); ff-only merge after a CLEAN verdict |

Out of scope for this slice (for the user): whether to commit generated facts tables; the delay-vs-Offset reader in src/ that this survey did not find; calibration and playback speed have no Hydra owner, so their pages map to "no Hydra code" rather than a function.
