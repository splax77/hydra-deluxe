# Doc screenshots

The pictures in `docs/images/` show Hydra Deluxe in the User Guide and on the differences page. One command takes all of them again:

```
pwsh -NoProfile -File tools\doc_shots.ps1
```

## What the command does

It runs the headless GUI test runner, `hydra_uitest`, on the shot list `tools/doc_shots/shots.txt`, and saves the pictures straight into `docs/images/`. The runner reads the checked-in test charts in `testdata/input`, with a scratch settings file and an empty database of its own ([ui-testing.md](ui-testing.md) says what the scratch settings hold). It never reads your song library or your settings, and the script passes it no library or settings file.

The shot list scans and analyzes the test charts once, then opens each screen and takes its picture. Before each picture it checks that the thing the caption talks about is on screen, with `expect-text` and `rect` lines. A missing element stops the run with `!! failed at line N`, so a wrong picture is never saved. Every wait in the list is capped by its `timeout` line.

Most pictures are cropped to one window or widget, with the `screenshot <file> | <ref>` form ([ui-testing.md](ui-testing.md) has the verb table). The main screen picture is the whole frame minus the version bar at the top, because the version bar names the build. The shot list marks that with a `rect` line directly before a `screenshot` line that has no ref; the script then keeps only the part of the frame below that rectangle. The comment at the top of `tools/doc_shots/shots.txt` says the same.

Last, the script runs `tools/doc_shots/shrink.py`. It does the crop above, then saves every PNG again with Pillow's `optimize=True`. That keeps every pixel and only makes the file smaller. It prints one `size:` line per file and a `total:` line, and fails when a file or the whole folder is over the user's limits; the constants at the top of `tools/doc_shots/shrink.py` hold them.

The script stops with a message naming `.\build_cpp.ps1 -Target hydra_uitest` when the runner is not built.

## When to run it

Run it before each release, so the pictures match the app that ships. Also run it after any change to a label or a layout that one of the pictures shows. Then open every changed picture and check it still shows what its caption says, and that no hover tooltip or version text crept in. Commit the changed pictures on a branch and get them reviewed like any other change.

The runner's scratch settings are not all defaults (they keep a smaller score range, for one). The first lines of `tools/doc_shots/shots.txt` open the settings popup and click `Reset to defaults`, then check that the settings button reads `Analysis settings: defaults`, so every picture shows default settings. The runner itself stays as it is.

## The one list of pictures

`tools/doc_shots/shots.txt` is the one list of the pictures. To add a picture, add its steps and its `screenshot` line there, run the command, and link the new file from the page. To drop one, delete its lines and its file. Nothing else in `docs/images/` should exist: every file there is a picture the shot list takes.
