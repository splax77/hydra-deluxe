// Dear ImGui's file functions (imconfig.h turns its own off). ImGui opens
// hydra_ui.ini and the font files itself; routing the open through
// fopen_utf8 gives it the same UTF-8 and long-path handling as the rest of
// Hydra. The rest matches ImGui's defaults, except the size, which comes from
// winstr's 64-bit open-file size.

#include "imgui.h"
#include "imgui_internal.h"

#include <cstring>
#include <string>

#include "core/winstr.h"

ImFileHandle ImFileOpen(const char* filename, const char* mode) {
    const std::wstring wide_mode(mode, mode + std::strlen(mode));  // "rb", "wt": ASCII
    return hydra::fopen_utf8(filename, wide_mode.c_str());
}

bool ImFileClose(ImFileHandle f) { return std::fclose(f) == 0; }

ImU64 ImFileGetSize(ImFileHandle f) {
    return hydra::open_file_size_bytes(f).value_or(static_cast<ImU64>(-1));
}

ImU64 ImFileRead(void* data, ImU64 size, ImU64 count, ImFileHandle f) {
    return std::fread(data, static_cast<size_t>(size), static_cast<size_t>(count), f);
}

ImU64 ImFileWrite(const void* data, ImU64 size, ImU64 count, ImFileHandle f) {
    return std::fwrite(data, static_cast<size_t>(size), static_cast<size_t>(count), f);
}
