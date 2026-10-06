// The Preview's layout and look as numbers: a struct mirroring Onyx's
// 3d-config.yml, shipped as assets/preview/3d-config.json. That file is the
// one source of these numbers (docs/adr/0008): the struct holds none of its
// own, so a PreviewConfig that was not loaded is all zeros, and a key the file
// lacks is an error, never a fallback. Every camera, highway, light and timing
// constant the renderer uses comes from here — never from a literal in the
// draw code — so the port stays checkable against Onyx's file.
//
// Colours follow Onyx's `stackColor`: "#rrggbb" -> (r/255, g/255, b/255, 1).

#ifndef HYDRA_RENDER_PREVIEW_CONFIG_H
#define HYDRA_RENDER_PREVIEW_CONFIG_H

#include <string>
#include <string_view>

namespace hydra::render {

struct Color {
    float r = 1, g = 1, b = 1, a = 1;
};

struct Vec3 {
    float x = 0, y = 0, z = 0;
};

struct LightConfig {
    Vec3 position;
    Color ambient{};
    Color diffuse{};
    Color specular{};
};

struct PreviewConfig {
    struct View {
        Color background{};
        float track_fade_bottom{};
        float track_fade_top{};
        float height_width_ratio{};
        Vec3 camera_position{};
        float camera_rotate{};  // degrees down, around X
        float camera_fov{};     // degrees
        float camera_near{};
        float camera_far{};
    } view;

    struct Track {
        float y{};  // the floor
        float x_left{}, x_right{};  // the note area
        float z_past{}, z_now{}, z_future{};
        float secs_future{};  // events this far ahead sit at z_future
        Color color_normal{};
        Color color_solo{};
        Color railing_color{};
        float railing_x_width{};
        float railing_y_top{}, railing_y_bottom{};
        float beats_z_past{}, beats_z_future{};
        float targets_z_past{}, targets_z_future{};
        float targets_secs_light{};
        LightConfig light{};
    } track;

    struct Gems {
        Color color_hit{};
        float secs_fade{};
        LightConfig light{};  // relative to the gem's top centre
    } gems;

    // Onyx's text.time_box. Hydra draws the box in its own monospace font,
    // so the file's `font` key is not read; size and margin are.
    struct Text {
        float time_box_size{};
        float time_box_margin{};
    } text;

    // Hydra-only (not in Onyx's file).
    struct Hydra {
        int msaa{};  // mirrors Onyx's prefMSAA default
        Color sp_active_color{};
        float sp_active_darken{};
        // How dim a fill the path passed over draws, next to the taken one.
        float fill_offered_alpha{};
        // The mark where an active SP window ends (D81): a bright edge across
        // the floor, this deep along the highway, and an upright triangle
        // beside each railing, this wide (base to apex) and this tall (its
        // base). build_highway_draws places them.
        float sp_end_edge_depth{};
        float sp_end_marker_width{};
        float sp_end_marker_height{};
    } hydra;
};

// "#rrggbb" or "#rrggbbaa" -> Color; anything else returns opaque magenta.
Color parse_hex_color(std::string_view text);

// Parse the JSON form of 3d-config.yml. Every key the struct holds must be in
// the file with the right type (a number, a "#rrggbb" string, or an object):
// one that is absent or of another type throws std::runtime_error starting
// "3d-config.json: missing key " and naming the key's dotted path as the file
// spells it, for example "track.time.secs_future". Malformed JSON throws
// std::runtime_error too.
PreviewConfig load_preview_config(const std::string& json_text);

}  // namespace hydra::render

#endif  // HYDRA_RENDER_PREVIEW_CONFIG_H
