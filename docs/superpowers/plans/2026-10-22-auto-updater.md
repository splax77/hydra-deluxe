# Hydra checks for its own updates (2026-10-22)

Task id UPD-P1. Planner: Fable. Nothing here is built yet; every task below waits for the user's yes (agent rule 3). A new on-screen strip, a new setting and a network call with no click are user-facing changes, so the exact words and the defaults go to the user first, as the numbered decisions below.

Research came from `updater-findings.md` in the session scratchpad. Every claim this plan relies on was checked in the code or at the source; the ones that could not be are listed at the end under "Not confirmed".

## What the user gets

Once a day, when the installed Hydra Deluxe starts, it asks GitHub for the latest release. If that release is newer than the running version, a one-line strip appears under the toolbar: "Hydra Deluxe 2.11.0 is available." with three buttons, `Update`, `Skip this version` and `Later`. Clicking `Update` downloads the installer in the background (the strip shows the progress), checks the download against a signature built into Hydra, and then starts the installer. Windows shows its one UAC prompt (the installer needs admin rights today, `PrivilegesRequired=admin` in `installer/hydra.iss` line 46). Hydra closes, the installer runs with its progress window, and Hydra starts again. The first start after that says "Updated to Hydra Deluxe 2.11.0." once. Nothing is ever installed without the click. A dev build, a worktree build or a copy of the exe outside the installed folder never checks and never offers.

## Recommendation: build it in, with WinSparkle as the alternative

Recommended: a built-in updater. It is one GET of `https://api.github.com/repos/splax77/hydra-deluxe/releases/latest` over the WinHTTP code Hydra already has (`src/net/dmbot_client.cpp`), one file download, one signature check with the Windows crypto API Hydra already links (bcrypt, `CMakeLists.txt` line 345), and one `ShellExecuteEx` of the installer with Inno's silent switches. The GitHub release page the README already points to (`README.md` line 28) is the feed, so there is nothing new to host. The strip is drawn by Dear ImGui like every other notice, so it can be driven by `hydra_uitest` and the MCP server, and the canned-fetcher seam the tests already use covers the network. The checked facts: the endpoint answers unauthenticated with the `-setup.exe` asset, its size, its `browser_download_url` and a `sha256:` digest, and it skips drafts and prereleases (findings, checked by a real call).

The honest alternative is WinSparkle (MIT, C API, EdDSA signatures, works with Inno installers). It gives a finished dialog, a download window and the signature check for free, and it is used by many Windows apps. The costs are real, though. It needs an "appcast" XML feed hosted somewhere Hydra can fetch, which GitHub releases do not produce on their own, so the release process grows a second artifact to keep right. Its dialogs are Win32 windows with their own look, outside Dear ImGui, so `hydra_uitest` cannot drive them and the real-app check is by eye only. And its one sharp edge is that with no public key set it installs unsigned updates and only logs "Using unsigned updates!" (findings, checked in its `src/updatedownloader.cpp`), so a build mistake fails open. Hydra's rule is to fail closed. If the user prefers WinSparkle anyway, tasks T2b, T3a and T3b below are replaced by its integration and the signing scheme becomes its Ed25519 one; T1, T4 and the decisions still apply.

## The signature scheme

Rule first: Hydra never runs a downloaded installer unless a signature made with the project's private key checks out against the public key compiled into Hydra. HTTPS and GitHub's own SHA-256 digest guard against corruption and a broken download, not against a hijacked GitHub account (the Notepad++ 2025 case in the findings). Both checks run; the signature is the one that matters.

Windows CNG has no Ed25519. The named-curve list (`learn.microsoft.com/en-us/windows/win32/seccng/cng-named-elliptic-curves`, read 2026-10-22) has `curve25519` for the ECDH and ECDSA providers, but no Edwards curve and no EdDSA; `curve25519` there is the key-agreement curve, not a signature scheme. So the choice is a small library for Ed25519 (Monocypher 4, single `.c` file, BSD-2 or CC0, `crypto_eddsa_check`; its default EdDSA uses BLAKE2b and real Ed25519 needs its optional SHA-512 file) or the signature scheme CNG does offer, ECDSA on the P-256 curve with SHA-256.

