Read docs/superpowers/plans/tasks/_rw-preamble.md first; it holds the rules.

# Task J5: the uitest harness frees the Preview's texture mid-frame

Task id: RW-J5. Base: the main tip your prompt gives. Branch `claude/rw-j5-harness-reset`, worktree `.claude\worktrees\rw-j5-harness-reset`; make it as the preamble says.

## The bug (found 2026-10-08, already diagnosed; don't re-diagnose)

`hydra_uitest --all` crashes about 1 run in 11 with 0xC0000005. The cause, caught under cdb with symbols:

- The Dear ImGui Test Engine runs each test step inside ImGui's end-of-frame hook (`ImGuiTestEngine_PreEndFrame`, `third_party/imgui_test_engine/imgui_te_engine.cpp:933`). That is after `run_frame` has drawn the app and before `ImGui_ImplDX11_RenderDrawData` sends the frame to the GPU.
- A test's first step is `reset_app` (`tests/ui/uitest_harness.cpp`, about line 267). It calls `preview->close()`, whose `release_targets` frees the highway texture, and then `h.app.reset()` destroys the renderer.
- When the previous test left the Preview tab open, this same frame has already recorded `ImGui::Image(srv)` (`src/ui/preview_tab.cpp:464`). `src/ui/preview_controller.h:91-93` already warns that `close()` must not run between that Image call and the end of the frame. The harness breaks that rule.
- When WARP (the software GPU) has caught up, binding the freed view faults inside WARP. WARP removes the device (0x887A0020). Every later Preview open then fails at `PreviewRenderer: object vs`, and a later font-atlas upload calls `UpdateSubresource(nullptr, ...)` and crashes the process.
- Stack at the root: `D3D10Warp!UMDevice::ShaderResourceViewReadAfterWriteHazard` < `d3d11!...SetShaderResources` < `ImGui_ImplDX11_RenderDrawData` < `uitest::Harness::frame` (uitest_harness.cpp:238). It reproduces on 1a0bdd8 too, so it predates the report windows.
- Hydra.exe itself never frees the Preview between drawing the image and rendering the frame. This is a test-harness bug only.

The diagnosis files are in `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\e40fb55d-4056-4249-a3d0-000442c5738a\scratchpad\crash-hunt\`, including `diag-harness.diff` (throwaway prints that logged the device-removed reason) and the loop scripts `run_slice.ps1` and `plain_loop.ps1`.

## The fix (harness only)

1. **Nothing a frame recorded is freed before it renders.** In `reset_app`, keep the old AppState (and anything that owns the Preview's renderer and textures) alive until the current frame has been drawn: move it into a "retired" slot on the Harness, and drop it at the start of the next `Harness::frame()` or right after `RenderDrawData`. If `preview->close()` must still run, it runs on the retired state at that point too. Check `Harness::stop()` for the same pattern (it runs at shutdown after the last frame, so it may be fine; say which).
2. **A lost device fails loudly.** After each frame's `RenderDrawData`, read `device->GetDeviceRemovedReason()`. If it isn't `S_OK`, fail the running test with a message that names the reason in hex and says the D3D device was removed, and stop the run there rather than letting later tests fail one by one. Words in a test failure message are developer text, not app text.
3. **No third_party edits.** Don't touch `third_party/`.

Write the fix so the cause is named in one comment at the retired slot, pointing at `preview_controller.h`'s warning.

## Proof

- Before the fix: run the slice (the first 18 scripts of `--all`: scan, analyze, cap-switch, preview, difficulty, analyze-on-preview, preview-path-overlay, preview-controls, preview-drain-box, preview-overlay-fit, preview-buttons-keys, scrub-hold, layout-drift, batch-strip-drift, settings-and-reports, dynamics, dynamics-reopen, stars) in a loop until it fails once, so you have a red run on your own build (the diagnosis saw about 4 in 44 plain runs). Keep its output line.
- After the fix: run the same slice at least 40 times with no failure. Give the counts.
- The device-removed check needs its own proof: show it fires (for example by temporarily reverting fix 1 and looping until WARP removes the device, then seeing the one clear failure instead of the cascade), then restore fix 1.
- Never `--all`, never the full hydra_tests suite.

## Owned files

`tests/ui/uitest_harness.{h,cpp}`, and the runner's main file under `tests/ui/` if `Harness::frame` or the run loop lives there.

## Return

`complete`, `branch`, `worktree`, `tip`, `report`, `questions`, `handoff`.
