#include "pathbuilder/pb_open.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>

#include "core/winstr.h"

namespace hydra::pathbuilder {

std::vector<ui::FileTypeFilter> chart_file_types() {
    // Kept beside chart_format_of's list; a format added there is added here.
    return {{"Charts (.mid, .chart, .sng, .srb)", "*.mid;*.chart;*.sng;*.srb"},
            {"All files", "*.*"}};
}

std::optional<std::string> ask_for_chart(HWND owner, bool* failed) {
    return ui::browse_for_file(owner, chart_file_types(), failed);
}

std::optional<std::string> take_dropped_file(void* hdrop) {
    const HDROP drop = static_cast<HDROP>(hdrop);
    std::optional<std::string> path;
    const UINT length = ::DragQueryFileW(drop, 0, nullptr, 0);
    if (length > 0) {
        std::wstring name(length + 1, L'\0');
        ::DragQueryFileW(drop, 0, name.data(), length + 1);
        name.resize(length);
        path = wide_to_utf8(name);
    }
    ::DragFinish(drop);
    return path;
}

}  // namespace hydra::pathbuilder
