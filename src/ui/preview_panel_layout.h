// The Preview panel's text layout numbers, which ui/preview_panel.cpp lays
// its overlay boxes and the Star Power gauge out by. They live in a header of
// their own, with no ImGui, so tests/test_overlay_layout.cpp reads the same
// numbers the panel uses. All are unscaled: the panel multiplies the pixel
// ones by px(1), and the text boxes' ones by the overlay's scale too.

#ifndef HYDRA_UI_PREVIEW_PANEL_LAYOUT_H
#define HYDRA_UI_PREVIEW_PANEL_LAYOUT_H

#include <cstddef>

namespace hydra::ui {

// A line of overlay text over its text size.
inline constexpr float kOverlayLineHeight = 1.25f;

// The next-activation box's notation line over its other lines' size. The
// user approved the ratio through the Preview mock (option B, D111), where the
// notation line is 17px and the box's other lines 12px.
inline constexpr float kNextActNotationScale = 17.0f / 12.0f;

// How many of the next-activation header's last words stay on one line, so
// its count ("2 of 3") never splits.
inline constexpr std::size_t kHeaderCountWords = 3;

// The gap between two stacked text boxes, in pixels.
inline constexpr float kOverlayPanelGap = 6.0f;

// The large type (the running score, the solo box's first line) over the
// time box's text size, and a line of it over its own size.
inline constexpr float kBigTypeScale = 1.8f;
inline constexpr float kBigTypeLineHeight = 1.2f;

// The Star Power gauge down the image's right edge, in pixels, not scaled
// with the text: its bar's width, its inset from the image's right edge, its
// margin from the image's top and bottom (where the right column starts too),
// and the gap between the drain box and the gauge.
inline constexpr float kGaugeBarWidth = 14.0f;
inline constexpr float kGaugeInset = 10.0f;
inline constexpr float kGaugeVMargin = 10.0f;
inline constexpr float kDrainGaugeGap = 6.0f;

}  // namespace hydra::ui

#endif  // HYDRA_UI_PREVIEW_PANEL_LAYOUT_H
