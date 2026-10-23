#include "ui/update_job.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>

#include <filesystem>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <vector>

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

// Closes the installer's process handle when the last copy of the check goes.
struct ProcessHandle {
    HANDLE handle = nullptr;
    explicit ProcessHandle(HANDLE h) : handle(h) {}
    ~ProcessHandle() {
        if (handle) CloseHandle(handle);
    }
    ProcessHandle(const ProcessHandle&) = delete;
    ProcessHandle& operator=(const ProcessHandle&) = delete;
};

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
    const std::wstring file = win32_path(installer);
    // Inno Setup's switches: a progress window with no questions, no reboot
    // and no Cancel, and a log beside the download.
    const std::wstring params =
        L"/SILENT /SUPPRESSMSGBOXES /NORESTART /NOCANCEL /LOG=\"" + win32_path(log) + L"\"";
    SHELLEXECUTEINFOW info{};
    info.cbSize = sizeof(info);
    info.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
    // Never "runas": the installer raises its own UAC prompt, so it can start
    // Hydra again as the user once it is done (the plan's ruling).
    info.lpVerb = L"open";
    info.lpFile = file.c_str();
    info.lpParameters = params.c_str();
    info.nShow = SW_SHOWNORMAL;
    if (!ShellExecuteExW(&info))
        throw std::runtime_error("the installer didn't start: " +
                                 std::system_category().message(static_cast<int>(GetLastError())));
    // An exe always hands back its process. Without one there is nothing to
    // watch, and Hydra waits for the installer to close it.
    auto process = std::make_shared<ProcessHandle>(info.hProcess);
    return [process] {
        return process->handle && WaitForSingleObject(process->handle, 0) == WAIT_OBJECT_0;
    };
}

void set_launcher_for_test(InstallerLauncher launcher) { g_launcher_for_test = std::move(launcher); }

}  // namespace hydra::ui
