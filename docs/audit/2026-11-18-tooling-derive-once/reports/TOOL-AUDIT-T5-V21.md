# TOOL-AUDIT-T5-V21 (verifier of F21). All claims "by reading". G = C:\Users\Patrick\.claude\gates, H = Hydra repo.

## K1 allow phrases: CONFIRMED
- Owner: G\review_rules.py:51 `LIBRARY_ALLOW_TEXT = "allow library run"`, :54 `KEY_ALLOW_TEXT = "allow key run"`. Comment :50 "a trailing . or ! is allowed"; the regex is case-insensitive.
- percall.py holds no literal (grep found none); it uses the constants. Copies of the literal found: G\README.md:13; H docs\agents\agent-rules.md:14, brief-preamble.md:49, clone-hero-mcp.md:29, this-machine.md:91, audit-method.md:125; H tools\ch_mcp\server.py:148, 466, 477. Matches the finder.
- Situation where they differ: docs say "exactly"; the owner accepts case changes and one trailing . or !. Not a conflict.
- Correction: reviewer.md lives in C:\Users\Patrick\.claude\agents\, not G\agents (G has no agents folder).
- Ruling: rulings\review-and-hooks.md:16 (Confirmed) "i want to be able to give explicit approval for a full library run." Covers the library phrase only, not the key phrase; it does not name a split. Line 47 is under Unconfirmed, as the finder said.
- Splits: none.

## K8 marker files: CONFIRMED
- H\.derive-once-gate says the rule is "DeriveOnceGatedPath in the user-scope hook (hooks\lib\derive_once_rules.ps1)"; H\.doc-review-gate says "(hooks\lib\doc_review_rules.ps1)". C:\Users\Patrick\.claude\hooks exists but holds only `state`; no `lib`. The live owner is G\review_rules.py (BUILTIN_CODE :22, markers :18). Two stale owner pointers.
- Splits: none (stale pointer, not two live answers).

## K2 "every markdown file": CONFIRMED
- Owner G\agent_stop.py:163 `DOC_SUFFIXES = (".md", ".html", ".htm")`; :172 also takes the doc-review marker path. Docs: derive-once-review.md:151 and :202, doc-review.md:29, ~\.claude\agents\reviewer.md say "every markdown file the range changes" (reviewer.md and doc-review.md confirmed by grep text; derive-once-review.md lines matched the phrase search). The docs name a narrower set than the code. Situation: a changed .html/.htm file in the range, read by a reviewer that lists only .md files.
- Splits: one, doc wording vs agent_stop.py:163. No ruling names it.

## K6 realapp.jsonl: CONFIRMED
- H\tools\hydra_ui_mcp.py:81 and G\review_rules.py:49 both hold the literal. Each comment names the other as the contract (hydra_ui_mcp.py:78-80 "The hooks repo's ref gate reads it"; review_rules.py:47-48 "Hydra's tools/hydra_ui_mcp.py owns its name"). G tests use review_rules.REALAPP_FILE, so no test ties the two names. Equal today.
- Splits: none today.

## Partial
- I9-129: review_rules.py:22 is confirmed wider than "src/ and ci.yml" (src, tests, tools, third_party, installer, .github, CMakeLists.txt, CMakePresets.json, build_cpp.ps1). I did not find the inventory row text, so what the row says is unchecked.
- Sign-off: agent_stop.py:105-109 confirmed (BLOCKED found first; no line records CLEAN with a problem text). Ruling line :19 not re-read.

## Not reached
Publish limit 20/2000, picker limits 30/40/20, D49 lists, rulings README wording and date, public-page list copies, ref-gate K1/K2, handoff checker, README :53/:57 note, test-fixture copies.
