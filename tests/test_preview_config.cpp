// Tests for render/preview_config: the shipped JSON loads to Onyx's
// 3d-config.yml values, a key the file lacks (or holds with the wrong type) is
// refused by name, and colours parse like Onyx's stackColor.

#include "doctest.h"

#include <string>

#include "json.hpp"
#include "render/preview_config.h"
#include "preview_config_util.h"

using namespace hydra::render;

namespace {

void check_color(const Color& c, float r, float g, float b, float a = 1.0f) {
    CHECK(c.r == doctest::Approx(r));
    CHECK(c.g == doctest::Approx(g));
    CHECK(c.b == doctest::Approx(b));
    CHECK(c.a == doctest::Approx(a));
}

void check_light(const LightConfig& l, float x, float y, float z) {
    CHECK(l.position.x == doctest::Approx(x));
    CHECK(l.position.y == doctest::Approx(y));
    CHECK(l.position.z == doctest::Approx(z));
    check_color(l.ambient, 0x33 / 255.0f, 0x33 / 255.0f, 0x33 / 255.0f);
    check_color(l.diffuse, 1, 1, 1);
    check_color(l.specular, 1, 1, 1);
}

// Every field that Onyx's 3d-config.yml sets, checked against that file.
void check_is_onyx(const PreviewConfig& c) {
    check_color(c.view.background, 0x1c / 255.0f, 0x1d / 255.0f, 0x2b / 255.0f);
    CHECK(c.view.track_fade_bottom == doctest::Approx(0.7734724292101341));
    CHECK(c.view.track_fade_top == doctest::Approx(0.8315946348733234));
    CHECK(c.view.height_width_ratio == doctest::Approx(1.1666666666));
    CHECK(c.view.camera_position.x == doctest::Approx(0));
    CHECK(c.view.camera_position.y == doctest::Approx(1.4));
    CHECK(c.view.camera_position.z == doctest::Approx(3));
    CHECK(c.view.camera_rotate == doctest::Approx(25));
    CHECK(c.view.camera_fov == doctest::Approx(45));
    CHECK(c.view.camera_near == doctest::Approx(0.1));
    CHECK(c.view.camera_far == doctest::Approx(100));

    CHECK(c.track.y == doctest::Approx(-1));
    CHECK(c.track.x_left == doctest::Approx(-1));
    CHECK(c.track.x_right == doctest::Approx(1));
    CHECK(c.track.z_past == doctest::Approx(2));
    CHECK(c.track.z_now == doctest::Approx(0));
    CHECK(c.track.z_future == doctest::Approx(-12));
    CHECK(c.track.secs_future == doctest::Approx(1.35));
    check_color(c.track.color_normal, 0x33 / 255.0f, 0x33 / 255.0f, 0x33 / 255.0f);
    check_color(c.track.color_solo, 0x33 / 255.0f, 0x33 / 255.0f, 0x99 / 255.0f);
    check_color(c.track.railing_color, 0x73 / 255.0f, 0x73 / 255.0f, 0x73 / 255.0f);
    CHECK(c.track.railing_x_width == doctest::Approx(0.09));
    CHECK(c.track.railing_y_top == doctest::Approx(-0.85));
    CHECK(c.track.railing_y_bottom == doctest::Approx(-1.1));
    CHECK(c.track.beats_z_past == doctest::Approx(0.05));
    CHECK(c.track.beats_z_future == doctest::Approx(-0.05));
    CHECK(c.track.targets_z_past == doctest::Approx(0.13));
    CHECK(c.track.targets_z_future == doctest::Approx(-0.13));
    CHECK(c.track.targets_secs_light == doctest::Approx(0.1666666));
    check_light(c.track.light, 0, -0.5f, 0.5f);

    check_color(c.gems.color_hit, 1, 1, 1);
    CHECK(c.gems.secs_fade == doctest::Approx(0.1));
    check_light(c.gems.light, 0, 1, 0.2f);
    CHECK(c.text.time_box_size == doctest::Approx(15));
    CHECK(c.text.time_box_margin == doctest::Approx(10));
}

}  // namespace

TEST_CASE("the shipped 3d-config.json loads to the Onyx values") {
    std::string text = shipped_preview_config_text();
    REQUIRE(!text.empty());
    PreviewConfig c = load_preview_config(text);
    check_is_onyx(c);
    CHECK(c.hydra.msaa == 4);
    check_color(c.hydra.sp_active_color, 0x6c / 255.0f, 0xf7 / 255.0f, 0xc6 / 255.0f);
    CHECK(c.hydra.sp_active_darken == doctest::Approx(0.2));
    CHECK(c.hydra.fill_offered_alpha == doctest::Approx(0.35));
}

TEST_CASE("load_preview_config refuses a missing key and names it") {
    const nlohmann::json shipped = nlohmann::json::parse(shipped_preview_config_text());

    nlohmann::json no_secs = shipped;
    no_secs["track"]["time"].erase("secs_future");
    CHECK_THROWS_WITH_AS(load_preview_config(no_secs.dump()),
                         doctest::Contains("3d-config.json: missing key track.time.secs_future"),
                         std::runtime_error);

    nlohmann::json no_msaa = shipped;
    no_msaa["hydra"].erase("msaa");
    CHECK_THROWS_WITH_AS(load_preview_config(no_msaa.dump()),
                         doctest::Contains("3d-config.json: missing key hydra.msaa"),
                         std::runtime_error);

    // A key holding the wrong type is refused the same way.
    nlohmann::json bad_rotate = shipped;
    bad_rotate["view"]["camera"]["rotate"] = "25";
    CHECK_THROWS_WITH_AS(load_preview_config(bad_rotate.dump()),
                         doctest::Contains("3d-config.json: missing key view.camera.rotate"),
                         std::runtime_error);
}

TEST_CASE("load_preview_config rejects malformed JSON") {
    CHECK_THROWS(load_preview_config("{ not json"));
}

TEST_CASE("parse_hex_color follows Onyx stackColor") {
    check_color(parse_hex_color("#333399"), 0x33 / 255.0f, 0x33 / 255.0f, 0x99 / 255.0f);
    check_color(parse_hex_color("#FFFFFF"), 1, 1, 1);
    check_color(parse_hex_color("#00000080"), 0, 0, 0, 0x80 / 255.0f);
    check_color(parse_hex_color("nope"), 1, 0, 1);
}

TEST_CASE("load_preview_config reads the time box size and margin") {
    nlohmann::json j = nlohmann::json::parse(shipped_preview_config_text());
    j["text"]["time_box"]["size"] = 20;
    j["text"]["time_box"]["margin"] = 12;
    PreviewConfig c = load_preview_config(j.dump());
    CHECK(c.text.time_box_size == doctest::Approx(20));
    CHECK(c.text.time_box_margin == doctest::Approx(12));
}