Recommended: ECDSA P-256 with SHA-256 through bcrypt. The reason is code: Hydra already links bcrypt into `hydra_core` and already hashes with it (`src/app/analysis.cpp` line 71, `src/app/preview_source.cpp` line 58), so the verifier is about forty lines of calls to `BCryptImportKeyPair`, `BCryptHashData` and `BCryptVerifySignature` and adds no third-party code to review, no licence line and no build step. The signing side is PowerShell with .NET's `System.Security.Cryptography.ECDsa`, which is on every Windows machine and signs in the IEEE P1363 form (`r || s`, 64 bytes) that `BCryptVerifySignature` reads. The security is the standard every TLS connection rests on. The one thing ECDSA asks that Ed25519 does not is a good random number at signing time; .NET's signer takes it from Windows, and the release machine is the user's own PC, so that holds. Monocypher stays the fallback if the ECDSA tooling turns out awkward in T2b; the verifier's seam (a function that takes the file, the signature bytes and the public key bytes) does not change.

The release-time steps, each owned by T2b and written into `docs/development.md` so they are done the same way every release:

Key generation happens once, with `tools/release_keygen.ps1`. It creates a P-256 key with .NET, prints the 64-byte public key as a C++ byte array for `src/app/update_key.h`, and writes the private key as a per-user DPAPI-protected file (PowerShell's `Export-Clixml` of a `SecureString`, which only the same Windows user on the same PC can read). The file lives outside the repo at `%USERPROFILE%\.hydra-release\update-signing.xml` (decision 12 below). It is never committed, never put in CI (CI builds and tests only; `.github/workflows/ci.yml` has no release step, checked), and the `.gitignore` gets no entry because the path is outside the tree. The user keeps a backup of the file in their own password store; a lost key means a new key and one last release that users must install by hand, which the plan says plainly in the docs.

Signing happens once per release, with `tools/release_sign.ps1 -Installer <path>`. It hashes the installer with SHA-256, signs the hash with the key, and writes `<installer>.sig` beside it, 64 raw bytes. `installer/build_installer.ps1` gets a `-Sign` switch that calls it at the end so the two files leave the build together.

Uploading: the release is created as a draft with both assets, then published, so there is never a moment where `releases/latest` names a release with a missing asset: `gh release create vX.Y.Z --draft <setup.exe> <setup.exe.sig> --notes-file <notes>` and then `gh release edit vX.Y.Z --draft=false`. The posted text goes through the doc-review gate as today.

What Hydra does when anything is missing or wrong: it refuses. No `-setup.exe` asset, two of them, no `.sig` asset, no `digest` field, a download larger than the asset's stated size, a digest that does not match, or a signature that does not verify, each ends the update with the sentence in decision 9 and deletes the download. Hydra never falls back to "install anyway".

## Decisions for the user

Each one names the recommendation and the exact text. No number appears in this plan that is not in one of these.

1. Where the notice lives. Recommended: a one-line strip under the toolbar, in the same place the rules-file warning sits today (`src/ui/library_toolbar.cpp` lines 150 to 157, under `render_status_line`), drawn in the normal text colour, not the warning colour. Alternative: a modal box at start. Not recommended; the forums in the findings say players want to be told, not stopped.

2. The notice's wording. Recommended: `Hydra Deluxe 2.11.0 is available.` followed on the same line by the three buttons. The version is the release's tag with its `v` removed. Hovering the version shows the release's notes as plain text (the `body` field of the release JSON, drawn with `TextUnformatted`, so nothing in it can run).

3. The buttons. Recommended: `Update`, `Skip this version`, `Later`. `Update` starts the download. `Skip this version` writes the version to the INI and hides the strip for that version only; a newer release shows again. `Later` hides the strip until the next check. Alternative: drop `Later` and let the strip stay. Not recommended; a strip that cannot be dismissed is the "repeated nag" the forums complain about.

4. The progress display. Recommended: the strip itself changes to `Downloading Hydra Deluxe 2.11.0...  12.3 of 33.2 MB` with a progress bar filling the rest of the line and a `Cancel` button; then `Checking the download...` while the digest and signature run; then `Starting the installer...`. The numbers are megabytes to one decimal, bytes divided by 1,000,000, the same unit the GitHub page shows. Cancel deletes the part-file and returns the strip to decision 2's state.

5. The setting. Recommended label: `Check for updates`, a checkbox, with the help marker text `Once a day, when Hydra starts, look for a newer release on GitHub. Nothing is downloaded or installed until you click Update.` Recommended default: on. The row lives in a new fourth group of the Analysis settings panel named `App`, after `Paths kept` (`src/ui/settings_bar.cpp` line 148), and it is never greyed out by the batch lock, because it is not part of a result's profile and `Reset to defaults` does not touch it. Alternative: default off. Not recommended; an updater that is off by default updates almost nobody. Second alternative: a separate place for the row. There is none today, and a new panel for one checkbox is more than it is worth; the `App` group name says why it sits apart from the analysis groups.

6. How often. Recommended: at most once per 24 hours, counted from the last finished check, stored in the INI, so a user who opens Hydra ten times a day makes one request. The `Check now` button in the same settings row ignores the 24 hours. The number comes from the findings' norms (MSIX's default is 24 hours; WinSparkle's floor is 1 hour) and GitHub's unauthenticated limit of 60 requests per hour per IP, which 24 hours keeps far from.

7. Timeouts and the size cap. Recommended: the download reuses the four WinHTTP timeouts `dmbot_client.cpp` already sets at line 171 (resolve 15 s, connect 20 s, send 30 s, receive 120 s), so a transfer that moves no bytes for 120 s fails. The size cap is the asset's `size` field from the release JSON; a body that goes past it is refused. No new number.

8. What a silent failure looks like. Recommended: the automatic daily check says nothing on any failure (no network, a 403 or 429 from GitHub, unreadable JSON); it tries again next day. A failure after the user clicked `Check now` or `Update` shows in the strip through `plain_error`, with the details under it through `plain_error_detail`, like every other job.

9. The refusal sentence. Recommended, for every refusal listed under "The signature scheme": `Hydra couldn't verify the update it downloaded, so it was not installed. Try again later, or download the release from GitHub yourself.` One sentence, one `ErrorKind` (`UpdateUnverified`); the raw detail names which check failed.

10. When the user declines the UAC prompt. Recommended: Hydra keeps running and the strip reads `The installer didn't start, so Hydra wasn't updated.` with the three buttons of decision 3 back, so `Update` tries again. The downloaded installer is kept for that retry and deleted at the next start.

11. The "updated" message. Recommended: on the first start after any version change, the strip reads `Updated to Hydra Deluxe 2.11.0.` with one `OK` button, and the INI's `last_run_version` is set to the running version. This also fires after a manual install, which is right: the message is about the app, not about the updater. No installer flag is needed, so `main.cpp` still takes no arguments in a shipped build.

12. Where the private key lives. Recommended: `%USERPROFILE%\.hydra-release\update-signing.xml`, DPAPI-protected to the user's Windows account, outside the repo and never in CI. The user backs it up in their password store.

13. The public-doc wording. Recommended, for the README under installing, after the SmartScreen sentence: "Hydra Deluxe checks for a newer release once a day when it starts and tells you under the toolbar. Click `Update` to download and install it; nothing is installed until you do. To stop the check, untick `Check for updates` in `Analysis settings`." The User Guide gets the same facts in its `Toolbar` section and a `Check for updates` row in the settings table, plus one sentence under "The first start after updating". The release note for the version that ships this says it in one paragraph. All of it in `docs/agents/public-doc-style.md` voice.

14. Which builds check. Recommended: only a Hydra.exe that sits in the folder the installer registered. The one rule is `app::installed_by_setup()`: the exe's folder (`app::exe_dir()`) equals `InstallLocation` under the Inno uninstall key for Hydra's frozen AppId (`HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{638FCDD7-88E0-438D-9C52-388C6C9CF4E3}_is1`), compared case-insensitively with trailing separators removed. A worktree build, the test runner and a copy of the exe on the desktop all read false and never check. Alternative: a compile-time flag in the ship preset. Not recommended; a shipped exe copied out of its folder would still check, and a dev build with the flag set by mistake would offer updates over itself.

## How it works, piece by piece

One owner per rule, named here so the executors do not each write their own.

The transport. `http_get` and its async WinHTTP machinery move from `dmbot_client.cpp` into a new `src/net/http.cpp` with two entry points: `http_get(url, cancel)` as today, and `http_download(url, path, size_cap, progress, cancel)`, which writes to `path` in chunks, updates an atomic byte counter the UI reads each frame, polls the same cancel flag, and refuses a body past `size_cap`. Both share one session setup, one timeout line and one status-code rule (only 200 counts, as today, line 216). The test seam `set_fetcher` moves to `http.h` and gains a download twin, `set_downloader`; a harness installs both. `dmbot_client.cpp` keeps its parsers and calls the shared transport. The raw detail strings stop saying "the leaderboard" and take the caller's name for the host (`"the leaderboard"`, `"GitHub"`), so one `fail` builds every network detail line.

The version rule. `src/core/app_version.h` gets `parse_version(string) -> optional<AppVersion>` (three whole numbers, a leading `v` allowed, anything else empty) and `AppVersion::operator<`. `HYDRA_VERSION` parses through the same function. "Newer" means strictly greater, so an equal or older release never offers (the rollback rule in the findings). This is the only place two versions are compared.

The release reading. `src/app/update_release.{h,cpp}` parses the `releases/latest` JSON into `UpdateRelease {version, notes, installer_url, installer_size, digest, sig_url}` with the refusal rules from "The signature scheme": exactly one asset named `*-setup.exe`, exactly one named `*-setup.exe.sig`, a `digest` starting `sha256:`, a tag that parses. Anything else throws `KindedError(ErrorKind::UpdateUnverified, ...)` with the raw text naming the missing piece. It lives in `hydra_dm` with the rest of the GUI-only network code, so `hydra_batch` and `hydra_fillcompare` keep linking without WinHTTP (`CMakeLists.txt` lines 410 to 420 and 530 to 532).

The verifier. `src/app/update_verify.{h,cpp}` has `sha256_file(path)` (bcrypt, like the two hashers above) and `verify_update(path, digest_hex, sig_bytes, public_key_bytes)`, which checks the digest first and then `BCryptVerifySignature` with `BCRYPT_ECDSA_P256_ALGORITHM`. The production key is one constant in `src/app/update_key.h`; the function takes the key as a parameter so tests use a test key. It lives in `hydra_core` (bcrypt is already there). Either failure throws `UpdateUnverified`.

The installed-exe rule. `app::installed_by_setup()` in `src/app/config.cpp` beside `exe_dir()`, as decision 14 says, with a test seam `set_installed_for_test` through the `ScopedHook` pattern in `tests/scoped_hook.h`.

The job. `src/ui/update_job.{h,cpp}`: `UpdateCheckJob` (one GET, parse, compare) and `UpdateDownloadJob` (download to `%TEMP%\HydraDeluxe-update\<asset name>.part`, rename on finish, fetch the `.sig`, verify), both `ResultJobBase` so their errors already come through `plain_error` and `plain_error_detail` (`src/ui/job_base.h` lines 107 to 115). `AppState` holds an `UpdateState` with the phase (none, offered, downloading, verifying, launching, declined, failed, updated), the release, the progress counter and the strip's text. The launch is `ShellExecuteExW` with the plain `open` verb (never `runas`; see "Ruling: how Hydra starts the installer" below), with `SEE_MASK_NOCLOSEPROCESS` and parameters `/SILENT /SUPPRESSMSGBOXES /NORESTART /NOCANCEL /LOG=<temp>\HydraDeluxe-update\setup.log`. Hydra stays open and watches the installer's process. Inno raises the UAC prompt itself. If the user accepts, the installer's `CloseApplications` (default on) asks Hydra to close through Windows Restart Manager, Hydra saves its INI and exits, and the installer relaunches Hydra as the original user. If the installer's process ends while Hydra is still running, the update did not happen (the UAC decline of decision 10, or any other early exit), and the strip shows decision 10's text.

The settings. Four INI keys in the one key table in `src/app/config.cpp`: `check_updates=1`, `skipped_update_version=`, `last_update_check=` (an ISO-8601 UTC timestamp, written by the one function that finishes a check) and `last_run_version=`. `Settings::with_analysis_defaults` and `Reset to defaults` leave them alone. The "should a check run now" rule is one function, `update_check_due(settings, now)`: on, installed, and more than 24 hours since `last_update_check` or no timestamp yet.

The installer. `installer/hydra.iss` line 82 loses `skipifsilent`, so a silent install also runs the `postinstall` line and relaunches Hydra as the original user (Inno's `postinstall` entries run as checked under `/SILENT` unless `skipifsilent`, and carry `runasoriginaluser` by default). A manual silent install relaunches too, which is harmless and noted in the file's design notes. The leftover download folder is deleted at the next start, after the "updated" message.

## Ruling: how Hydra starts the installer (user, 2026-10-22)

T4 found, from Inno Setup's source, that `runasoriginaluser` (the default for `postinstall` entries) only works when Setup raised its own UAC prompt: `Setup.SpawnClient.pas` runs the entry with Setup's own rights when no spawn server exists, and the spawn server exists only after Setup re-launches itself elevated (`NeedToRespawnSelfElevated` in `Setup.SpawnServer.pas`). A `runas` launch from Hydra elevates Setup first, so dropping `skipifsilent` would relaunch Hydra as administrator. The user chose, in their words: "Use the recommended fix: start the installer normally, Inno asks for admin, Hydra stays open until the installer closes it, and a declined prompt keeps Hydra running with decision 10's message."

What that changes. T3a launches with `open`, keeps the process handle, and maps "installer process ended while Hydra still runs" to decision 10's state; the canned launcher seam answers "started then closed us", "ended early" or "failed to start". T3a also owns `src/ui/main.cpp`'s `WndProc` for one addition: handle `WM_QUERYENDSESSION` (return TRUE) and `WM_ENDSESSION` (save the INI through the normal exit path and close), so Restart Manager's request closes Hydra cleanly; the real installer-closes-Hydra step is checked once by hand, since no test can drive Restart Manager. T4's line-82 change (drop `skipifsilent`) is correct with this launch.

## The build

Six executor tasks on Opus. T1, T2a and T2b start together from main; T4 can start with them because its files overlap none of theirs. T3a forks from the join of T1, T2a and T2b. T3b forks from T3a's join. Interfaces are fixed here so parallel tasks agree without talking: `net::http_download(const std::string& url, const std::string& path, uint64_t size_cap, std::atomic<uint64_t>* received, const std::atomic<bool>* cancel)`; `app::UpdateRelease` with the six fields above and `app::parse_latest_release_json(const std::string&) -> UpdateRelease`; `app::verify_update(const std::string& path, const std::string& digest_hex, const std::vector<uint8_t>& sig, const std::array<uint8_t, 64>& public_key)`; `core::parse_version` and `core::AppVersion` as above; `app::installed_by_setup()` and `app::set_installed_for_test(std::optional<bool>)`.

### T1: one transport for GET and download (owner: `src/net/http.h`, `src/net/http.cpp`, `src/net/dmbot_client.h`, `src/net/dmbot_client.cpp`, `CMakeLists.txt` hydra_dm block only, `tests/test_http_download.cpp` new, `tests/test_cli.cpp` one new case)

Move the WinHTTP code as "The transport" says. `http_download` writes through a small `download_to_file(source, path, size_cap, received)` helper that takes a byte-source callback, so the chunking, the cap and the `.part` rename are tested without a socket. Keep every existing dm test green with no edit to the parsers.

Acceptance criteria, each a test:
- A1 `download_to_file` with a 100-byte source and cap 100 writes 100 bytes and the `received` counter ends at 100; with cap 99 it throws `UpdateUnverified`, the `.part` file is gone and no final file exists.
- A2 a source that sets `cancel` mid-way ends with `Cancelled` and no file left behind.
- A3 `tests/test_dm_report.cpp` and `tests/test_app_reports.cpp` run green unchanged (the seam moved but kept its name).
- A4 in `test_cli.cpp`: the bytes of `hydra_batch.exe` and `hydra_fillcompare.exe` contain no `winhttp.dll` string in any case (the import table stores DLL names in ASCII), so the CLI tools still link no WinHTTP.

Tests it may run: `hydra_tests.exe -sf=*test_http_download.cpp`, `-sf=*test_dm_report.cpp`, `-sf=*test_app_reports.cpp`, `-tc=*winhttp*` in `test_cli.cpp`.

### T2a: the version rule, the release reading and the installed-exe rule (owner: `src/core/app_version.h`, `src/app/update_release.h`, `src/app/update_release.cpp`, `src/app/config.h` and `config.cpp` for `installed_by_setup` only, `src/core/error_kind.h`, `src/app/user_messages.cpp` and `.h` for the one new sentence, `tests/test_update_release.cpp` new, `testdata/update/latest.json` new, `tests/single_owner/rules/app.json` and `core.json` new rows)

The fixture `testdata/update/latest.json` is the real reply for v2.10.2 (asset name, size 33,241,389, the digest from the findings) with a second fake `.sig` asset added, saved once; tests edit copies of it in memory.

Acceptance criteria, each a test in `test_update_release.cpp`:
- B1 `parse_version`: `"v2.10.2"` and `"2.10.2"` give 2,10,2; `"2.9.1" < "2.10.2"` is true; equal is not less; `"2.10"`, `"abc"`, `""` give empty.
- B2 the fixture parses to the six fields; `installer_size` is 33241389 and the digest is the 64 hex digits without the `sha256:` prefix.
- B3 each of these refuses with `ErrorKind::UpdateUnverified`: no `-setup.exe` asset, two of them, no `.sig`, no `digest`, a digest not starting `sha256:`, a tag that does not parse, a body that is not JSON.
- B4 `plain_error` of that kind is exactly decision 9's sentence and `plain_error_detail` is the raw text.
- B5 `installed_by_setup()` is false in the test process (no registry match for the build folder) and `set_installed_for_test(true)` makes it true; the single-owner rows name `installed_by_setup` as the owner of "is this the installed exe" and `parse_version` as the owner of "is this version newer".

Tests it may run: `hydra_tests.exe -sf=*test_update_release.cpp`, `-sf=*test_app_settings.cpp`, and the single-owner scan case `-tc=*single owner*` once.

### T2b: the verifier and the signing tools (owner: `src/app/update_verify.h`, `src/app/update_verify.cpp`, `src/app/update_key.h`, `CMakeLists.txt` hydra_core source list only, `tools/release_keygen.ps1` new, `tools/release_sign.ps1` new, `tools/test_release_sign.ps1` new, `testdata/update/test-key.xml`, `testdata/update/test-public-key.txt`, `testdata/update/sample.bin`, `testdata/update/sample.bin.sig`, `tests/test_update_verify.cpp` new, `docs/development.md` release section)

Write the two scripts. Run `release_keygen.ps1` once into `testdata/update/` to make the test pair (the test private key is not secret and is committed unprotected; the script's default path for the real key is decision 12's). Sign `sample.bin` with the test key and commit the `.sig`. `update_key.h` ships with the test public key until the user runs the keygen for real and pastes the production key in; T2b's report says so in capitals, and B7 pins that the user has to do it before a release.

Acceptance criteria:
- B6 `verify_update(sample.bin, its sha256, sample.bin.sig, test public key)` passes; flipping any one byte of the file, the signature or the digest fails with `UpdateUnverified`; a 63-byte signature fails; a signature from a second test key fails.
- B7 a test reads `update_key.h`'s constant and fails if it equals the test public key, with the message "replace the test key with the production key before release". The main session leaves this test failing on purpose until the user runs the keygen; the merge note says so. (Alternative, if the user prefers a green suite: a `HYDRA_RELEASE_KEY_SET` CMake option the ship preset turns on, checked by the same test. The user picks.)
- B8 `tools/test_release_sign.ps1` signs `sample.bin` with the test key and compares the bytes to the committed `.sig` (ECDSA signatures are randomized, so instead it verifies the fresh signature with the test public key through .NET, and checks the C++ fixture `.sig` the same way): the PowerShell and C++ sides agree on the format.
- B9 `docs/development.md` gains "Releasing" with the exact commands from "The signature scheme", in order: build with `-Sign`, draft release with both assets, publish.

Tests it may run: `hydra_tests.exe -sf=*test_update_verify.cpp`, `pwsh -NoProfile -File tools/test_release_sign.ps1`.

### T3a: the settings, the jobs and the state (owner: `src/app/config.h`, `src/app/config.cpp` key table, `src/ui/update_job.h`, `src/ui/update_job.cpp`, `src/ui/app_state.h`, `src/ui/app_state.cpp`, `CMakeLists.txt` hydra_ui source list only, `tests/test_update_state.cpp` new, `tests/test_config.cpp` one case; forks from the join of T1, T2a, T2b)

Add the four INI keys and `update_check_due`. Build the two jobs and `AppState::UpdateState` with the phases and the three actions `update_now()`, `skip_version()`, `later()`, plus `check_updates_now()` for the settings button, `cancel_update()` and the launch step as "The job" says. The launch is behind a seam (`set_launcher_for_test`) so a test can answer "started", "declined" or "failed".

Acceptance criteria, in `test_update_state.cpp` unless named:
- C1 (`test_config.cpp`) the four keys round-trip through `save_file`/`load_file`; a missing `check_updates` reads on; `with_analysis_defaults` leaves all four as they were.
- C2 `update_check_due`: off gives false; not installed gives false; on, installed, no timestamp gives true; a timestamp 23 hours old gives false; 25 hours old gives true.
- C3 with the canned fetcher returning the fixture edited to v9.9.9, a check lands in phase `offered` with the strip text `Hydra Deluxe 9.9.9 is available.`; the same fixture at the running version or lower lands in `none`.
- C4 `skip_version()` writes `skipped_update_version=9.9.9` and a second check for 9.9.9 lands in `none`; a check for 9.9.10 lands in `offered`.
- C5 `later()` lands in `none` and writes nothing but `last_update_check`.
- C6 a failed automatic check (fetcher throws `NetHttpStatus` 403, then `NetUnreachable`) lands in `none` with empty text; the same failure after `check_updates_now()` lands in `failed` with `plain_error`'s sentence as the text and `plain_error_detail` as the detail line.
- C7 `update_now()` with the canned downloader copying `testdata/update/sample.bin` and the canned fetcher serving its `.sig` reaches `launching`; the canned launcher answering "declined" lands in `declined` with decision 10's text and the file kept; "started" sets the exit request; a wrong `.sig` lands in `failed` with decision 9's sentence and the file deleted.
- C8 `cancel_update()` during the download lands in `offered` with no part-file left.
- C9 on construction with `last_run_version=2.10.1` and `HYDRA_VERSION` 2.10.2, the phase is `updated` with text `Updated to Hydra Deluxe 2.10.2.` and the INI now says 2.10.2; with an equal or empty value, phase `none` and the INI written once.

Tests it may run: `hydra_tests.exe -sf=*test_update_state.cpp`, `-sf=*test_config.cpp`, `-sf=*test_app_settings.cpp`.

### T3b: the strip and the settings row (owner: `src/ui/library_toolbar.cpp`, `src/ui/settings_bar.cpp`, `src/ui/library_parts.h`, `tests/ui/uitest_library.cpp` or a new `tests/ui/uitest_update.cpp`, `tests/ui/uitest_harness.cpp` canned fetcher only; forks from T3a's join)

Draw the strip from `UpdateState` as decisions 1 to 4, 10 and 11 say, and add the `App` group with the `Check for updates` row, its help marker and `Check now` as decision 5 says. The harness's canned fetcher answers the `releases/latest` URL with the fixture at v9.9.9 and `set_installed_for_test(true)` only inside the update tests.

Acceptance criteria, in one uitest script `update-strip`:
- D1 after the check, `on_screen("Hydra Deluxe 9.9.9 is available.")` and the three button labels are visible; hovering the version shows the fixture's notes text.
- D2 clicking `Skip this version` hides the strip; a rerun of the check keeps it hidden.
- D3 clicking `Update` with the canned downloader shows `Downloading Hydra Deluxe 9.9.9...` and a `Cancel` button; after the canned launcher answers "declined", the strip reads decision 10's text with the three buttons back.
- D4 the `Analysis settings` panel shows the `App` group with `Check for updates` ticked; unticking it writes `check_updates=0` to the INI; with the batch lock on (the existing locked-panel test's setup) the row stays enabled.
- D5 at a start with `last_run_version` older, the strip reads `Updated to Hydra Deluxe <HYDRA_VERSION>.`, `OK` hides it, and the next start shows nothing.
- D6 the existing `settings-and-reports` script still passes (the panel gained a group without moving the others).

Tests it may run: `hydra_uitest update-strip`, `hydra_uitest settings-and-reports`.

### T4: the installer line, the docs and the decision record (owner: `installer/hydra.iss`, `installer/build_installer.ps1` for the `-Sign` switch only, `README.md`, `docs/UserGuide.md`, `docs/handoffs/release-next-notes.md`, `docs/audit/2026-10-03-fix-decisions.md`, `CONTEXT.md` one paragraph; runs in parallel from main)

Drop `skipifsilent` on line 82 of `hydra.iss` and add the design note. Add `-Sign` to `build_installer.ps1`, calling `tools/release_sign.ps1` (T2b writes it; T4 only calls it by path, and the call fails loudly if the script is missing). Write decision 13's text into the README and the User Guide, the settings-table row, the release-note paragraph, a D-number recording the user's answers to all fourteen decisions, and a CONTEXT.md paragraph under "Reports and messages" naming the owners (`parse_version`, `installed_by_setup`, `update_check_due`, `verify_update`).

Acceptance criteria:
- E1 `tests/test_docs_match_code.cpp` runs green (it pins User Guide settings against the code; the new row must match the label and default in `settings_bar.cpp` and `config.h`). If the test does not cover the settings table today, T4 adds the one row that does.
- E2 the reviewer reads `hydra.iss` line 82 against Inno's `postinstall` and `runasoriginaluser` rules and the README sentence against decision 5's label and default.
- E3 the D-record lists all fourteen decisions with the user's words.

Tests it may run: `hydra_tests.exe -sf=*test_docs_match_code.cpp`.

## Testing without GitHub, and the real check

Nothing in the tests touches the network. The release reply is `testdata/update/latest.json`, served by the canned fetcher through the moved `set_fetcher` seam. The download is `testdata/update/sample.bin`, copied by the canned `set_downloader`. The signature is `sample.bin.sig`, made once with the committed test key; the C++ verifier and the PowerShell signer are cross-checked in B6 and B8. The installer launch is a seam that answers started, declined or failed. The real transport is exercised only by hand, as today for the leaderboard (`test_dm_report.cpp` line 553 shows the pattern of a manual case that resets the seam).

The user's real check, after the merge and before the release that ships this: build the installer with `-Sign` using the production key, install it over the current Hydra, then set `last_update_check=` to empty in `hydra_settings.ini` and start Hydra. With the production release still at or below the installed version the strip stays hidden; to see the offer, temporarily publish a draft-then-public test release with a higher tag, or edit the INI's `last_run_version` to an older value to see decision 11's message. The full path (offer, download, UAC, silent install, relaunch, "Updated to") is seen once for real on the first release after this one, which is also when B7's key check first matters. Memory rule `ui-changes-need-real-app-check` applies to the strip: the main session captures it in the real window at the user's window size before merging T3b, with the canned fetcher through `Hydra.exe --uitest` attached mode.

## Out of scope (for the user to approve)

- Code-signing the installer with an Authenticode certificate (SmartScreen). Hydra has no certificate; the README's SmartScreen sentence stays.
- Checking for updates from the command-line tools.
- Delta or background-applied updates; the installer always runs in full with its progress window.
- Updating from a prerelease or draft; `releases/latest` never names one.
- Any change to what the installer keeps (databases, INIs, reports) on upgrade.

## Not confirmed

The findings' note that the installer leaves `hydra*.db` and INI files alone is confirmed (`hydra.iss` lines 17 to 20). Two things were not confirmed in this planning pass and T4 must check them against Inno Setup's documentation before editing: that a `postinstall` entry without `skipifsilent` runs under `/SILENT` as if ticked, and that such an entry runs as the original (unelevated) user by default. The exact `ERROR_CANCELLED` code from `ShellExecuteExW` on a UAC decline is from the findings and the Win32 docs, not from a run; C7's canned launcher pins the behaviour, and the user's real check sees it once. The release process has no script in the repo; the session cards show releases made by hand with `gh` and the installer attached, which T2b's docs section formalizes.

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| Only a strictly newer version is offered; equal or older never | The version rule | T2a, T3a | B1, C3 |
| One version-compare function | The version rule | T2a | B5 single-owner row |
| One "is this the installed exe" rule; dev and worktree builds never check | Decision 14 | T2a, T3a | B5, C2 |
| Every error line through plain_error / plain_error_detail | The job; decision 8 | T3a | C6, C7 |
| Refuse on missing asset, two assets, missing .sig, missing digest, bad tag, bad JSON | The signature scheme | T2a | B3 |
| Refuse on bad digest, bad signature, wrong length, wrong key | The signature scheme | T2b | B6 |
| Refuse a body past the asset's size; no file left | Decision 7 | T1 | A1 |
| Cancel leaves no part-file | Decision 4 | T1, T3a | A2, C8 |
| Production key must replace the test key before release | The signature scheme | T2b | B7 |
| PowerShell signer and C++ verifier agree | The signature scheme | T2b | B8 |
| Release steps written down: sign, draft, publish | The signature scheme | T2b | B9 |
| Private key outside the repo and CI | Decision 12 | T2b | keygen default path is outside the tree; reviewer checks the script writes nowhere under the repo |
| CLI tools link no WinHTTP | The release reading | T1 | A4 |
| dm parsers unchanged | T1 | T1 | A3 |
| One check per 24 hours; Check now ignores it | Decision 6 | T3a | C2, C6 |
| Silent automatic failure, visible manual failure | Decision 8 | T3a | C6 |
| Skip hides that version only | Decision 3 | T3a, T3b | C4, D2 |
| Later hides until the next check | Decision 3 | T3a | C5 |
| UAC decline keeps Hydra running with decision 10's text | Decision 10 | T3a, T3b | C7, D3 |
| Launch success exits Hydra through the normal path | The job | T3a | C7 ("started" sets the exit request) |
| "Updated to X" shown once after a version change | Decision 11 | T3a, T3b | C9, D5 |
| Setting label, default on, not locked, not reset | Decision 5 | T3a, T3b | C1, D4 |
| Strip wording and buttons as decided; notes shown as plain text | Decisions 1 to 4 | T3b | D1, D3 |
| Existing settings panel flows still pass | T3b | T3b | D6 |
| Silent install relaunches Hydra as the user | The installer | T4 | E2 (reviewer against Inno docs; listed under Not confirmed) |
| Public docs say Hydra checks for updates, in the decided words | Decision 13 | T4 | E1, E2 |
| All fourteen decisions recorded | Decisions | T4 | E3 |
| No network in tests | Testing section | T1, T3a, T3b | A1, C3, D1 use the seams; the harness installs both seams in `reset_app` |
| Real-app capture of the strip before merge | Testing section | main session | the user sees the capture |
