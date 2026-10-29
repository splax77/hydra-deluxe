// The updater's two background jobs and the state the version bar draws from
// (D259 and D260 in docs/audit/2026-10-03-fix-decisions.md).
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

#include "app/job_base.h"
#include "app/update_release.h"

namespace hydra::ui {

// The GitHub API reply that names the latest published release.
inline constexpr const char* kLatestReleaseUrl =
    "https://api.github.com/repos/splax77/hydra-deluxe/releases/latest";

// What the version bar shows about updates. AppState writes it; the bar only
// reads it (through AppState::version_bar_status in the resting state) and
// calls AppState's update actions.
struct UpdateState {
    enum class Phase {
        None,         // no update step: the bar shows its resting status
        Offered,      // `release` is newer than this Hydra: Update, Skip this version, Later
        Downloading,  // the installer is downloading (AppState::update_received)
        Verifying,    // the digest and signature checks run
        Launching,    // the installer was started; Hydra waits for it to close Hydra
        Declined,     // the installer ended without updating, or didn't start (then
                      // `detail` says why); the three buttons are back
        Failed,       // an update failed: `text` and `detail`
        Updated,      // the first start after a version change: one OK button
    };
    Phase phase = Phase::None;
    // What this run's start-up check found.
    enum class LastCheck {
        NotRun,    // none finished yet (not the installed exe, or still running)
        Failed,    // it failed: `check_error` says why
        UpToDate,  // the latest release is this Hydra's version or older
        Newer,     // the latest release, in `found`, is newer than this Hydra
    };
    LastCheck last_check = LastCheck::NotRun;
    // The failed check's app::plain_error sentence. Empty otherwise.
    std::string check_error;
    // The release a Newer check returned, notes included; kept after Later
    // and Skip this version so the bar can still name it.
    std::optional<app::UpdateRelease> found;
    // The release offered or being installed. Set from Offered to Declined,
    // and in Failed when an update of it failed (so Update can try again).
    std::optional<app::UpdateRelease> release;
    // The bar's sentence for the phase. Empty in None.
    std::string text;
    // Failed and a Declined that didn't start: app::plain_error_detail of the
    // error, the line under `text`.
    std::string detail;
};

// What the version bar says after the app's name: AppState::version_bar_status
// works it out, and the bar only draws it.
struct VersionBarStatus {
    // Green for up to date, amber for a newer release known, grey for every
    // other state.
    enum class Dot { Neutral, UpToDate, Available };
    Dot dot = Dot::Neutral;
    // The sentence, drawn in the normal text colour. Empty when the whole
    // status is dimmed.
    std::string text;
    // A dimmed sentence after `text`. Empty in most states.
    std::string dimmed;
    // Whether the resting bar offers its own Update button (AppState::update_now).
    // Never true while a phase shows: the phase draws its own buttons.
    bool update_button = false;
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
