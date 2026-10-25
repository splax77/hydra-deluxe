// Every character Hydra draws comes out as itself, not as the "?" fallback,
// whenever this machine has a font that holds it. The fonts are the ones
// setup_imgui loads: the main and mono fonts with the backup list in
// src/ui/fallback_fonts.h merged behind each.

#include "doctest.h"

#include <cstdio>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "imgui.h"

#include "core/winstr.h"  // file_exists_utf8
#include "headless_imgui_fixture.h"
#include "ui/fallback_fonts.h"
#include "ui/fonts.h"  // g_mono_font

using hydra::test::HeadlessImGui;

namespace {

// The characters the user's library uses that the main font file lacks, then
// one character from each script the backup list is there for.
const std::vector<ImWchar> kSample = {
    // From the library: titles, artists, charters and folders.
    0x0102, 0x0113, 0x0142, 0x014D, 0x0161, 0x017C, 0x01DD, 0x0265,
    0x0287, 0x028C, 0x02DD, 0x03A9, 0x1D09, 0x200E, 0x2010, 0x221E,
    // One per script.
    0x3042,   // Japanese kana: HIRAGANA LETTER A
    0x4E2D,   // Chinese: CJK ideograph "middle"
    0xD55C,   // Korean Hangul: HAN
    0x0416,   // Cyrillic: ZHE
    0x03BB,   // Greek: lambda
    0x0915,   // Devanagari: KA
    0x0E01,   // Thai: KO KAI
    0x2211,   // math: N-ARY SUMMATION
    0x1F600,  // above U+FFFF: GRINNING FACE
};

// The backup font files present on this machine, each loaded on its own into
// a separate atlas, so whether a file holds a character is read from the
// file and not from the merged fonts under test.
struct BackupFiles {
    ImFontAtlas atlas;
    std::vector<std::pair<std::string, ImFont*>> present;
    std::vector<std::string> absent;

    explicit BackupFiles(float size) {
        for (std::string_view file : hydra::ui::kFallbackFontFiles) {
            const std::string path = hydra::ui::fallback_font_path(file);
            ImFont* font = hydra::file_exists_utf8(path)
                               ? atlas.AddFontFromFileTTF(path.c_str(), size)
                               : nullptr;
            if (font)
                present.emplace_back(std::string(file), font);
            else
                absent.emplace_back(file);
        }
    }

    // The first present file that holds c, or "" when none does.
    std::string holder(ImWchar c) const {
        for (const auto& [file, font] : present)
            if (font->IsGlyphInFont(c)) return file;
        return {};
    }

    std::string absent_list() const {
        std::string s;
        for (const std::string& f : absent) s += (s.empty() ? "" : ", ") + f;
        return s.empty() ? "none" : s;
    }
};

std::string hex(ImWchar c) {
    char buf[16];
    snprintf(buf, sizeof buf, "U+%04X", static_cast<unsigned>(c));
    return buf;
}

// The glyph `font` draws for c at `size`, or null when it would draw the
// fallback instead.
const ImFontGlyph* own_glyph(ImFont* font, float size, ImWchar c) {
    ImFontGlyph* g = font->GetFontBaked(size)->FindGlyphNoFallback(c);
    return g && g->Codepoint == c ? g : nullptr;
}

}  // namespace

TEST_CASE("font coverage: a character some backup font holds draws in the main and mono fonts") {
    HeadlessImGui imgui;
    ImFont* main_font = ImGui::GetIO().FontDefault;
    ImFont* mono_font = hydra::ui::g_mono_font;
    REQUIRE(main_font != nullptr);
    REQUIRE(mono_font != nullptr);
    const float size = main_font->Sources[0]->SizePixels;
    const BackupFiles backups(size);

    int checked = 0;
    for (const ImWchar c : kSample) {
        const std::string file = backups.holder(c);
        if (file.empty()) {
            MESSAGE(hex(c) << " skipped: no backup font present here holds it (absent here: "
                           << backups.absent_list() << ")");
            continue;
        }
        ++checked;
        CAPTURE(file);
        INFO(hex(c) << " is in " << file);
        CHECK_MESSAGE(own_glyph(main_font, size, c) != nullptr, hex(c) << " is '?' in the main font");
        CHECK_MESSAGE(own_glyph(mono_font, size, c) != nullptr, hex(c) << " is '?' in the mono font");
    }
    // A machine with none of the backup fonts would otherwise pass having
    // checked nothing.
    CHECK_MESSAGE(checked > 0, "no sample character was checked: no backup font present here "
                               "holds any of them (absent here: "
                                   << backups.absent_list() << ")");
}

TEST_CASE("font coverage: U+2010 HYPHEN draws in the main font") {
    HeadlessImGui imgui;
    ImFont* main_font = ImGui::GetIO().FontDefault;
    REQUIRE(main_font != nullptr);
    const std::string segoe = hydra::ui::fallback_font_path("segoeui.ttf");
    REQUIRE_MESSAGE(hydra::file_exists_utf8(segoe),
                    segoe << " is absent here; this case needs Segoe UI");
    const float size = main_font->Sources[0]->SizePixels;
    CHECK(own_glyph(main_font, size, 0x2010) != nullptr);
}

TEST_CASE("font coverage: the main and mono fonts draw printable ASCII from their own files") {
    HeadlessImGui imgui;
    for (ImFont* font : {ImGui::GetIO().FontDefault, hydra::ui::g_mono_font}) {
        REQUIRE(font != nullptr);
        const float size = font->Sources[0]->SizePixels;
        for (ImWchar c = 0x21; c < 0x7F; ++c) {
            const ImFontGlyph* g = own_glyph(font, size, c);
            REQUIRE(g != nullptr);
            CHECK_MESSAGE(g->SourceIdx == 0, hex(c) << " comes from a backup font");
        }
    }
}

TEST_CASE("font coverage: the backups leave the main and mono fonts' ascent and descent alone") {
    HeadlessImGui imgui;
    for (ImFont* font : {ImGui::GetIO().FontDefault, hydra::ui::g_mono_font}) {
        REQUIRE(font != nullptr);
        const ImFontConfig* own = font->Sources[0];
        // The same file loaded alone, with no backups behind it.
        ImFontAtlas alone;
        ImFontConfig cfg;
        cfg.FontDataOwnedByAtlas = false;
        ImFont* bare = alone.AddFontFromMemoryTTF(own->FontData, own->FontDataSize,
                                                  own->SizePixels, &cfg);
        REQUIRE(bare != nullptr);
        const ImFontBaked* merged_baked = font->GetFontBaked(own->SizePixels);
        const ImFontBaked* bare_baked = bare->GetFontBaked(own->SizePixels);
        CHECK(merged_baked->Ascent == bare_baked->Ascent);
        CHECK(merged_baked->Descent == bare_baked->Descent);
    }
}
