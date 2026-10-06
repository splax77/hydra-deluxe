# Handoff: SP end marks need triangles, not boxes (2026-10-06)

## Where things stand

The user asked why the Preview showed Star Power ending exactly on a note when it really ends between notes. The Preview was not working the SP end out again. The floor tint, the drain box and the Paths tab's backend timings all read the stored deact tick. The trouble was drawing. A gem is drawn centred on its hit line, so its top half covers about 26 ms of highway. In The Ministry of Lost Souls, activation 4, SP ends 26 ms after a kick + blue cymbal (m194.4.288 counted, end at m194.4.318, next chord m194.4.384 uncounted). That gem hid the tint's edge.

I made six mock designs, painted on the user's own screenshot. The user picked **design C**: a crisp edge across the floor at the SP end, drawn under the gems, plus a small **triangle on each rail pointing inward** at the same spot. The mocks are in `docs/handoffs/2026-10-06-sp-end-mocks/sp_end_mocks.html`. Open it in a browser; it is self-contained. Design C is the fourth mock.

That work merged to `main` as **1818f1c** (decision D81, not pushed). The full suite passed once after the merge: 1,147 unit tests and 64 of 64 GUI tests. But the renderer had no triangle shape, so the build agent drew the rail markers as **boxes**, and I merged that without showing the user. The user rejected the boxes: "that isn't the design i approved. if triangles don't exist currently, figure out how to make them." The rejected look is in `burnout-boxes-rejected.png` in the same folder.

The user then said to stop all work and write this handoff. Nothing is running.

## What the next session does

Replace the box notches with triangles that match mock C. Then **show the user screenshots before any review or merge**. The look is the user's call, and the boxes went wrong exactly because that step was skipped.

**Adding the triangle.** Shapes are small vertex lists in `src/render/obj_loader.cpp` (`make_flat_quad`, `make_box`), named in `MeshId` in `src/render/highway_draw.h` and uploaded once in `src/render/preview_renderer.cpp` around line 386. A triangle is one more of those. Mirroring with a negative scale may flip the winding and get culled, so either add a left and a right mesh or check the cull mode.

**Placing it.** The notches are emitted in `build_highway_draws` in `src/render/highway_draw.cpp`. They use `railing_x` (the one owner of the rails' X extent) and `TrackState::sp_active_ends` (the one owner of where visible SP windows end). Keep both owners; only the shape and size change. In the mock, each triangle sits on its rail, centred on the SP-end line, pointing inward toward the lanes. It's about 0.7 of the rail's on-screen width in both width and height, and it's small, not a post. An upright triangle facing the camera will likely match the mock better than one lying flat on the rail, because the camera looks down the highway and a flat one would squash into a sliver. Try both if unsure and show the user.

**Colour.** The boxes came out pale cyan under the lighting, not the full SP teal. Check whether the material can be drawn unlit or brighter, so the triangle reads in the same colour as the edge.

**Config.** The current keys in `assets/preview/3d-config.json` (hydra section) are `sp_end_edge_depth` 0.1, `sp_end_notch_size` 0.18 and `sp_end_notch_rise` 0.06. The edge depth stays. The notch keys will need new values, and maybe new names, for the triangle. Update `src/render/preview_config.{h,cpp}`, `tests/test_preview_config.cpp`, `tests/test_highway_draw.cpp` (the notch cases check boxes now), the D81 entry in `docs/audit/2026-10-03-fix-decisions.md` (it names the three values), and the UserGuide's Preview sentences if they say "box".

**Seeing it.** Build `hydra_uitest` in the worktree and run the scrub script: `hydra_uitest --script docs/handoffs/2026-10-06-sp-end-mocks/burnout-scrub-script.txt --shots <folder>`. It opens Burnout, analyzes it, and scrubs to 48.3, 48.6 and 48.75 s, just before activation 1's SP runs out. The scrub slider takes seconds (`type **/##scrub | 48.6`). In that spot SP ends right on a kick chord, so the floor edge hides under the kick bar and the rail markers are what show. Also try to get a frame where SP ends between notes, like the Ministry case, so the edge shows too.

**Then the usual gate.** Get the user's yes on the screenshots, then a derive-once review by a fresh Sonnet agent (`docs/agents/derive-once-review.md`), merge with `git merge --no-ff --no-commit` followed by `git commit -m ... --trailer ...` (merge itself has no `--trailer` option), and run the full suite once.

## Loose ends

A worktree is ready and untouched: `.claude/worktrees/sptri` on branch `claude/sptri`, forked from `main` at 1818f1c. Use it or remove it.

`build-cpp\Release\Hydra.exe` was not relinked after the merge, because Hydra was open from that folder. The test executables were rebuilt. Close the app and run `.\build_cpp.ps1` to see the current state.

The reviewer left three notes that block nothing. A small helper next to `toggle_on_after` could own "a span ends here", so the Toggle reading stays in one place. An old plan, `docs/superpowers/plans/2026-09-27-preview-sp-drain-box.md`, still has `railing_x_width` in its code listings. A scan row for `tests/test_single_owner.cpp` could keep `railing_x_width` inside `railing_x` and the config loader.

The installed app's database (`C:\Program Files\Hydra\hydra.db`) holds a stale 1.8.2 record for The Ministry of Lost Souls. The numbers above came from the dev build's database in `build-cpp\Release`.

This handoff and its folder are not committed.

## Journal state at handoff

Subagents active in the last 30 minutes and not finished:
  agent-a5168e77ce8d8ec25: unfinished, last tool call SubagentHandback, touched 23:56
  agent-ac6947ca5b38eb3d1: unfinished, last tool call SubagentHandback, touched 23:57

Both had handed back their final reports: the SPEDGE build agent and its derive-once reviewer, which signed CLEAN at 798f257. Neither has work left.
