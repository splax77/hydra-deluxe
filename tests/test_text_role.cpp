// Unit tests for font_for (ui/text_role.h), the one rule for which font and
// size each kind of text draws in. The table below pins every role's font and
// size as literals; it never asks font_for to work out what it expects.

#include "doctest.h"

#include <cfloat>
#include <cmath>
#include <cstddef>
#include <iterator>

#include "imgui.h"

#include "headless_imgui_fixture.h"
#include "ui/fonts.h"
#include "ui/text_role.h"

using hydra::ui::FontPick;
using hydra::ui::TextRole;
using hydra::ui::font_for;

namespace {

using hydra::test::HeadlessImGui;

enum class Face { Mono, Default };

struct Row {
    TextRole role;
    Face face;
    float size;  // 0 keeps the current size
};

// One row per role: the font it draws in and the size it pushes.
const Row kRows[] = {
    {TextRole::Notation, Face::Mono, 0.0f},
    {TextRole::Figure, Face::Mono, 0.0f},
    {TextRole::Label, Face::Default, 0.0f},
    {TextRole::Prose, Face::Default, 0.0f},
    {TextRole::Title, Face::Default, 0.0f},
    {TextRole::Control, Face::Default, 0.0f},
    {TextRole::Overlay, Face::Mono, 0.0f},
    {TextRole::SongTitle, Face::Default, 26.0f},
    {TextRole::HeadlineScore, Face::Default, 40.0f},
    {TextRole::HeadlinePath, Face::Mono, 24.0f},
    {TextRole::DialogQuestion, Face::Default, 20.0f},
};

}  // namespace

TEST_CASE("text_role: every role draws in its font at its size") {
    REQUIRE(std::size(kRows) == hydra::ui::kTextRoleCount);
    for (std::size_t i = 0; i < std::size(kRows); ++i) {
        CHECK(static_cast<std::size_t>(kRows[i].role) == i);  // one row per role, in order
    }

    HeadlessImGui imgui;
    ImFont* const mono = hydra::ui::g_mono_font;
    ImFont* const deflt = ImGui::GetIO().FontDefault;
    REQUIRE(mono != nullptr);
    REQUIRE(deflt != nullptr);
    REQUIRE(mono != deflt);  // else the table could not tell the faces apart

    ImGui::NewFrame();
    // Inside a mono scope, so a default-font role must switch back.
    ImGui::PushFont(mono, 0.0f);
    for (const Row& row : kRows) {
        CAPTURE(static_cast<int>(row.role));
        ImFont* const want = row.face == Face::Mono ? mono : deflt;
        const FontPick pick = font_for(row.role);
        CHECK(pick.font == want);
        CHECK(pick.size == row.size);
        {
            hydra::ui::ScopedFont scope(row.role);
            CHECK(ImGui::GetFont() == want);
        }
        CHECK(ImGui::GetFont() == mono);  // the scope popped its font
    }
    ImGui::PopFont();
    ImGui::EndFrame();
}

TEST_CASE("text_role: text_size and measure_in_role measure in the role's font at its size") {
    HeadlessImGui imgui;
    ImFont* const mono = hydra::ui::g_mono_font;
    ImFont* const deflt = ImGui::GetIO().FontDefault;
    ImGui::NewFrame();
    ImGui::PushFont(deflt, 18.0f);

    // HeadlinePath is mono at 24 px; CalcTextSizeA on that font and size is the
    // independent measure.
    const char* const text = "3- 1 2 E3+";
    const float want = mono->CalcTextSizeA(24.0f, FLT_MAX, 0.0f, text).x;
    CHECK(hydra::ui::measure_in_role(TextRole::HeadlinePath)(text) == doctest::Approx(want));
    // ImGui::CalcTextSize rounds the width up to a whole pixel.
    CHECK(hydra::ui::text_size(TextRole::HeadlinePath, text).x == std::ceil(want));
    CHECK(hydra::ui::text_size(TextRole::HeadlinePath, text).y > 0.0f);
    CHECK(ImGui::GetFont() == deflt);  // both popped their scope
    CHECK(ImGui::GetFontSize() == 18.0f);

    // A size-0 role measures at the size in force.
    const float want_notation = mono->CalcTextSizeA(18.0f, FLT_MAX, 0.0f, text).x;
    CHECK(hydra::ui::measure_in_role(TextRole::Notation)(text) == doctest::Approx(want_notation));

    ImGui::PopFont();
    ImGui::EndFrame();
}

TEST_CASE("text_role: measure_in_role with a size measures in the role's font at that size") {
    HeadlessImGui imgui;
    ImFont* const mono = hydra::ui::g_mono_font;
    ImFont* const deflt = ImGui::GetIO().FontDefault;
    ImGui::NewFrame();
    ImGui::PushFont(deflt, 18.0f);

    // Figure is mono; 9 px is neither the size in force nor the role's own.
    const char* const text = "12 Wiii";
    const float want = mono->CalcTextSizeA(9.0f, FLT_MAX, 0.0f, text).x;
    CHECK(hydra::ui::measure_in_role(TextRole::Figure, 9.0f)(text) == doctest::Approx(want));
    CHECK(want != doctest::Approx(deflt->CalcTextSizeA(9.0f, FLT_MAX, 0.0f, text).x));
    CHECK(ImGui::GetFont() == deflt);
    CHECK(ImGui::GetFontSize() == 18.0f);

    ImGui::PopFont();
    ImGui::EndFrame();
}

TEST_CASE("text_role: with no mono font, mono roles take the current font") {
    HeadlessImGui imgui;
    ImFont* const loaded_mono = hydra::ui::g_mono_font;
    ImFont* const deflt = ImGui::GetIO().FontDefault;
    REQUIRE(loaded_mono != nullptr);
    REQUIRE(loaded_mono != deflt);
    hydra::ui::g_mono_font = nullptr;

    ImGui::NewFrame();
    // The current font is one the default-font roles never pick, so a mono
    // role that returned the default font would fail here.
    ImGui::PushFont(loaded_mono, 0.0f);
    ImFont* const current = ImGui::GetFont();
    for (const Row& row : kRows) {
        CAPTURE(static_cast<int>(row.role));
        const FontPick pick = font_for(row.role);
        CHECK(pick.font != nullptr);
        CHECK(pick.font == (row.face == Face::Mono ? current : deflt));
    }
    ImGui::PopFont();
    ImGui::EndFrame();
}
