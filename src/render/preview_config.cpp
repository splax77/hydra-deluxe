#include "render/preview_config.h"

#include <cstdlib>
#include <stdexcept>

#include "core/error_kind.h"
#include "json.hpp"

namespace hydra::render {

using json = nlohmann::json;

namespace {

// What a malformed colour draws as: opaque magenta, so the mistake shows
// (docs/adr/0008).
constexpr Color kBadColor{1, 0, 1, 1};

}  // namespace

Color parse_hex_color(std::string_view text) {
    auto hex = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    if (text.empty() || text[0] != '#' || (text.size() != 7 && text.size() != 9))
        return kBadColor;
    float out[4] = {0, 0, 0, 1};
    for (size_t i = 0; i + 1 < text.size(); i += 2) {
        int hi = hex(text[1 + i]), lo = hex(text[2 + i]);
        if (hi < 0 || lo < 0) return kBadColor;
        out[i / 2] = static_cast<float>(hi * 16 + lo) / 255.0f;
    }
    return Color{out[0], out[1], out[2], out[3]};
}

namespace {

// A JSON object and its dotted path from the file's root ("" for the root).
// The file holds every number the Preview draws with, so each read below
// needs its key: one that is absent, or holds another type, is refused by
// its dotted path.
struct Node {
    const json& j;
    std::string path;
};

std::string key_path(const Node& n, const char* key) {
    return n.path.empty() ? std::string(key) : n.path + "." + key;
}

bool is_number(const json& v) { return v.is_number(); }
bool is_string(const json& v) { return v.is_string(); }
bool is_object(const json& v) { return v.is_object(); }

const json& need(const Node& n, const char* key, bool (*ok)(const json&)) {
    if (n.j.is_object()) {
        const auto it = n.j.find(key);
        if (it != n.j.end() && ok(*it)) return *it;
    }
    throw KindedError(ErrorKind::PreviewAssets, "3d-config.json: missing key " + key_path(n, key));
}

Node sub(const Node& n, const char* key) { return {need(n, key, is_object), key_path(n, key)}; }

float get_f(const Node& n, const char* key) { return need(n, key, is_number).get<float>(); }

int get_i(const Node& n, const char* key) { return need(n, key, is_number).get<int>(); }

Color get_color(const Node& n, const char* key) {
    return parse_hex_color(need(n, key, is_string).get<std::string>());
}

Vec3 get_vec3(const Node& n, const char* key) {
    const Node v = sub(n, key);
    return Vec3{get_f(v, "x"), get_f(v, "y"), get_f(v, "z")};
}

LightConfig get_light(const Node& n, const char* key) {
    const Node l = sub(n, key);
    LightConfig out;
    out.position = get_vec3(l, "position");
    out.ambient = get_color(l, "ambient");
    out.diffuse = get_color(l, "diffuse");
    out.specular = get_color(l, "specular");
    return out;
}

}  // namespace

PreviewConfig load_preview_config(const std::string& json_text) {
    json parsed;
    try {
        parsed = json::parse(json_text);
    } catch (const json::exception& e) {
        throw KindedError(ErrorKind::PreviewAssets, std::string("3d-config.json: ") + e.what());
    }
    const Node root{parsed, ""};
    PreviewConfig c;

    const Node view = sub(root, "view");
    c.view.background = get_color(view, "background");
    const Node fade = sub(view, "track_fade");
    c.view.track_fade_bottom = get_f(fade, "bottom");
    c.view.track_fade_top = get_f(fade, "top");
    c.view.height_width_ratio = get_f(view, "height_width_ratio");
    const Node cam = sub(view, "camera");
    c.view.camera_position = get_vec3(cam, "position");
    c.view.camera_rotate = get_f(cam, "rotate");
    c.view.camera_fov = get_f(cam, "fov");
    c.view.camera_near = get_f(cam, "near");
    c.view.camera_far = get_f(cam, "far");

    const Node track = sub(root, "track");
    c.track.y = get_f(track, "y");
    const Node area = sub(track, "note_area");
    c.track.x_left = get_f(area, "x_left");
    c.track.x_right = get_f(area, "x_right");
    const Node time = sub(track, "time");
    c.track.z_past = get_f(time, "z_past");
    c.track.z_now = get_f(time, "z_now");
    c.track.z_future = get_f(time, "z_future");
    c.track.secs_future = get_f(time, "secs_future");
    const Node color = sub(track, "color");
    c.track.color_normal = get_color(color, "normal");
    c.track.color_solo = get_color(color, "solo");
    const Node rail = sub(track, "railings");
    c.track.railing_color = get_color(rail, "color");
    c.track.railing_x_width = get_f(rail, "x_width");
    c.track.railing_y_top = get_f(rail, "y_top");
    c.track.railing_y_bottom = get_f(rail, "y_bottom");
    const Node beats = sub(track, "beats");
    c.track.beats_z_past = get_f(beats, "z_past");
    c.track.beats_z_future = get_f(beats, "z_future");
    const Node targets = sub(track, "targets");
    c.track.targets_z_past = get_f(targets, "z_past");
    c.track.targets_z_future = get_f(targets, "z_future");
    c.track.targets_secs_light = get_f(targets, "secs_light");
    c.track.light = get_light(track, "light");

    const Node gems = sub(sub(root, "objects"), "gems");
    c.gems.color_hit = get_color(gems, "color_hit");
    c.gems.secs_fade = get_f(gems, "secs_fade");
    c.gems.light = get_light(gems, "light");

    // text.time_box.font is not read: Hydra draws the box in its own font.
    const Node time_box = sub(sub(root, "text"), "time_box");
    c.text.time_box_size = get_f(time_box, "size");
    c.text.time_box_margin = get_f(time_box, "margin");

    const Node hy = sub(root, "hydra");
    c.hydra.msaa = get_i(hy, "msaa");
    c.hydra.sp_active_color = get_color(hy, "sp_active_color");
    c.hydra.sp_active_darken = get_f(hy, "sp_active_darken");
    c.hydra.fill_offered_alpha = get_f(hy, "fill_offered_alpha");

    return c;
}

}  // namespace hydra::render
