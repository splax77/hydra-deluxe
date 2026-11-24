#include "ui/win32_dialogs.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <knownfolders.h>
#include <shlobj.h>  // SHGetKnownFolderItem, KF_FLAG_DEFAULT
#include <shobjidl.h>
#include <wrl/client.h>

#include <functional>

#include "core/winstr.h"

using Microsoft::WRL::ComPtr;

namespace hydra::ui {

namespace {

// Shows the system's file dialog `clsid` (the open or the save dialog) over
// `owner`, set up by `configure`, and returns the picked item's file-system
// path. Every picker below goes through it, so they cancel and fail alike.
std::optional<std::string> show_file_dialog(HWND owner, REFCLSID clsid,
                                            const std::function<void(IFileDialog&)>& configure,
                                            bool* failed) {
    if (failed) *failed = false;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    bool we_initialized_com = SUCCEEDED(hr);
    // RPC_E_CHANGED_MODE means COM is already initialized on this thread in a
    // different mode (the Win32/DX11 message loop may have done so) -- fine,
    // just don't uninitialize what we didn't initialize.
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        if (failed) *failed = true;
        return std::nullopt;
    }

    std::optional<std::string> result;
    ComPtr<IFileDialog> dialog;
    if (SUCCEEDED(CoCreateInstance(clsid, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog)))) {
        configure(*dialog.Get());

        if (SUCCEEDED(dialog->Show(owner))) {
            ComPtr<IShellItem> item;
            if (SUCCEEDED(dialog->GetResult(&item))) {
                PWSTR path = nullptr;
                if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                    result = wide_to_utf8(path);
                    CoTaskMemFree(path);
                }
            }
        }
        // Show() failing is (almost always) the user cancelling: not a failure.
    } else {
        if (failed) *failed = true;
    }

    if (we_initialized_com) CoUninitialize();
    return result;
}

// `types` as the dialog takes them. The dialog reads the strings until Show
// returns, so `types` must outlive the call.
std::vector<COMDLG_FILTERSPEC> filter_specs(const std::vector<FileTypeFilter>& types) {
    std::vector<COMDLG_FILTERSPEC> specs;
    for (const FileTypeFilter& t : types) specs.push_back({t.name.c_str(), t.patterns.c_str()});
    return specs;
}

// The folder `utf8_folder` as a shell item, or the user's Documents folder
// when it is empty or cannot be opened. Null only when neither can be had.
// The shell parses the path, so it gets the form shell_path gives.
ComPtr<IShellItem> start_folder(const std::string& utf8_folder) {
    ComPtr<IShellItem> folder;
    const std::wstring wide =
        utf8_folder.empty() ? std::wstring() : shell_path(win32_path(utf8_folder));
    if (!wide.empty()) {
        if (SUCCEEDED(SHCreateItemFromParsingName(wide.c_str(), nullptr, IID_PPV_ARGS(&folder))))
            return folder;
        folder.Reset();
    }
    SHGetKnownFolderItem(FOLDERID_Documents, KF_FLAG_DEFAULT, nullptr, IID_PPV_ARGS(&folder));
    return folder;
}

}  // namespace

std::optional<std::string> browse_for_folder(HWND owner, bool* failed) {
    return show_file_dialog(
        owner, CLSID_FileOpenDialog,
        [](IFileDialog& dialog) {
            DWORD flags = 0;
            dialog.GetOptions(&flags);
            dialog.SetOptions(flags | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
        },
        failed);
}

std::optional<std::string> browse_for_file(HWND owner, const std::vector<FileTypeFilter>& types,
                                           bool* failed) {
    const std::vector<COMDLG_FILTERSPEC> specs = filter_specs(types);
    return show_file_dialog(
        owner, CLSID_FileOpenDialog,
        [&](IFileDialog& dialog) {
            DWORD flags = 0;
            dialog.GetOptions(&flags);
            dialog.SetOptions(flags | FOS_FILEMUSTEXIST | FOS_FORCEFILESYSTEM);
            if (!specs.empty())
                dialog.SetFileTypes(static_cast<UINT>(specs.size()), specs.data());
        },
        failed);
}

std::optional<std::string> browse_for_save_file(HWND owner,
                                                const std::vector<FileTypeFilter>& types,
                                                const std::wstring& default_extension,
                                                bool* failed,
                                                const std::optional<SaveDialogStart>& start) {
    const std::vector<COMDLG_FILTERSPEC> specs = filter_specs(types);
    return show_file_dialog(
        owner, CLSID_FileSaveDialog,
        [&](IFileDialog& dialog) {
            DWORD flags = 0;
            dialog.GetOptions(&flags);
            dialog.SetOptions(flags | FOS_OVERWRITEPROMPT | FOS_FORCEFILESYSTEM);
            if (!specs.empty())
                dialog.SetFileTypes(static_cast<UINT>(specs.size()), specs.data());
            if (!default_extension.empty()) dialog.SetDefaultExtension(default_extension.c_str());
            if (start) {
                // SetFolder, not SetDefaultFolder: the start wins over the
                // folder Windows remembers from this app's last dialog.
                const ComPtr<IShellItem> folder = start_folder(start->folder);
                if (folder) dialog.SetFolder(folder.Get());
                if (!start->file_name.empty())
                    dialog.SetFileName(utf8_to_wide(start->file_name).c_str());
            }
        },
        failed);
}

}  // namespace hydra::ui
