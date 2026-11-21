# Coverage (T3)

Each file or folder in the plan's scope, with the scout that inventoried it. "Not inventoried" names what no scout read; finders still search it for copies.

| Where | Scout | Notes |
|---|---|---|
| `gates\` percall, progress, delete_rule, bash_windows_rule, media_rule, shell_output, common, `lib\hook_input.ps1`, status_append.ps1 | I1 | all read in full |
| `gates\` stall, wake, agent_stop, posture, prompt, waiver, cleanup_sweep.ps1, session_cards_start.ps1 | I2 | all read in full |
| `gates\` ref_judge, review_rules, review_dispatch, publish_gate, ruling_rules, publish.ps1, ref_gate.sh, install_ref_gate.ps1, sync_settings, settings.fragment.json, `.derive-once-gate` | I3 | all read in full; every other file in `gates\` is in I1 or I2 (listing at 21:05) |
| `tools\session_cards\` (10 Python files, writer-brief.md, hand_tags.txt) | I4 | all read in full; the made-up test transcript in `fixtures\` not opened |
| both `CLAUDE.md` files, agent-rules.md, brief-preamble.md, model-postures.md, the five agent files, `gates\README.md` | I5 | all read in full |
| `docs\agents\` derive-once-review, doc-review, handoff, integrate, audit-method, owner-map, premise-map, this-machine, rulings\README, issue-tracker, triage-labels, domain, doc-screenshots, public-doc-style | I6 | all read in full |
| `docs\agents\` ui-testing, clone-hero-mcp, test-rubric, test-rubric-examples, and the other pages not in I5 or I6 | I7 | all read in full |
| memory notes (MEMORY.md and 53 files), both skills, `.claude\commands\handoff.md` | I8 | all read in full; `memory\archive\` and `skills\synced\` out of scope |
| `gates\tests\` (24 test files, three .ps1 tests, lib\harness.ps1, replay.py, replay_extract.py, shell_output_sizes.py, probe_results.md) | I9 | all read in full, across the hooks-removal merge (row I9-192); reflog_moves.json removed before it was opened |
| Hydra `tools\`: check_pointers, check_handoff, doc_tokens, build_slot, hydra_ui_mcp, find_cmake, new_worktree, capture_window, doc_shots.ps1, fake_hydra_live.py to line 60, precheck constants | I10 | read in full |
| Hydra `tools\`: ch_mcp, ch_probe\constants.py, capture\*.ps1, doc_shots (shots.txt to about line 30, shrink.py), fake_hydra_live.py from line 60, precheck, replay*.cpp, mutate, tools\test_* | I10b | headers, constant blocks and their comments only |
| **Not inventoried** | none | function bodies of ch_mcp, ch_mcp\tests, ch_probe beyond constants.py, agent\*.ts beyond their top constants, precheck check bodies, replay function bodies, mutate bodies and mutate\test_*.py, test_hydra_ui_mcp.py, tools\test_* beyond their heads, shots.txt after about line 30; other tools\ scripts I10 lists in its report section 5 |
