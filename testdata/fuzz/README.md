# Saved fuzz inputs

Each folder here holds inputs that once broke one parser. The folder names are the fuzz targets in `tests/fuzz/harness.h` (`kTargets`). The test case "fuzz regressions" in `tests/test_fuzz_regressions.cpp` feeds every file in them through the same harness body the fuzz exes run, so a saved file stays a test for good. It needs no code.

To keep a crash, copy the fuzzer's `crash-<hash>` file into the target's folder under a short name that says what it broke, for example `mid/null-track-read.mid`. Commit it together with the parser fix. A file whose parser is not fixed yet turns the suite red, so it stays out until the fix lands. Which names the test skips (the `.gitkeep` files and this README) is decided in `is_saved_input` in that test file.

How to run the fuzzers is in "Fuzzing the file parsers" in `docs/development.md`.
