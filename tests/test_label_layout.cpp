// The timeline's label rule (ui/label_layout.h): no two drawn numbers
// overlap, the first always shows, and every label stays on the strip.

#include "doctest.h"

#include <optional>
#include <vector>

#include "ui/label_layout.h"

using hydra::ui::spaced_labels;

namespace {

// No two drawn labels come within `gap` of each other, and all sit inside.
void check_spaced(const std::vector<std::optional<float>>& out, const std::vector<float>& widths,
                  float left, float right, float gap) {
    std::optional<float> end;
    for (size_t i = 0; i < out.size(); ++i) {
        if (!out[i]) continue;
        CHECK(*out[i] >= left);
        CHECK(*out[i] + widths[i] <= right + 0.001f);
        if (end) CHECK(*out[i] >= *end + gap - 0.001f);
        end = *out[i] + widths[i];
    }
}

}  // namespace

TEST_CASE("label layout: labels with room all show, starting at their marks") {
    const std::vector<float> marks{50, 150, 250};
    const std::vector<float> widths{10, 10, 10};
    const auto out = spaced_labels(marks, widths, 0, 300, 4);
    REQUIRE(out.size() == 3);
    CHECK(*out[0] == doctest::Approx(50));
    CHECK(*out[1] == doctest::Approx(150));
    CHECK(*out[2] == doctest::Approx(250));
}

TEST_CASE("label layout: 39 crowded numbers never overlap, first and last show") {
    // Activation numbers on a 480 px strip, bunched like a long chart's: runs
    // of marks a few pixels apart.
    std::vector<float> marks, widths;
    float x = 2.0f;
    for (int i = 1; i <= 39; ++i) {
        x += (i % 5 == 0) ? 40.0f : 4.0f;
        marks.push_back(x);
        widths.push_back(i < 10 ? 6.0f : 12.0f);
    }
    const float right = 480;
    const float gap = 3;
    const auto out = spaced_labels(marks, widths, 0, right, gap);
    check_spaced(out, widths, 0, right, gap);
    CHECK(out.front().has_value());
    CHECK(out.back().has_value());

    // No label left out would have fit between its drawn neighbours. Every
    // mark here ends well inside the strip, so a label would start at its
    // mark.
    REQUIRE(marks.back() + widths.back() <= right);
    for (size_t i = 0; i < out.size(); ++i) {
        if (out[i]) continue;
        CAPTURE(i);
        std::optional<float> prev_end, next_start;
        for (size_t j = i; j-- > 0;)
            if (out[j]) {
                prev_end = *out[j] + widths[j];
                break;
            }
        for (size_t k = i + 1; k < out.size(); ++k)
            if (out[k]) {
                next_start = *out[k];
                break;
            }
        const bool clear_before = !prev_end || marks[i] >= *prev_end + gap;
        const bool clear_after = !next_start || marks[i] + widths[i] + gap <= *next_start;
        CHECK_FALSE((clear_before && clear_after));
    }
}

TEST_CASE("label layout: labels at the ends stay on the strip") {
    const float left = 0;
    const float right = 300;
    const std::vector<float> marks{left, right};
    const std::vector<float> widths{12, 12};
    const auto out = spaced_labels(marks, widths, left, right, 3);
    // The first starts at its mark on the left edge; the last, whose mark is
    // the right edge, moves left so it ends there.
    CHECK(*out[0] == doctest::Approx(left));
    CHECK(*out[1] == doctest::Approx(right - widths[1]));
}

TEST_CASE("label layout: the last label drops the ones it would cover, never the first") {
    const std::vector<float> marks{10, 50, 94, 100};
    const std::vector<float> widths{10, 10, 10, 10};
    const auto out = spaced_labels(marks, widths, 0, 200, 2);
    check_spaced(out, widths, 0, 200, 2);
    CHECK(out[0].has_value());
    CHECK(out[1].has_value());
    CHECK_FALSE(out[2].has_value());
    CHECK(out[3].has_value());

    // Two labels on top of each other: only the first.
    const auto two = spaced_labels({10, 11}, {10, 10}, 0, 200, 2);
    CHECK(two[0].has_value());
    CHECK_FALSE(two[1].has_value());
}
