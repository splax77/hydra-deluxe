# CMakeLists.txt is the one owner of library membership

Apart from the programs' own source files (src/ui/main.cpp and src/cli/), the
code under src/ is built as twelve libraries that Hydra's own programs link:
Hydra.exe, the command-line tools, the tests and the GUI test runner. Which
library a file is in is written in one place, the source lists in
`CMakeLists.txt`. Which libraries a library may use is written there too, in
its link lines (target_link_libraries). A test, tests/test_layering.cpp, reads both
and fails when a file includes a header from a library its own library does
not link. The plan is docs/superpowers/plans/2026-10-28-hydra-libraries.md,
and the split was built from it in tasks T3, T1a, T1b, T2, T1c, T5, T4 and T6
on 2026-10-29.

Think of the libraries as the floors of a building. A file may walk downstairs
to any floor its own floor's stairs reach, but never upstairs. The link lines
are the stairs, and the layering test walks every include to check.

## Why

Before the split, everything below the GUI was one library, so a program
that wanted only the Preview, or only the window set-up, got all of Hydra.
The folders already looked like components, but the include lines did not.
The scoring code in core/ read the `Song` class out of parse/, the replay in
core/ reached up into app/ for a text formatter, app/ reached up into audio/
for the chart sync rule, and the GUI's window set-up sat in the same file as
the code that draws Hydra's whole window. The split fixed those includes and
gave each part a library of its own, so a new program links only the parts it
needs. The Path Builder is the first program that does.

## The libraries

`CMakeLists.txt` holds the exact lists. This is their order, lowest first,
with what each one's link lines name.

`hydra_core` is the chart model, the `Song` class, scoring, replay, stars,
timing, rules and strings. It links nothing of Hydra's. `hydra_image` is the
one stb_image build, and it links nothing of Hydra's either.

`hydra_parse` (the chart, MIDI, .sng and .srb readers) and `hydra_search` (the
graph, the engine and the pather) each link `hydra_core`. `hydra_store` (the
database, the chart index and the library file) links `hydra_core`, and
`hydra_search` privately, because only its .cpp needs it. `hydra_app` sits on
top of the logic: analysis, settings, reports and the text the screens show.
It links `hydra_parse`, `hydra_search` and `hydra_store`, and it has no GUI in
it. `hydra_batch` and `hydra_fillcompare` link it alone.

`hydra_audio` (decode, mix and play), `hydra_render` (the Preview's note
highway) and `hydra_dm` (the DMBot leaderboards) each link `hydra_app`.
`hydra_render` also links `hydra_image`. `hydra_preview` is the Preview's
controller, loader and transport, with no ImGui in it; it links `hydra_app`,
`hydra_audio` and `hydra_render`. `hydra_uibase` is the GUI base: ImGui's
set-up, the theme, the fonts, the shared widgets, the icons and the folder
dialog. It links ImGui, `hydra_render` and `hydra_audio`, and nothing in it
reads the app's state. `hydra_ui` is Hydra's own window, and it links
`hydra_uibase`, `hydra_preview` and `hydra_dm`.

Two more libraries serve the GUI test runner. `hydra_uitest_engine` is the
app-free half: the offscreen device, the test engine, the command verbs and
the script runner. It links `hydra_uibase`. `hydra_uitest_harness` is
Hydra's half and links `hydra_uitest_engine` and `hydra_ui`. Two tool
libraries, `hydra_precheck_lib` and `hydra_replay_json`, sit beside the
programs that use them.

## Decisions

**Files stay in their folders.** A library is a list in `CMakeLists.txt`, not
a folder. So src/ui/ holds files of three libraries (`hydra_preview`,
`hydra_uibase` and `hydra_ui`), and src/app/ holds files of two (`hydra_app`
and `hydra_dm`). Moving the files to match would have meant editing include
lines in dozens of files and hundreds of single-owner rule lines, and it
would change nothing a program can see. A header with a .cpp file of the same
name belongs to that file's library. A header with no such twin is listed on
its own line beside its folder's .cpp files.

**The `Song` class lives in core/.** It moved from src/parse/song.h to
src/core/song.h, with the code that reads only the model: the constructor,
the generated fills, `apply_timesig` and the timing-map checks. The loaders and
the name helpers stayed in parse/, and src/parse/song.h includes the new
header. That ended the cycle between core/ and parse/, so each can be a
library of its own.

**The test owns no list of its own.** It reads the source lists and the link
lines from `CMakeLists.txt`. A PUBLIC link passes on to whoever links the
library; a PRIVATE link serves that library only. Every branch of an `if()` in
the file counts as taken. A third-party or system header stands for a link
line through a row in tests/layering/externals.json, and each row carries
examples the test checks it against. So a new library needs no edit to the
test, and a new third-party header needs one row. A file under src/ that no
library lists fails the test too.

**Wrong-way includes were parked with a date, and none are left.** While the
split was being built, tests/layering/known_gaps.json listed the includes that
still pointed the wrong way, each under the task that would remove it. Its
dates went through the single-owner baseline's reader (`parse_dates` in
tests/single_owner/engine.h), so the same 14-day cap held (ADR 0029). Each task
deleted its own entries as its proof. The list is now empty, and the test case
"layering: no known gap is left" keeps it so. A new wrong-way include is fixed
in its own change, never parked: an entry would hide it, because the dated
case only checks that an entry names a real include.

## Rejected

**Folder moves to match the libraries.** See "Files stay in their folders".

**One library for core/ and parse/ together.** It cost nothing at once, but
it left the cycle in the include lines. The `Song` move broke the cycle
instead, and it dragged no loader with it.

**Known gaps with no dates.** Grouping entries by task, with no clock, would
have worked too. Reusing the baseline's reader kept one owner for the date
rule, and the eight tasks finished well inside 14 days.

**A library list written in the test.** That would be a second copy of the
build's membership, and the two would drift.
