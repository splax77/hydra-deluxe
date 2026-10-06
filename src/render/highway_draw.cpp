#include "render/highway_draw.h"

#include <algorithm>
#include <cmath>
#include <optional>

#include "app/preview_view.h"  // struck_at

using namespace DirectX;

namespace hydra::render {

const char* texture_file(TextureId id) {
    switch (id) {
        case TextureId::BoxRed:            return "box-red.png";
        case TextureId::BoxYellow:         return "box-yellow.png";
        case TextureId::BoxBlue:           return "box-blue.png";
        case TextureId::BoxGreen:          return "box-green.png";
        case TextureId::BoxEnergy:         return "box-energy.png";
        case TextureId::CymbalYellow:      return "cymbal-yellow.png";
        case TextureId::CymbalBlue:        return "cymbal-blue.png";
        case TextureId::CymbalGreen:       return "cymbal-green.png";
        case TextureId::CymbalEnergy:      return "cymbal-energy.png";
        case TextureId::LongKick:          return "long-kick.jpg";
        case TextureId::LongEnergy:        return "long-energy.jpg";
        case TextureId::OverlayGhost:      return "overlay-ghost.png";
        case TextureId::OverlayAccent:     return "overlay-accent.png";
        case TextureId::Line1:             return "line-1.png";
        case TextureId::Line2:             return "line-2.png";
        case TextureId::Line3:             return "line-3.png";
        case TextureId::TargetRed:         return "target-red.png";
        case TextureId::TargetYellow:      return "target-yellow.png";
        case TextureId::TargetBlue:        return "target-blue.png";
        case TextureId::TargetGreen:       return "target-green.png";
        case TextureId::TargetRedLight:    return "target-red-light.png";
        case TextureId::TargetYellowLight: return "target-yellow-light.png";
        case TextureId::TargetBlueLight:   return "target-blue-light.png";
        case TextureId::TargetGreenLight:  return "target-green-light.png";
        case TextureId::LaneRed:           return "lane-red.png";
        case TextureId::LaneYellow:        return "lane-yellow.png";
        case TextureId::LaneBlue:          return "lane-blue.png";
        case TextureId::LaneGreen:         return "lane-green.png";
        case TextureId::None:
        case TextureId::Count:             break;
    }
    return "";
}

HighwayCamera make_camera(const PreviewConfig& cfg, float aspect) {
    HighwayCamera cam;
    const Vec3 p = cfg.view.camera_position;
    // Onyx: view = rotateX(tilt) . translate(-pos) on column vectors, i.e.
    // translate first, then rotate — the same order with row vectors.
    XMMATRIX view = XMMatrixTranslation(-p.x, -p.y, -p.z) *
                    XMMatrixRotationX(XMConvertToRadians(cfg.view.camera_rotate));
    XMMATRIX proj = XMMatrixPerspectiveFovRH(XMConvertToRadians(cfg.view.camera_fov),
                                             aspect, cfg.view.camera_near,
                                             cfg.view.camera_far);
    XMStoreFloat4x4(&cam.view, view);
    XMStoreFloat4x4(&cam.proj, proj);
    cam.view_pos = p;
    return cam;
}

TrackRect track_rect(const PreviewConfig& cfg, int width, int height) {
    TrackRect r;
    r.width = std::max(1, width);
    r.height = std::max(1, height);
    const int t =
        std::min(r.height, static_cast<int>(std::lround(r.width * cfg.view.height_width_ratio)));
    r.track_height = std::max(1, t);
    r.top = r.height - r.track_height;
    r.aspect = static_cast<float>(r.width) / static_cast<float>(r.track_height);
    return r;
}

int track_height(const PreviewConfig& cfg, int width, int height) {
    return track_rect(cfg, width, height).track_height;
}

double far_time(const PreviewConfig& cfg, double now_s, double speed) {
    return now_s + speed * cfg.track.secs_future;
}

double time_to_z(const PreviewConfig& cfg, double now_s, double t_s, double speed) {
    const double far_t = far_time(cfg, now_s, speed);
    return cfg.track.z_now +
           (cfg.track.z_future - cfg.track.z_now) * ((t_s - now_s) / (far_t - now_s));
}

double z_to_time(const PreviewConfig& cfg, double now_s, double z, double speed) {
    const double far_t = far_time(cfg, now_s, speed);
    return now_s + (far_t - now_s) * ((z - cfg.track.z_now) /
                                      (cfg.track.z_future - cfg.track.z_now));
}

void pad_x(const PreviewConfig& cfg, Pad pad, float& x1, float& x2) {
    const float w = (cfg.track.x_right - cfg.track.x_left) / 4.0f;
    x1 = cfg.track.x_left + static_cast<int>(pad) * w;
    x2 = x1 + w;
}

void railing_x(const PreviewConfig& cfg, bool right, float& x1, float& x2) {
    const PreviewConfig::Track& T = cfg.track;
    x1 = right ? T.x_right : T.x_left - T.railing_x_width;
    x2 = right ? T.x_right + T.railing_x_width : T.x_left;
}

float railing_outer_x(const PreviewConfig& cfg, bool right) {
    float x1, x2;
    railing_x(cfg, right, x1, x2);
    return right ? x2 : x1;
}

XMMATRIX stretch_matrix(const DrawCommand& cmd) {
    float sx = std::fabs(cmd.hi[0] - cmd.lo[0]);
    float sy = std::fabs(cmd.hi[1] - cmd.lo[1]);
    float sz = std::fabs(cmd.hi[2] - cmd.lo[2]);
    if (sy == 0.0f) sy = 1.0f;  // Onyx Flat: yScale 1
    const float cx = (cmd.lo[0] + cmd.hi[0]) * 0.5f;
    const float cy = (cmd.lo[1] + cmd.hi[1]) * 0.5f;
    const float cz = (cmd.lo[2] + cmd.hi[2]) * 0.5f;
    return XMMatrixScaling(sx, sy, sz) * XMMatrixTranslation(cx, cy, cz);
}

LightConfig light_for(const PreviewConfig& cfg, const DrawCommand& cmd) {
    if (cmd.light == LightKind::Global) return cfg.track.light;
    if (cmd.light == LightKind::Unlit) {
        LightConfig l;
        l.ambient = Color{1.0f, 1.0f, 1.0f, 1.0f};
        l.diffuse = Color{0.0f, 0.0f, 0.0f, 0.0f};
        l.specular = Color{0.0f, 0.0f, 0.0f, 0.0f};
        return l;
    }
    // Onyx LightOffset: relative to the box's top centre.
    LightConfig l = cfg.gems.light;
    l.position.x += (cmd.lo[0] + cmd.hi[0]) * 0.5f;
    l.position.y += std::max(cmd.lo[1], cmd.hi[1]);
    l.position.z += (cmd.lo[2] + cmd.hi[2]) * 0.5f;
    return l;
}

namespace {

DrawCommand flat(float x1, float y, float z1, float x2, float z2, Material mat,
                 float alpha = 1.0f, DepthMode depth = DepthMode::Less) {
    DrawCommand c;
    c.mesh = MeshId::Flat;
    c.lo[0] = x1; c.lo[1] = y; c.lo[2] = z1;
    c.hi[0] = x2; c.hi[1] = y; c.hi[2] = z2;
    c.material = mat;
    c.alpha = alpha;
    c.light = LightKind::Global;
    c.depth = depth;
    return c;
}

Material color_mat(Color c) {
    Material m;
    m.kind = MaterialKind::Color;
    m.color = c;
    return m;
}

Material tex_mat(TextureId t) {
    Material m;
    m.kind = MaterialKind::Texture;
    m.texture = t;
    return m;
}

Material overlay_mat(TextureId base, TextureId overlay) {
    Material m;
    m.kind = MaterialKind::TextureOverlay;
    m.texture = base;
    m.overlay = overlay;
    return m;
}

// Gem sizes from Onyx's drawDrumPlay / drawGem (mtolly/onyx,
// haskell/packages/onyx-lib-game/src/Onyx/Game/Graphics.hs, lines 660-668 at
// commit 84d5e51). They are literals in Onyx's code, not keys in its
// 3d-config.yml, so they live here rather than in PreviewConfig.
constexpr float kGhostWidthScale = 0.7f;        // Onyx: (v - xCenter) * 0.7
constexpr float kPadGemHalfSize = 0.5f / 2.0f;  // Onyx: reference = 0.5 / 2

TextureId lane_tex(Pad p) {
    switch (p) {
        case Pad::Red:    return TextureId::LaneRed;
        case Pad::Yellow: return TextureId::LaneYellow;
        case Pad::Blue:   return TextureId::LaneBlue;
        case Pad::Green:  return TextureId::LaneGreen;
    }
    return TextureId::LaneRed;
}

TextureId target_tex(Pad p, bool light) {
    switch (p) {
        case Pad::Red:    return light ? TextureId::TargetRedLight : TextureId::TargetRed;
        case Pad::Yellow: return light ? TextureId::TargetYellowLight : TextureId::TargetYellow;
        case Pad::Blue:   return light ? TextureId::TargetBlueLight : TextureId::TargetBlue;
        case Pad::Green:  return light ? TextureId::TargetGreenLight : TextureId::TargetGreen;
    }
    return TextureId::TargetRed;
}

// Onyx drawGem's texture table (no lefty flip).
TextureId gem_tex(const TrackGem& g, bool od) {
    if (g.kick) return od ? TextureId::LongEnergy : TextureId::LongKick;
    if (g.cymbal) {
        if (od) return TextureId::CymbalEnergy;
        switch (g.pad) {
            case Pad::Yellow: return TextureId::CymbalYellow;
            case Pad::Blue:   return TextureId::CymbalBlue;
            case Pad::Green:  return TextureId::CymbalGreen;
            case Pad::Red:    break;  // never a cymbal
        }
    }
    if (od) return TextureId::BoxEnergy;
    switch (g.pad) {
        case Pad::Red:    return TextureId::BoxRed;
        case Pad::Yellow: return TextureId::BoxYellow;
        case Pad::Blue:   return TextureId::BoxBlue;
        case Pad::Green:  return TextureId::BoxGreen;
    }
    return TextureId::BoxRed;
}

Color blend(const Color& a, const Color& b) {
    return Color{(a.r + b.r) * 0.5f, (a.g + b.g) * 0.5f, (a.b + b.b) * 0.5f, 1.0f};
}

}  // namespace

std::vector<DrawCommand> build_highway_draws(const TrackState& state, const PreviewConfig& cfg,
                                             double now_s, double speed) {
    std::vector<DrawCommand> out;
    const PreviewConfig::Track& T = cfg.track;
    const double far_t = far_time(cfg, now_s, speed);
    const double near_t = z_to_time(cfg, now_s, T.z_past, speed);
    auto z_of = [&](double t) { return static_cast<float>(time_to_z(cfg, now_s, t, speed)); };

    const TrackWindow win = state.window(near_t, far_t);
    // Is the note at `t_s` struck at the playhead? The score box asks the
    // same predicate in ms, so a chord on the playhead is lit here and counted
    // there (D48, Q27). The highway works in seconds; convert at the call.
    auto struck = [now_s](double t_s) { return app::struck_at(now_s * 1000.0, t_s * 1000.0); };

    // 1. Floor: one flat per stretch of (solo, active SP) state. Onyx tints
    //    the floor for solos only; the active SP window is Hydra's addition,
    //    drawn the same way in the energy colour darkened to floor brightness.
    {
        std::vector<ToggleSpan> solos = state.make_toggle_bounds(win, near_t, far_t, &TrackInstant::solo);
        std::vector<ToggleSpan> sps = state.make_toggle_bounds(win, near_t, far_t, &TrackInstant::sp_active);
        std::vector<double> cuts{near_t, far_t};
        for (const ToggleSpan& s : solos) { cuts.push_back(s.t1); cuts.push_back(s.t2); }
        for (const ToggleSpan& s : sps) { cuts.push_back(s.t1); cuts.push_back(s.t2); }
        std::sort(cuts.begin(), cuts.end());
        cuts.erase(std::unique(cuts.begin(), cuts.end()), cuts.end());
        auto state_at = [](const std::vector<ToggleSpan>& spans, double t) {
            for (const ToggleSpan& s : spans)
                if (s.t1 <= t && t < s.t2) return s.on;
            return false;
        };
        const Color sp_col{cfg.hydra.sp_active_color.r * cfg.hydra.sp_active_darken,
                           cfg.hydra.sp_active_color.g * cfg.hydra.sp_active_darken,
                           cfg.hydra.sp_active_color.b * cfg.hydra.sp_active_darken, 1.0f};
        for (size_t i = 0; i + 1 < cuts.size(); ++i) {
            const double t1 = cuts[i], t2 = cuts[i + 1];
            if (t2 <= t1) continue;
            const double mid = (t1 + t2) * 0.5;
            const bool solo = state_at(solos, mid), sp = state_at(sps, mid);
            Color col = solo ? T.color_solo : T.color_normal;
            if (sp) col = solo ? blend(T.color_solo, sp_col) : sp_col;
            out.push_back(flat(T.x_left, T.y, z_of(t1), T.x_right, z_of(t2), color_mat(col)));
        }
    }

    // 2. Railings: two boxes along the whole visible depth.
    for (bool right : {false, true}) {
        DrawCommand rail;
        rail.mesh = MeshId::Box;
        railing_x(cfg, right, rail.lo[0], rail.hi[0]);
        rail.lo[1] = T.railing_y_top;    rail.lo[2] = T.z_past;
        rail.hi[1] = T.railing_y_bottom; rail.hi[2] = T.z_future;
        rail.material = color_mat(T.railing_color);
        out.push_back(rail);
    }

    // 3. Beat lines (depth test off): bar / beat / half-beat flats.
    for (const TrackInstant& inst : win) {
        if (!inst.beat) continue;
        TextureId tex = *inst.beat == app::PreviewBeatKind::Bar    ? TextureId::Line1
                        : *inst.beat == app::PreviewBeatKind::Beat ? TextureId::Line2
                                                                   : TextureId::Line3;
        const float z = z_of(inst.t);
        out.push_back(flat(T.x_left, T.y, z + T.beats_z_past, T.x_right, z + T.beats_z_future,
                           tex_mat(tex), 1.0f, DepthMode::Always));
    }

    // 4. Lane strips (depth off): a fill window lights all four lanes (Onyx's
    //    BRE look), dimmed to fill_offered_alpha for a fill the path passed
    //    over and at full strength for the one it activates on. That fill's
    //    activation lane then draws once more in the lit target texture, so
    //    the lane to hit stands out from the other three.
    {
        auto strip_tex = [&](Pad pad, double t1, double t2, TextureId tex, float alpha) {
            float x1, x2;
            pad_x(cfg, pad, x1, x2);
            out.push_back(flat(x1, T.y, z_of(t1), x2, z_of(t2), tex_mat(tex), alpha,
                               DepthMode::Always));
        };
        auto strip = [&](Pad pad, double t1, double t2, float alpha) {
            strip_tex(pad, t1, t2, lane_tex(pad), alpha);
        };
        for (const ToggleSpan& s : state.make_toggle_bounds(win, near_t, far_t, &TrackInstant::fill)) {
            if (!s.on) continue;
            for (Pad p : {Pad::Red, Pad::Yellow, Pad::Blue, Pad::Green})
                strip(p, s.t1, s.t2, cfg.hydra.fill_offered_alpha);
        }
        for (const ToggleSpan& s :
             state.make_toggle_bounds(win, near_t, far_t, &TrackInstant::fill_taken)) {
            if (!s.on) continue;
            for (Pad p : {Pad::Red, Pad::Yellow, Pad::Blue, Pad::Green}) strip(p, s.t1, s.t2, 1.0f);
        }
        // Each lit stretch carries its own pad, so two taken fills that touch
        // each light their own lane (D53 item 3).
        for (const LaneSpan& s : state.make_lane_bounds(win, near_t, far_t))
            strip_tex(s.pad, s.t1, s.t2, target_tex(s.pad, true), 1.0f);
    }

    // 5. Strike line (Onyx targets) and the glow after a hit, depth off.
    for (Pad p : {Pad::Red, Pad::Yellow, Pad::Blue, Pad::Green}) {
        float x1, x2;
        pad_x(cfg, p, x1, x2);
        out.push_back(flat(x1, T.y, T.z_now + T.targets_z_past, x2, T.z_now + T.targets_z_future,
                           tex_mat(target_tex(p, false)), 1.0f, DepthMode::Always));
    }
    {
        // Each lane glows from its most recent hit only; a kick lights nothing.
        bool lit[4] = {false, false, false, false};
        for (auto it = win.rbegin(); it != win.rend(); ++it) {
            if (!struck(it->t) || it->t <= near_t) continue;
            const float alpha = static_cast<float>(1.0 - (now_s - it->t) / T.targets_secs_light);
            if (alpha <= 0.0f) continue;
            for (const TrackGem& g : it->notes) {
                if (g.kick || lit[static_cast<int>(g.pad)]) continue;
                lit[static_cast<int>(g.pad)] = true;
                float x1, x2;
                pad_x(cfg, g.pad, x1, x2);
                out.push_back(flat(x1, T.y, T.z_now + T.targets_z_past, x2,
                                   T.z_now + T.targets_z_future, tex_mat(target_tex(g.pad, true)),
                                   alpha, DepthMode::Always));
            }
        }
    }

    // 6. SP end marks (Hydra, D81), depth off: a gem centred near an active
    //    window's end hides the tint's edge, so each end visible on the
    //    highway gets a bright edge across the floor and a triangle beside
    //    each railing, where no gem sits. They draw before the gems, so a gem
    //    still covers the edge. The ends are TrackState::sp_active_ends.
    //    Edge and triangles are unlit, so both show the mark's own colour
    //    (sp_end_color, D84) as given and match. The triangles stand upright
    //    facing the camera. Each one's base rests on its railing's outer
    //    edge and its apex points away from the lanes, as in the approved
    //    mock; it sits at floor height at the end's depth, so on screen it is
    //    level with the edge.
    {
        const Material mark = color_mat(cfg.hydra.sp_end_color);
        const float half_edge = cfg.hydra.sp_end_edge_depth * 0.5f;
        const float marker_w = cfg.hydra.sp_end_marker_width;
        const float half_marker_h = cfg.hydra.sp_end_marker_height * 0.5f;
        for (double t_end : state.sp_active_ends(win)) {
            const float z = z_of(t_end);
            DrawCommand edge = flat(T.x_left, T.y, z + half_edge, T.x_right, z - half_edge, mark,
                                    1.0f, DepthMode::Always);
            edge.light = LightKind::Unlit;
            out.push_back(edge);
            for (bool right : {false, true}) {
                const float outer = railing_outer_x(cfg, right);
                DrawCommand n;
                n.mesh = right ? MeshId::TriangleRight : MeshId::TriangleLeft;
                n.lo[0] = right ? outer : outer - marker_w;
                n.hi[0] = right ? outer + marker_w : outer;
                n.lo[1] = T.y - half_marker_h;
                n.hi[1] = T.y + half_marker_h;
                // The mesh is flat at its own z = 0, so this depth only keeps
                // the model matrix invertible for the normal matrix.
                n.lo[2] = z + half_edge;
                n.hi[2] = z - half_edge;
                n.material = mark;
                n.light = LightKind::Unlit;
                n.depth = DepthMode::Always;
                out.push_back(n);
            }
        }
    }

    // 7. Gems, latest (farthest) first. A struck note (on or past the strike
    //    line) flashes white there and fades over secs_fade.
    for (auto it = win.rbegin(); it != win.rend(); ++it) {
        const bool od = toggle_on_after(it->overdrive);
        std::optional<float> fade;
        if (struck(it->t)) {
            const double age = now_s - it->t;
            if (age >= cfg.gems.secs_fade) continue;
            fade = static_cast<float>(1.0 - age / cfg.gems.secs_fade);
        }
        const float z = fade ? z_of(now_s) : z_of(it->t);
        for (const TrackGem& g : it->notes) {
            float x1, x2;
            if (g.kick) {
                x1 = T.x_left;
                x2 = T.x_right;
            } else {
                pad_x(cfg, g.pad, x1, x2);
            }
            // Hydra departs from Onyx here: Onyx shrinks every ghost, kicks
            // included, but a shrunken kick reads as a bar that stops short
            // of the highway edge. A ghost kick shows only the overlay.
            if (g.velocity == Velocity::Ghost && !g.kick) {
                const float cx = x1 + (x2 - x1) * 0.5f;
                x1 = cx + (x1 - cx) * kGhostWidthScale;
                x2 = cx + (x2 - cx) * kGhostWidthScale;
            }
            // A kick's box is as deep and tall as half its width (Onyx:
            // (x2' - x1') / 2); a pad gem's is fixed.
            const float ref = g.kick ? (x2 - x1) * 0.5f : kPadGemHalfSize;
            DrawCommand c;
            c.mesh = g.kick ? MeshId::Kick : g.cymbal ? MeshId::Cymbal : MeshId::Tom;
            c.lo[0] = x1; c.lo[1] = T.y - ref; c.lo[2] = z - ref;
            c.hi[0] = x2; c.hi[1] = T.y + ref; c.hi[2] = z + ref;
            if (fade) {
                c.material = color_mat(cfg.gems.color_hit);
                c.alpha = *fade;
            } else {
                TextureId base = gem_tex(g, od);
                c.material = g.velocity == Velocity::Ghost    ? overlay_mat(base, TextureId::OverlayGhost)
                             : g.velocity == Velocity::Accent ? overlay_mat(base, TextureId::OverlayAccent)
                                                              : tex_mat(base);
                c.alpha = 1.0f;
            }
            c.light = LightKind::GemOffset;
            c.depth = DepthMode::Less;
            out.push_back(c);
        }
    }
    return out;
}

}  // namespace hydra::render
