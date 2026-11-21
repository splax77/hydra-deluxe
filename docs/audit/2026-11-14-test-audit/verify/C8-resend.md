Agent: a9fb37a804c7f242c (T5-V-RESEND-C8, verify re-send for slice C8)

# Verify re-send, slice C8

Records checked: 4 of 4 from the first verifier's "Not reached" list. Agree with 4 verdicts, dispute 0. None is a Keep, none proposes a saved expected-output file. Every mark is "by reading": I ran nothing, including git.

What I opened: `tests/test_precheck_check1.cpp` 1-150 (fixture, helpers and cases 1.1 and 1.2); `tests/test_precheck_check24.cpp` 1-219 (fixture, `Run`, cases 2.3 and 2.4); `tools/precheck/views.cpp` whole; `tools/precheck/braces.cpp` whole; `tools/precheck/check_4.cpp` whole; `tools/precheck/check_rows.cpp` 1-205 and 205-280; and the records at `records/C8.md` 21-57 and 169-200. I grepped the repo for `innermost`, `codelex::split` and `PCX-J2`. I did not open `codelex` itself, `single_owner::scan_file` or `parse_rules_file`; where an answer needs them I say "record only".

Question 2 for these four: no `src/` caller. `make_view` is called by `check_2.cpp`, `check_4.cpp` and `ViewCache::get`; `Braces` by `definitions.cpp`, `records.cpp`, `check_5.cpp`, `changed_names.cpp` and `test_pick.cpp`; `run_check_4` only through `kChecks`. I agree with the records. Question 3 for these four: each record writes "claim not checked", names the real input it needs (real tracked source files, a real compiling file's braces, the real `tests/single_owner/rules/tests.json`, the real rule files at HEAD) and an independent owner (the compiler, the scan itself). That meets the brief; none says "not an engine claim".

## Records

### 1.1 views (`test_precheck_check1.cpp:121`)

Agree: Investigate, claim not checked only. I worked each expectation through `make_view` (`views.cpp:19-42`). Line 125: a comment byte becomes a space and a line break stays, so `// note` after `1;` and its one space gives eight spaces, and the block comment `/* x` newline `y */` gives four spaces, the break, then four plus the original space. Line 126: the string body `a{b` becomes three spaces and the char `'}'` becomes `' '` (`views.cpp:32-35` keeps the quotes). Line 127: one start at 0 plus four breaks is five lines. Lines 128-131: `line_of` is `upper_bound` over the starts, so offset 0 is line 1, the first break is still line 1, and `const` and `char c` are lines 2 and 4. Line 132: `View::line` cuts `[starts[1], starts[2]-1)`. Lines 135-136 read right only if `codelex::split` reads `'#'` as a Python string and `# note` as a comment; I did not open `codelex`, so that half is record only. The record's per-line mutations hold: a `lower_bound` turns 128 red; a skipped Comment branch turns 125 red; dropping the push of 0 turns 127 red.

Two gaps the record does not list, both coverage notes and not faults. The unclosed-literal branch (`closed ? p.end - 1 : p.end`, `views.cpp:34-35`) is never reached, because every string and char in the test is closed; a change that always took `p.end - 1` stays green. The empty literal `""` (the `open + 1 >= p.end` continue at `views.cpp:33`) and a prefixed literal are also never given.

### 1.2 braces (`test_precheck_check1.cpp:139`)

Agree: Investigate, claim not checked only. I numbered the input `a{b{}c{}}{` by offset: opens at 1, 3, 6 and 9, closes at 4, 7 and 8. Every expectation matches `braces.cpp:5-29`: `close(1,100)` is 8; `close(1,8)` is empty because `it->second >= limit` (`braces.cpp:22`), so turning `>=` into `>` is red at line 143; `close(3,100)` is 4; `close(9,100)` is empty because the `{` never meets a `}`; the parent of 3 is 1 and of 1 is `kTop`; the children of 1 are 3 and 6 and of `kTop` are 1 and 9. The record's claim that `innermost` has no case holds: my grep of the repo for `innermost` finds it only in `braces.*`, `changed_names.cpp` and `test_pick.cpp`, no test file. One more gap the record does not list: a `}` met with an empty stack (the `!stack.empty()` guard at `braces.cpp:13`) is never given.

### 2.3 check 4, a source-dir macro line prints once (`test_precheck_check24.cpp:148`)

Agree: Investigate, claim not checked only. I followed the run end to end. The fixture row is in `main` and unchanged in `feature`, so `AddedKeys::against` (`check_rows.cpp:76-87`) does not count it as added, and no row-check item prints. The added line `std::ifstream in(std::string(HYDRA_SOURCE_DIR) + "/src/app/x.cpp");` matches check 4's own regex at `check_4.cpp:36` (`ifstream`, then `[^;]*`, then `.cpp"`). The row's own hit gives an item of kind `E`, because `row_kind` (`check_rows.cpp:241-249`) returns `E` for a test file whose owner holds `tests/source_tree.h`. `run_check_4` (`check_4.cpp:95-101`) puts that hit line in `e_hit_lines`, and `run_check_4_spellings` skips it (`check_4.cpp:61`). The text the test types, `answers "Which test reads the source tree?": <line>  -- the scan row in tests/single_owner/rules/tests.json gives this question to sourcetree::root in tests/source_tree.h`, is what `check_rows.cpp:224-227` builds (`row_why` plus `answers "<question>": <hit text>`). Whether `h.text` is the raw or the trimmed line, I did not read in `scan_file`; record only. The record's mutations hold: dropping the skip prints a second item and the vector comparison goes red; giving `row_kind` another letter leaves `e_hit_lines` empty and the spelling's wording prints, also red. The record's point that the expected text is built from the fixture's own row, not the real file, is right: the owner and question strings at test lines 156-166 are typed in the test. I confirmed `PCX-J2` appears nowhere under the repo outside `docs/audit` except test lines 161-162, so the record's Q5 holds.

### 2.4 check 4, a new rule row with no examples or a variable exemption (`test_precheck_check24.cpp:188`)

Agree: Investigate, claim not checked only. I ran the regex by hand. `variable_exemption` (`check_4.cpp:42-45`) against the pattern `\bscore(?!(base|combo)\s*\.)` matches `(?!(base|combo)\s*\.`, so the message `(` + match + `...)` is `((?!(base|combo)\s*\....)`, which is exactly the text typed at test line 216. The bare row has both lists empty, so `check_4.cpp:78-84` prints must-match first and must-not-match second at line 4, both with the typed reason; its pattern `\bplanted\b` does not trip the exemption regex. The exempting row has both lists filled, so only the exemption item prints, at line 20. `AddedLines::whole({})` on test line 203 leaves no file for the spelling loop, so the count of three holds. The record's per-line mutations hold: dropping either list check turns 209 or 212 red; dropping the exemption turns 215 and 218 red.

Two notes the record does not make. First, `variable_exemption` also accepts a lookbehind (`<?`) and the optional group forms, and only one lookahead shape is pinned, so a change that stopped accepting the lookbehind stays green; a coverage note. Second, the `Fixture` git repo is built only so `Run` can be constructed; the case reads no file from it. That is not a fault.

## Splits

No new split. The records' own splits stand. Two additions to the first verifier's S1 (the two blankers): `codelex::split` has a third caller, `check_5.cpp:164`, which splits `v.raw` itself instead of reading a `View`. I did not open that code, so I do not know if it repeats the string-blanking step. Record only.

## Not reached

None. All four records were read against their test lines and the production code they call.
