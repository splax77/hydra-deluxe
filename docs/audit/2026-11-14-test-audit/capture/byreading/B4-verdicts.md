# B4 verdicts - agent ab9725b884ba9959b (batch B4, 43 records, all judged by reading; no capture run served any of them)

Production code followed: tools/precheck/test_pick.cpp (Picker::run, collect_names, names_of, collect_callers, caller_scope, call_site, grep_tests, test_scope, exe_for, shared_code_in), tools/precheck/changed_names.cpp (Finder, classify, read_type/read_constant/read_function), tools/precheck/range.cpp (ChangedLines::parse_diff, resolve_range), braces.cpp, views.cpp, definitions.cpp (keywords, namespace_head), tools/ch_mcp/{chdis.py, game_files.py, bot_run.py}. Every test calls the real function (pick_tests, shared_code, changed_names, ChangedLines::from_diff, resolve_range, Image.dis, set_song_title, Env) on real git fixture repos or real text; no stand-ins. Git's own diff output (hunk shapes) is assumed standard.

| record | test (file:line) | claim | verdict | evidence |
|---|---|---|---|---|
| C9 1.2 | test_precheck_pick.cpp:84 | changing `a()` picks tests naming `run` and `Mode::A` only, not the 2-levels-up `outer` | HOLDS BY READING | changed_names gives `a:declared` for line 2 (changed_names.cpp declared()); call_site (test_pick.cpp:671-702) records `run`<-a and selector `Mode::A` from the call line; test_scope falls to plain search (declares_type true in a .cpp -> nullopt, :631); `outer` is never a key because only names present before the loop (:654) are walked |
| C9 1.3 | :117 | selector on the `if` guard line above the call counts | HOLDS BY READING | call_site guard walk (test_pick.cpp:685-697): the `if` brace is on line 4, loop reaches line 4, guard_line true, selector_values gives `Rule::New` |
| C9 1.4 | :136 | deleted class gives the class name, not its members | HOLDS BY READING | names_of (:485-500): `public:` line gives side as Declared but `enclosed` (Square Body) plus declares_function skips it (:494-496); `int s_ = 0;` classifies Other; test_scope returns nullopt for base_only (:617) so plain search picks test_square only |
| C9 1.5 | :158 | multi-line member body gives the class, not the member | HOLDS BY READING | body lines: enclosing() marks side `member=true` (changed_names.cpp:315-317), names_of skips members (:493); `const int twice = ...` reads as Constant but is dropped (`body` and not Type, changed_names.cpp:353) |
| C9 1.6 | :180 | function changed only by the comment above does not walk callers | HOLDS BY READING | declared(...,comment=true) gives `shallow:comment`; collect_callers skips Comment (:659); only test_shallow names it |
| C9 1.7 | :200 | same-named unrelated function in another folder is not a caller | HOLDS BY READING | caller_scope (:583) = one.h plus its includers; b/two.cpp does not include it, so `go`/`Kind::B` are never recorded; only test_mode_a reaches one.h |
| C9 1.8 | :223 | caller reaching the header through another header counts | HOLDS BY READING | reaching() follows includers through mid.h to far.cpp; declaring_headers walks mid.h then one.h for the enum (:526-541) |
| C9 1.9 | :241 | .cpp that reaches its header only through another header still has callers | HOLDS BY READING | function_headers -> declaring_headers: level 1 mid.h has no `run(`, level 2 one.h does (:526-541) |
| C9 1.10 | :258 | function only in a .cpp has callers only in that file | HOLDS BY READING | function_headers empty, scope = own file; there.cpp never scanned |
| C9 1.11 | :277 | tools/ function's callers in tools/ are followed | HOLDS BY READING | kIncludeRoots includes tools; lib.h declares `step(`; main.cpp is in scope and production; selector Opt::Fast resolved through lib.h |
| C9 1.12 | :292 | key in a helper header picks every test including it | HOLDS BY READING | naming() (:407-419) puts includers of tests/calc_helpers.h in via_helper; grep_tests adds them (:735) |
| C9 1.17 | :388 | header with more includers than the limit runs whole suite | HOLDS BY READING | shared_code_in (:180-184) counts direct includers via IncludeMap (src/u1, src/sub/u2, tools/u3 via ../; u4's comment-only include is blanked); 3>2 whole suite, 3>3 false |
| C9 1.27 | :442 | changed CMakeLists.txt makes the pick the whole suite | HOLDS BY READING | Picker::run (:354-358) returns at the first shared-code file; file_name==CMakeLists.txt; tests stays empty |
| C9 1.28 | :450 | files under tests/single_owner/ never picked | HOLDS BY READING | runnable_test needs no second `/` after tests/ (:28); helper_header excludes the folder (:34); only test_k via kStamp |
| C9 1.29 | :483 | test naming an unrelated same-name function not picked | HOLDS BY READING | test_scope(run) = reaching(one.h) = one.h, one.cpp, fix.h, test_t_a, test_t_c; test_t_b only reaches two.h |
| C9 1.30 | :492 | test reaching header through a test helper header is picked | HOLDS BY READING | fix.h includes a/one.h so test_t_c is in reaching(one.h) |
| C9 1.31 | :502 | name in a main file no exe target lists is found by plain name | HOLDS BY READING | exe_for: no CMakeLists.txt -> raw() returns an empty optional (git_text.cpp:228) -> no targets -> nullptr; definer_headers returns nullopt for main (:556) -> plain search |
| C9 1.32 | :515 | deleted class still picks a test that names it, no include needed | HOLDS BY READING | base_only -> test_scope nullopt (:617) -> plain search; test_none only includes the header, never names Ghost |
| C9 1.33 | :529 | selector picks only tests reaching its enum's header | HOLDS BY READING | declares_type false in place.cpp (the word before `Rule` is `(`), declaring_headers finds rule.h, reaching excludes test_rule_out |
| C9 1.34 | :549 | function with no declaring header found by plain name | HOLDS BY READING | definer_headers -> function_headers empty -> nullopt -> plain |
| C9 1.35 | :561 | selector whose left part is a namespace found by plain name | HOLDS BY READING | selector_values gives `util::kFast`; declares_type(...,"util") false (word before it is `namespace`, not enum/class/struct/union), declaring_headers empty -> nullopt -> plain |
| C9 1.38 | :623 | exe with no constant in CMakeLists keeps plain-name search | HOLDS BY READING | exe_for: one target found, defines_constant false (:447) -> nullptr; plain search picks test_b not test_a |
| C9 1.39 | :631 | main file two exe targets list keeps plain-name search | HOLDS BY READING | exe_targets_listing returns 2 targets, `size() != 1` -> nullptr (:445) |
| C9 1.40 | :640 | exe constant no test names keeps plain-name search | HOLDS BY READING | tests_naming(HYDRA_TOOL_EXE) empty -> nullptr (:449) |
| C9 3.1 | test_precheck_pick_lines.cpp:38 | deleted lines in base numbering; deleted file keyed by base path | HOLDS BY READING | parse_diff (range.cpp:157-200): hunks `-2,3 +1,0` and `-5,0 +3`; `+++ /dev/null` leaves cur empty so no added entry; `--- a/` sets old_cur |
| C9 3.2 | :68 | a hunk records both sides | HOLDS BY READING | hunks `-0,0 +1` and `-2,3 +3` give added {1,3}, deleted {2,3,4} |
| C9 3.4 | :103 | merge commit alone resolves to first parent...second parent | HOLDS BY READING | resolve_range (range.cpp:104-124): two parents -> left parents[0], tip parents[1], base fork_point; whole_tree keeps the merge as tip; one parent -> base tip^1 |
| C9 4.1 | test_precheck_pick_names.cpp:28 | line in member body gives `note_count` without `Song::` | HOLDS BY READING | read_function takes the word before `(`; enclosing stops at the namespace head -> `note_count:body` |
| C9 4.2 | :38 | local struct gives struct and function | HOLDS BY READING | enclosing: Local (Type) then run -> `Local:body run:body`; `int y = 0;` classifies Other |
| C9 4.3 | :49 | Cell inside place_fills_ch10; plain block gives only the function | HOLDS BY READING | `for (...)` head is Other (keyword `for` is not an ident), name empty, skipped |
| C9 4.4 | :69 | changed comment line above function gives the function (comment) | HOLDS BY READING | Finder::run comment branch (:231-240) calls declared(...,true) on the next code line |
| C9 4.5 | :83 | constexpr/static const/brace-init gives the constant | HOLDS BY READING | read_constant handles the `=` and head-`{` forms; Constant -> NameKind::Value |
| C9 4.6 | :95 | line in enum gives `Enum::Value`, then the enum as body | HOLDS BY READING | ctx Enum -> add_enumerators on the masked line (comment blanked); enclosing then adds FillDeadlineRule Body; order matches |
| C9 4.7 | :106 | class block line gives class and declared function; head line gives class declared | HOLDS BY READING | strip_head drops `template<...>` and `public:`; read_type skips `final` and the base list |
| C9 4.8 | :122 | line in namespace with no enclosing head gives nothing | HOLDS BY READING | `using Ticks = int` -> Other; enclosing stops at Namespace |
| C9 4.9 | :134 | name in a comment/string never appears | HOLDS BY READING | make_view blanks comment and string interior; `s.log(...)` hits `.` in read_function -> Other; blank line 3 resolves to `}` (declared returns) |
| C9 4.10 | :143 | control blocks, lambdas, init lists name nothing of their own | HOLDS BY READING | lambda head has unclosed `sort(` so close_paren is npos -> Other; text_before reads through `b_{2}` so the constructor head is `Player`; `if` is a keyword |
| C9 4.11 | :158 | name reached two ways listed once with stronger kind | HOLDS BY READING | Finder::add keeps min kind; enum order Body < Comment (changed_names.h:20-25) |
| C9 4.12 | :167 | one-line enum gives its values | HOLDS BY READING | declared(): Enum && head branch calls add_enumerators on the text between `{` and its close |
| C9 4.13 | :177 | data member with parentheses is not a function | HOLDS BY READING | lines 2-7 each fail read_function (`void` keyword; `>` after `)`; `int` before first `(`; `]` after `)`; `=` before `(`) |
| D1 21 | test_bot_run.py:492 | server's Env backs up the key with reg.exe | HOLDS BY READING | bot_run.py:357-370 `@dataclass Env` default `run_reg = game_files.run_reg`; game_files.py:84-95 runs `reg.exe`; start_run passes env.run_reg to backup (bot_run.py:767); server.py:70 builds `bot_run.Env(out=...)`, default kept |
| D1 49 | test_bot_run.py:1242 | song.ini with no `[song]` section is refused by the writer | HOLDS BY READING | game_files.set_song_title (:320-333): `[other]` lines are skipped by _song_section_lines (:291), entries empty -> SettingNotFound before any write |
| D1 53 | test_chdis.py:97 | dis prints the listing up to the padding | HOLDS BY READING | Image.lines (chdis.py:147-163): decoded TINY_CODE by hand: call target 0x1020 -> `Demo$$Target`; dword at 0x2000 -> flt=100.0 i=1120403456; 0x2200 has no file bytes -> `(bss)`; 0x1008 in meta -> `str:'hi'`; `ret` then `int3` breaks. Comment column 58 checked: `;` sits at index 58 on all four commented expected rows. Capstone's own text formatting is taken as given (not read). |

## Side notes
- tests/test_precheck_pick.cpp:2-5 cite a removed plan with a `git show` pointer, and :462, :579 and the code comments in test_pick.cpp (:429, :577, :596) cite "D299" / the user's ruling as the source of rules. Comments, not evidence.
- test_precheck_pick.cpp:75-77 (not in my rows) pins 30/40/20 as typed numbers.
- test_precheck_pick_lines.cpp:103 (3.4) tests resolve_range (range.cpp), not the picker.
- test_chdis.py:102 pins capstone's exact text and the typed number 1120403456; it needs capstone installed.
- test_bot_run.py:492-493 only compares a default field to a function. It does not run reg.exe, and does not check the server's Env (server.py:70) or that start_run uses it (bot_run.py:767). The claim in its name holds by reading, the test covers just the default.
- test_precheck_pick.cpp:499-501 comment says no exe target lists the file in the fixture; true because the fixture has no CMakeLists.txt (raw() returns an empty optional, git_text.cpp:228).
- No wrong line numbers found in the 43 rows.

## Counts
SHOWN BY RUN 0 | HOLDS BY READING 43 | CONTRADICTED 0 | CANNOT TELL 0 | not reached 0
