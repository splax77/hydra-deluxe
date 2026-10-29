#include "ui/win32_dialogs.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shobjidl.h>
#include <wrl/client.h>

#include <functional>

#include "core/winstr.h"

using Microsoft::WRL::ComPtr;

namespace hydra::ui {

namespace {

// Shows the system's open dialog over `owner`, set up by `configure`, and
// returns the picked item's file-system path. Both pickers below go through
// it, so they cancel and fail alike.
std::optional<std::string> show_open_dialog(HWND owner,
                                            const std::function<void(IFileOpenDialog&)>& configure,
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
    ComPtr<IFileOpenDialog> dialog;
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
                                   IID_PPV_ARGS(&dialog)))) {
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

}  // namespace

std::optional<std::string> browse_for_folder(HWND owner, bool* failed) {
    return show_open_dialog(
        owner,
        [](IFileOpenDialog& dialog) {
            DWORD flags = 0;
            dialog.GetOptions(&flags);
            dialog.SetOptions(flags | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
        },
        failed);
}

std::optional<std::string> browse_for_file(HWND owner, const std::vector<FileTypeFilter>& types,
                                           bool* failed) {
    // The dialog reads these until Show returns; `types` outlives the call.
    std::vector<COMDLG_FILTERSPEC> specs;
    for (const FileTypeFilter& t : types) specs.push_back({t.name.c_str(), t.patterns.c_str()});
    return show_open_dialog(
        owner,
        [&](IFileOpenDialog& dialog) {
            DWORD flags = 0;
            dialog.GetOptions(&flags);
            dialog.SetOptions(flags | FOS_FILEMUSTEXIST | FOS_FORCEFILESYSTEM);
            if (!specs.empty())
                dialog.SetFileTypes(static_cast<UINT>(specs.size()), specs.data());
        },
        failed);
}

}  // namespace hydra::ui
