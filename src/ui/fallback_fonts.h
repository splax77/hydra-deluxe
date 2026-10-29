// The Windows fonts that draw the characters Hydra's own fonts lack.
//
// Hydra's main and mono fonts cover Latin and, for the main font, Japanese.
// Clone Hero libraries hold titles in every script, plus odd punctuation and
// symbols. setup_imgui (src/ui/imgui_setup.cpp) merges every file below that
// exists on this machine behind each of the two fonts, in this order, so a
// character comes from the first file that has it. A missing file is skipped.
// tests/test_font_coverage.cpp reads this same list to know which characters
// this machine can draw.

#ifndef HYDRA_UI_FALLBACK_FONTS_H
#define HYDRA_UI_FALLBACK_FONTS_H

#include <array>
#include <string>
#include <string_view>

namespace hydra::ui {

// The folder Windows keeps its fonts in.
inline constexpr std::string_view kWindowsFontsDir = "C:\\Windows\\Fonts\\";

// File names in kWindowsFontsDir, tried in order. The order is for coverage
// and look: Segoe UI comes first because it is the closest match for a gap
// in Latin text. The East Asian fonts all stay: each fills gaps the others
// leave, so they are tried in turn, not as alternatives.
inline constexpr std::array<std::string_view, 14> kFallbackFontFiles = {
    "segoeui.ttf",   // Segoe UI: Latin, Greek, Cyrillic, punctuation
    "seguisym.ttf",  // Segoe UI Symbol: math, arrows, symbols
    "YuGothM.ttc",   // Yu Gothic Medium: Japanese
    "meiryo.ttc",    // Meiryo: Japanese
    "msgothic.ttc",  // MS Gothic: Japanese
    "malgun.ttf",    // Malgun Gothic: Korean
    "msyh.ttc",      // Microsoft YaHei: Simplified Chinese
    "msjh.ttc",      // Microsoft JhengHei: Traditional Chinese
    "Nirmala.ttc",   // Nirmala UI: Indic scripts
    "LeelawUI.ttf",  // Leelawadee UI: Thai, Lao, Khmer
    "ebrima.ttf",    // Ebrima: African scripts
    "gadugi.ttf",    // Gadugi: Cherokee, Unified Canadian Aboriginal
    "seguihis.ttf",  // Segoe UI Historic: historic scripts
    "seguiemj.ttf",  // Segoe UI Emoji: emoji, drawn in one color by ImGui's loader
};

// The full path of one entry of kFallbackFontFiles.
inline std::string fallback_font_path(std::string_view file) {
    std::string path(kWindowsFontsDir);
    path += file;
    return path;
}

}  // namespace hydra::ui

#endif  // HYDRA_UI_FALLBACK_FONTS_H
