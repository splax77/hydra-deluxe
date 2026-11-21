// Tests for render/preview_config: each key of the shipped JSON loads into
// its field, a key the file lacks (or holds with the wrong type) is refused
// by name, and colours parse like Onyx's stackColor.

#include "doctest.h"

#include <string>

#include "core/error_kind.h"
#include "json.hpp"
#include "render/preview_config.h"
#include "preview_config_util.h"

using namespace hydra::render;

namespace {

using nlohmann::json;

void check_color(const Color& c, float r, float g, float b, float a = 1.0f) {
    CHECK(c.r == doctest::Approx(r));
    CHECK(c.g == doctest::Approx(g));
    CHECK(c.b == doctest::Approx(b));
    CHECK(c.a == doctest::Approx(a));
}

// The number at a JSON pointer such as "/view/camera/fov", read by the
// test's own parse of the file, not by the loader.
double num_at(const json& j, const std::string& ptr) {
    INFO(ptr);
    return j.at(json::json_pointer(ptr)).get<double>();
}

// A colour key's text, turned into a colour by parse_hex_color (whose own
// rule is checked in "parse_hex_color follows Onyx stackColor").
void check_color_at(const Color& c, const json& j, const std::string& ptr) {
    INFO(ptr);
    const Color want = parse_hex_color(j.at(json::json_pointer(ptr)).get<std::string>());
    check_color(c, want.r, want.g, want.b, want.a);
}

void check_light_at(const LightConfig& l, const json& j, const std::string& ptr) {
    CHECK(l.position.x == doctest::Approx(num_at(j, ptr + "/position/x")));
    CHECK(l.position.y == doctest::Approx(num_at(j, ptr + "/position/y")));
    CHECK(l.position.z == doctest::Approx(num_at(j, ptr + "/position/z")));
    check_color_at(l.ambient, j, ptr + "/ambient");
    check_color_at(l.diffuse, j, ptr + "/diffuse");
    check_color_at(l.specular, j, ptr + "/specular");
}

}  // namespace

// Each field the loader fills equals the value at its own key in the file, so
// a key read into the wrong field fails here.
TEST_CASE("the shipped 3d-config.json loads each key into its field") {
    const std::string text = shipped_preview_config_text();
    REQUIRE(!text.empty());
    const PreviewConfig c = load_preview_config(text);
    const json j = json::parse(text);

    check_color_at(c.view.background, j, "/view/background");
    CHECK(c.view.track_fade_bottom == doctest::Approx(num_at(j, "/view/track_fade/bottom")));
    CHECK(c.view.track_fade_top == doctest::Approx(num_at(j, "/view/track_fade/top")));
    CHECK(c.view.height_width_ratio == doctest::Approx(num_at(j, "/view/height_width_ratio")));
    CHECK(c.view.camera_position.x == doctest::Approx(num_at(j, "/view/camera/position/x")));
    CHECK(c.view.camera_position.y == doctest::Approx(num_at(j, "/view/camera/position/y")));
    CHECK(c.view.camera_position.z == doctest::Approx(num_at(j, "/view/camera/position/z")));
    CHECK(c.view.camera_rotate == doctest::Approx(num_at(j, "/view/camera/rotate")));
    CHECK(c.view.camera_fov == doctest::Approx(num_at(j, "/view/camera/fov")));
    CHECK(c.view.camera_near == doctest::Approx(num_at(j, "/view/camera/near")));
    CHECK(c.view.camera_far == doctest::Approx(num_at(j, "/view/camera/far")));

    CHECK(c.track.y == doctest::Approx(num_at(j, "/track/y")));
    CHECK(c.track.x_left == doctest::Approx(num_at(j, "/track/note_area/x_left")));
    CHECK(c.track.x_right == doctest::Approx(num_at(j, "/track/note_area/x_right")));
    CHECK(c.track.z_past == doctest::Approx(num_at(j, "/track/time/z_past")));
    CHECK(c.track.z_now == doctest::Approx(num_at(j, "/track/time/z_now")));
    CHECK(c.track.z_future == doctest::Approx(num_at(j, "/track/time/z_future")));
    CHECK(c.track.secs_future == doctest::Approx(num_at(j, "/track/time/secs_future")));
    check_color_at(c.track.color_normal, j, "/track/color/normal");
    check_color_at(c.track.color_solo, j, "/track/color/solo");
    check_color_at(c.track.railing_color, j, "/track/railings/color");
    CHECK(c.track.railing_x_width == doctest::Approx(num_at(j, "/track/railings/x_width")));
    CHECK(c.track.railing_y_top == doctest::Approx(num_at(j, "/track/railings/y_top")));
    CHECK(c.track.railing_y_bottom == doctest::Approx(num_at(j, "/track/railings/y_bottom")));
    CHECK(c.track.beats_z_past == doctest::Approx(num_at(j, "/track/beats/z_past")));
    CHECK(c.track.beats_z_future == doctest::Approx(num_at(j, "/track/beats/z_future")));
    CHECK(c.track.targets_z_past == doctest::Approx(num_at(j, "/track/targets/z_past")));
    CHECK(c.track.targets_z_future == doctest::Approx(num_at(j, "/track/targets/z_future")));
    CHECK(c.track.targets_secs_light == doctest::Approx(num_at(j, "/track/targets/secs_light")));
    check_light_at(c.track.light, j, "/track/light");

    check_color_at(c.gems.color_hit, j, "/objects/gems/color_hit");
    CHECK(c.gems.secs_fade == doctest::Approx(num_at(j, "/objects/gems/secs_fade")));
    check_light_at(c.gems.light, j, "/objects/gems/light");
    CHECK(c.text.time_box_size == doctest::Approx(num_at(j, "/text/time_box/size")));
    CHECK(c.text.time_box_margin == doctest::Approx(num_at(j, "/text/time_box/margin")));

    CHECK(c.hydra.msaa == j.at(json::json_pointer("/hydra/msaa")).get<int>());
    check_color_at(c.hydra.sp_active_color, j, "/hydra/sp_active_color");
    CHECK(c.hydra.sp_active_darken == doctest::Approx(num_at(j, "/hydra/sp_active_darken")));
    CHECK(c.hydra.fill_offered_alpha == doctest::Approx(num_at(j, "/hydra/fill_offered_alpha")));
}

// The hydra section holds only keys the loader reads, plus its "_comment":
// with any one of them erased, the loader refuses the file by that key's
// name. A key left behind by an old feature, which nothing reads, fails here.
TEST_CASE("every key in the shipped hydra section is one the loader needs") {
    const json shipped = json::parse(shipped_preview_config_text());
    REQUIRE(shipped.at("hydra").is_object());
    REQUIRE(!shipped.at("hydra").empty());
    for (const auto& kv : shipped.at("hydra").items()) {
        if (kv.key() == "_comment") continue;
        json without = shipped;
        without["hydra"].erase(kv.key());
        const std::string want = "3d-config.json: missing key hydra." + kv.key();
        CHECK_THROWS_WITH_AS(load_preview_config(without.dump()), doctest::Contains(want.c_str()),
                             std::runtime_error);
    }
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

// A file that is not JSON is refused as a Preview assets error naming the
// file, the same way a missing key is, not as the JSON library's own error.
TEST_CASE("load_preview_config rejects malformed JSON") {
    bool refused = false;
    try {
        load_preview_config("{ not json");
    } catch (const hydra::KindedError& e) {
        refused = true;
        CHECK(e.kind() == hydra::ErrorKind::PreviewAssets);
        CHECK(std::string(e.what()).rfind("3d-config.json: ", 0) == 0);
    }
    CHECK(refused);
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
