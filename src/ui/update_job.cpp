#include "ui/update_job.h"

#include <filesystem>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <vector>

#include "app/report_files.h"  // start_program
#include "app/update_verify.h"
#include "core/winstr.h"  // join_folder, os_path, win32_path
#include "net/http.h"

namespace hydra::ui {

namespace {

// The name the transport gives GitHub in its raw detail lines.
constexpr const char* kGitHub = "GitHub";

std::string g_update_folder_for_test;
InstallerLauncher g_launcher_for_test;

void remove_file(const std::string& path) {
    std::error_code ec;
    std::filesystem::remove(os_path(path), ec);
}

}  // namespace

// ---- UpdateCheckJob ---------------------------------------------------

void UpdateCheckJob::start() { spawn([this] { run(); }); }

void UpdateCheckJob::run() {
    run_guarded([this] {
        const std::string body = net::http_get(kLatestReleaseUrl, &cancel_, kGitHub);
        if (is_cancelled()) return false;
        release_ = app::parse_latest_release_json(body);
        return true;
    });
}

// ---- UpdateDownloadJob ------------------------------------------------

UpdateDownloadJob::UpdateDownloadJob(app::UpdateRelease release, std::string folder,
                                     std::array<uint8_t, 64> public_key, bool reuse)
    : release_(std::move(release)),
      folder_(std::move(folder)),
      public_key_(public_key),
      reuse_(reuse),
      installer_path_(join_folder(folder_, update_installer_name(release_))) {}

void UpdateDownloadJob::start() { spawn([this] { run(); }); }

void UpdateDownloadJob::run() {
    run_guarded([this] {
        try {
            std::error_code ec;
            std::filesystem::create_directories(os_path(folder_), ec);
            const bool kept = reuse_ && std::filesystem::is_regular_file(os_path(installer_path_), ec);
            if (!kept)
                net::http_download(release_.installer_url, installer_path_, release_.installer_size,
                                   &received_, &cancel_, kGitHub);
            else
                received_.store(release_.installer_size);
            throw_if_cancelled();
            verifying_.store(true);
            const std::string sig = net::http_get(release_.sig_url, &cancel_, kGitHub);
            throw_if_cancelled();
            app::verify_update(installer_path_, release_.digest,
                               std::vector<uint8_t>(sig.begin(), sig.end()), public_key_);
            throw_if_cancelled();
        } catch (...) {
            // A refused, failed or cancelled update never leaves an installer
            // behind to be started later.
            remove_file(installer_path_);
            throw;
        }
        return true;
    });
}

std::string update_installer_name(const app::UpdateRelease& release) {
    return "HydraDeluxe-" + core::version_text(release.version) + "-setup.exe";
}

std::string update_folder() {
    if (!g_update_folder_for_test.empty()) return g_update_folder_for_test;
    std::error_code ec;
    const std::filesystem::path temp = std::filesystem::temp_directory_path(ec);
    if (ec) throw std::runtime_error("the temp folder could not be found: " + ec.message());
    return wide_to_utf8((temp / L"HydraDeluxe-update").native());
}

void set_update_folder_for_test(std::string folder) { g_update_folder_for_test = std::move(folder); }

InstallerEnded launch_installer(const std::string& installer, const std::string& log) {
    if (g_launcher_for_test) return g_launcher_for_test(installer, log);
    // Inno Setup's switches: a progress window with no questions, no reboot
    // and no Cancel, and a log beside the download.
    const std::wstring params =
        L"/SILENT /SUPPRESSMSGBOXES /NORESTART /NOCANCEL /LOG=\"" + win32_path(log) + L"\"";
    // Without a process to watch, ended() stays false and Hydra waits for the
    // installer to close it.
    std::shared_ptr<app::StartedProgram> program = app::start_program(win32_path(installer), params);
    return [program] { return program->ended(); };
}

void set_launcher_for_test(InstallerLauncher launcher) { g_launcher_for_test = std::move(launcher); }

}  // namespace hydra::ui
