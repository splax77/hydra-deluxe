// The updater's two background jobs and the state the strip under the toolbar
// draws from (docs/superpowers/plans/2026-10-22-auto-updater.md, "The job").
// UpdateCheckJob asks GitHub for the latest release; UpdateDownloadJob fetches
// its installer and signature and checks them. AppState owns both, holds the
// UpdateState and starts the installer through the launcher below.

#ifndef HYDRA_UI_UPDATE_JOB_H
#define HYDRA_UI_UPDATE_JOB_H

#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>

#include "app/update_release.h"
#include "ui/job_base.h"

namespace hydra::ui {

// The GitHub API reply that names the latest published release.
inline constexpr const char* kLatestReleaseUrl =
    "https://api.github.com/repos/splax77/hydra-deluxe/releases/latest";

// What the strip under the toolbar shows about updates. AppState writes it;
// the strip only reads it and calls AppState's update actions.
struct UpdateState {
    enum class Phase {
        None,         // nothing to show
        Offered,      // `release` is newer than this Hydra: Update, Skip this version, Later
        Downloading,  // the installer is downloading (AppState::update_received)
        Verifying,    // the digest and signature checks run
        Launching,    // the installer was started; Hydra waits for it to close Hydra
        Declined,     // the installer ended without updating, or didn't start (then
                      // `detail` says why); the three buttons are back
        Failed,       // a check or update the user asked for failed: `text` and `detail`
        Updated,      // the first start after a version change: one OK button
        UpToDate,     // Check now found nothing newer: one OK button
    };
    Phase phase = Phase::None;
    // The release offered or being installed. Set from Offered to Declined,
    // and in Failed when an update of it failed (so Update can try again).
    std::optional<app::UpdateRelease> release;
    // The strip's sentence for the phase. Empty in None.
    std::string text;
    // Failed and a Declined that didn't start: app::plain_error_detail of the
    // error, the line under `text`.
    std::string detail;
};

// GETs kLatestReleaseUrl and reads it with app::parse_latest_release_json.
// Whether the release is newer, or skipped, is AppState's to say.
class UpdateCheckJob : public ResultJobBase {
public:
    UpdateCheckJob() = default;
    ~UpdateCheckJob() { shutdown(); }

    void start();

    // Valid once finished() && ok().
    const app::UpdateRelease& release() const { return release_; }

private:
    void run();
    app::UpdateRelease release_;
};

// Downloads the release's installer into `folder`, fetches its .sig and runs
// app::verify_update against `public_key`. Any failure, a refusal or a cancel
// deletes the installer. With `reuse` set and the installer already in the
// folder (kept after a declined start), it skips the download and checks the
// kept file again.
class UpdateDownloadJob : public ResultJobBase {
public:
    UpdateDownloadJob(app::UpdateRelease release, std::string folder,
                      std::array<uint8_t, 64> public_key, bool reuse);
    ~UpdateDownloadJob() { shutdown(); }

    void start();

    // Where the installer goes: `folder` joined with update_installer_name.
    const std::string& installer_path() const { return installer_path_; }
    // Bytes downloaded so far, read each frame.
    uint64_t received() const { return received_.load(); }
    // True once the download is done and the checks run.
    bool verifying() const { return verifying_.load(); }

private:
    void run();
    app::UpdateRelease release_;
    std::string folder_;
    std::array<uint8_t, 64> public_key_;
    bool reuse_;
    std::string installer_path_;
    std::atomic<uint64_t> received_{0};
    std::atomic<bool> verifying_{false};
};

// The installer's file name for a release, "HydraDeluxe-2.11.0-setup.exe".
std::string update_installer_name(const app::UpdateRelease& release);

// The folder downloads go to: <temp>\HydraDeluxe-update, or the test override.
std::string update_folder();
// Test seam: a non-empty folder replaces update_folder's answer; empty puts
// the temp folder back.
void set_update_folder_for_test(std::string folder);

// Starts the installer at `installer`, writing Inno Setup's log to `log`, and
// returns a check that says whether its process has ended. Throws when the
// installer could not be started, with Windows' reason as the text. The real
// one starts it through app::start_program, so the installer raises its own
// UAC prompt ("Ruling: how Hydra starts the installer" in the plan).
using InstallerEnded = std::function<bool()>;
using InstallerLauncher =
    std::function<InstallerEnded(const std::string& installer, const std::string& log)>;
InstallerEnded launch_installer(const std::string& installer, const std::string& log);
// Test seam: an installed launcher replaces the real one; empty restores it.
void set_launcher_for_test(InstallerLauncher launcher);

}  // namespace hydra::ui

#endif  // HYDRA_UI_UPDATE_JOB_H
