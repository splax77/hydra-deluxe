// Tests for render/highway_draw: the numbers and order of Onyx's drawDrumPlay,
// checked without a GPU.

#include "doctest.h"

#include <cmath>
#include <cstddef>
#include <vector>

#include "app/preview_view.h"
#include "render/highway_draw.h"
#include "render/obj_loader.h"
#include "render/overlay_layout.h"
#include "render/track_state.h"
#include "preview_config_util.h"
#include "preview_scene_fixtures.h"

using namespace DirectX;
using namespace hydra;
using namespace hydra::app;
using namespace hydra::render;
using namespace hydra::test_fixtures;

namespace {

XMFLOAT3 transform(const XMFLOAT4X4& m, float x, float y, float z) {
    XMVECTOR v = XMVector3TransformCoord(XMVectorSet(x, y, z, 1.0f), XMLoadFloat4x4(&m));
    XMFLOAT3 out;
    XMStoreFloat3(&out, v);
    return out;
}

bool is_lane_tex(TextureId t) { return t >= TextureId::LaneRed && t <= TextureId::LaneGreen; }

std::vector<const DrawCommand*> of_mesh(const std::vector<DrawCommand>& cmds, MeshId id) {
    std::vector<const DrawCommand*> out;
    for (const DrawCommand& c : cmds)
        if (c.mesh == id) out.push_back(&c);
    return out;
}

}  // namespace

TEST_CASE("time_to_z: Onyx's linear time->depth map") {
    PreviewConfig cfg = shipped_preview_config();
    CHECK(time_to_z(cfg, 10.0, 10.0, 1.0) == doctest::Approx(0.0));
    CHECK(time_to_z(cfg, 10.0, 11.35, 1.0) == doctest::Approx(-12.0));
    CHECK(time_to_z(cfg, 10.0, 10.0 - 0.225, 1.0) == doctest::Approx(2.0));
    CHECK(z_to_time(cfg, 10.0, 2.0, 1.0) == doctest::Approx(9.775));
    CHECK(z_to_time(cfg, 10.0, -12.0, 1.0) == doctest::Approx(11.35));
    // Speed stretches the far time.
    CHECK(time_to_z(cfg, 10.0, 12.7, 2.0) == doctest::Approx(-12.0));
}

TEST_CASE("far_time: now plus speed times secs_future") {
    const PreviewConfig cfg = shipped_preview_config();  // secs_future 1.35
    CHECK(far_time(cfg, 2.0, 1.0) == doctest::Approx(3.35));
    CHECK(far_time(cfg, 2.0, 2.0) == doctest::Approx(4.7));
    CHECK(z_to_time(cfg, 2.0, cfg.track.z_future, 1.0) == doctest::Approx(far_time(cfg, 2.0, 1.0)));
}

TEST_CASE("pad_x: the note area split into four lanes, left to right") {
    PreviewConfig cfg = shipped_preview_config();
    float x1, x2;
    pad_x(cfg, Pad::Red, x1, x2);
    CHECK(x1 == doctest::Approx(-1.0f));
    CHECK(x2 == doctest::Approx(-0.5f));
    pad_x(cfg, Pad::Yellow, x1, x2);
    CHECK(x1 == doctest::Approx(-0.5f));
    CHECK(x2 == doctest::Approx(0.0f));
    pad_x(cfg, Pad::Blue, x1, x2);
    CHECK(x1 == doctest::Approx(0.0f));
    CHECK(x2 == doctest::Approx(0.5f));
    pad_x(cfg, Pad::Green, x1, x2);
    CHECK(x1 == doctest::Approx(0.5f));
    CHECK(x2 == doctest::Approx(1.0f));
}

TEST_CASE("make_camera: Onyx's tilted view and right-handed projection") {
    PreviewConfig cfg = shipped_preview_config();
    HighwayCamera cam = make_camera(cfg, 1.0f);
    CHECK(cam.view_pos.y == doctest::Approx(1.4f));
    // World (0,-1,0) -> view (0, -0.907, -3.733): below centre, in front.
    XMFLOAT3 p = transform(cam.view, 0.0f, -1.0f, 0.0f);
    CHECK(p.x == doctest::Approx(0.0f).epsilon(0.001));
    CHECK(p.y == doctest::Approx(-0.907f).epsilon(0.01));
    CHECK(p.z == doctest::Approx(-3.733f).epsilon(0.01));
    // The far end of the highway sits above the near end on screen.
    XMFLOAT3 far = transform(cam.view, 0.0f, -1.0f, -12.0f);
    CHECK(far.y > p.y);
    CHECK(far.z < p.z);  // farther in front of the camera (RH: -Z)
    // Both project inside the clip volume with positive w.
    XMMATRIX vp = XMLoadFloat4x4(&cam.view) * XMLoadFloat4x4(&cam.proj);
    XMVECTOR clip = XMVector4Transform(XMVectorSet(0.0f, -1.0f, -12.0f, 1.0f), vp);
    CHECK(XMVectorGetW(clip) > 0.0f);
    CHECK(std::fabs(XMVectorGetY(clip) / XMVectorGetW(clip)) < 1.0f);
    XMVECTOR clip_near = XMVector4Transform(XMVectorSet(0.0f, -1.0f, 0.0f, 1.0f), vp);
    CHECK(XMVectorGetY(clip_near) / XMVectorGetW(clip_near) <
          XMVectorGetY(clip) / XMVectorGetW(clip));
}

TEST_CASE("stretch_matrix and light_for") {
    DrawCommand c;
    c.lo[0] = -1; c.lo[1] = -1.25f; c.lo[2] = -0.25f;
    c.hi[0] = -0.5f; c.hi[1] = -0.75f; c.hi[2] = 0.25f;
    XMMATRIX m = stretch_matrix(c);
    // The unit cube's (0.5, 0.5, 0.5) corner lands on the box's hi corner.
    XMVECTOR v = XMVector3TransformCoord(XMVectorSet(0.5f, 0.5f, 0.5f, 1.0f), m);
    CHECK(XMVectorGetX(v) == doctest::Approx(-0.5f));
    CHECK(XMVectorGetY(v) == doctest::Approx(-0.75f));
    CHECK(XMVectorGetZ(v) == doctest::Approx(0.25f));

    PreviewConfig cfg = shipped_preview_config();
    c.light = LightKind::GemOffset;
    LightConfig l = light_for(cfg, c);
    CHECK(l.position.x == doctest::Approx(-0.75f));
    CHECK(l.position.y == doctest::Approx(-0.75f + 1.0f));
    CHECK(l.position.z == doctest::Approx(0.0f + 0.2f));
    c.light = LightKind::Global;
    CHECK(light_for(cfg, c).position.y == doctest::Approx(-0.5f));
    // Unlit: full ambient, nothing else, so the shader returns the colour.
    c.light = LightKind::Unlit;
    const LightConfig u = light_for(cfg, c);
    CHECK(u.ambient.r == 1.0f);
    CHECK(u.ambient.g == 1.0f);
    CHECK(u.ambient.b == 1.0f);
    CHECK(u.ambient.a == 1.0f);
    for (const Color& off : {u.diffuse, u.specular}) {
        CHECK(off.r == 0.0f);
        CHECK(off.g == 0.0f);
        CHECK(off.b == 0.0f);
        CHECK(off.a == 0.0f);
    }

    // A Flat keeps Y scale 1.
    DrawCommand f;
    f.lo[0] = -1; f.lo[1] = -1; f.lo[2] = 0.13f;
    f.hi[0] = 1;  f.hi[1] = -1; f.hi[2] = -0.13f;
    XMVECTOR top = XMVector3TransformCoord(XMVectorSet(0.0f, 0.5f, 0.0f, 1.0f), stretch_matrix(f));
    CHECK(XMVectorGetY(top) == doctest::Approx(-0.5f));
}

TEST_CASE("build_highway_draws: order and geometry for a frame") {
    PreviewConfig cfg = shipped_preview_config();
    PreviewScene scene = timed_scene();
    scene.notes = {note(1000.0, PreviewLane::Red), note(1500.0, PreviewLane::Yellow, true),
                   note(1500.0, PreviewLane::Kick)};
    scene.solos = {span(900.0, 1200.0)};
    scene.fills = {fill(span(1300.0, 1500.0), PreviewFillState::Offered)};
    scene.beats = {{0, 1000.0, PreviewBeatKind::MeasureLine}, {0, 1250.0, PreviewBeatKind::Half}};
    TrackState st = build_track_state(scene, TrackStateOptions{});

    const double now = 1.0;  // the Red note is exactly at the strike line
    std::vector<DrawCommand> cmds = build_highway_draws(st, cfg, now, 1.0);
    REQUIRE(!cmds.empty());

    // Order: floor flats, two railing boxes, beat lines, lane strips, four
    // targets, gems. Find the first index of each kind.
    size_t first_box = cmds.size(), first_always = cmds.size(), first_gem = cmds.size();
    for (size_t i = 0; i < cmds.size(); ++i) {
        if (cmds[i].mesh == MeshId::Box && first_box == cmds.size()) first_box = i;
        if (cmds[i].depth == DepthMode::Always && first_always == cmds.size()) first_always = i;
        if ((cmds[i].mesh == MeshId::Tom || cmds[i].mesh == MeshId::Cymbal ||
             cmds[i].mesh == MeshId::Kick) && first_gem == cmds.size())
            first_gem = i;
    }
    CHECK(first_box < first_always);
    CHECK(first_always < first_gem);
    CHECK(cmds[0].mesh == MeshId::Flat);
    CHECK(cmds[0].depth == DepthMode::Less);
    CHECK(cmds[0].material.kind == MaterialKind::Color);

    // Floor covers near..far: first flat starts at z_past (2), the last floor
    // flat ends at z_future (-12); the solo stretch is tinted #333399.
    CHECK(cmds[0].lo[2] == doctest::Approx(2.0f));
    bool saw_solo = false;
    float floor_far = 0.0f;
    for (size_t i = 0; i < first_box; ++i) {
        if (cmds[i].material.color.b > 0.5f) saw_solo = true;
        floor_far = cmds[i].hi[2];
    }
    CHECK(saw_solo);
    CHECK(floor_far == doctest::Approx(-12.0f));

    // Railings.
    CHECK(cmds[first_box].lo[0] == doctest::Approx(-1.09f));
    CHECK(cmds[first_box].hi[0] == doctest::Approx(-1.0f));
    CHECK(cmds[first_box].lo[1] == doctest::Approx(-0.85f));
    CHECK(cmds[first_box].hi[1] == doctest::Approx(-1.1f));
    CHECK(cmds[first_box + 1].hi[0] == doctest::Approx(1.09f));

    // Beat lines: a Bar at now (z 0 +-0.05, line-1) and a Half at +0.25 s.
    int bars = 0, halves = 0;
    for (const DrawCommand& c : cmds) {
        if (c.material.texture == TextureId::Line1) {
            ++bars;
            CHECK(c.lo[2] == doctest::Approx(0.05f));
            CHECK(c.hi[2] == doctest::Approx(-0.05f));
            CHECK(c.depth == DepthMode::Always);
        }
        if (c.material.texture == TextureId::Line3) ++halves;
    }
    // The Bar sits exactly at now, well inside the (near, far) window.
    CHECK(bars == 1);
    CHECK(halves == 1);

    // Fill: four lane strips from 1.3 s to kSpanEndTicks past the 1.5 s note.
    // timed_scene() has one tick per millisecond, hence the 1000.
    int lanes = 0;
    for (const DrawCommand& c : cmds) {
        if (is_lane_tex(c.material.texture)) {
            ++lanes;
            CHECK(c.lo[2] == doctest::Approx(time_to_z(cfg, now, 1.3, 1.0)));
            CHECK(c.hi[2] ==
                  doctest::Approx(time_to_z(cfg, now, 1.5 + kSpanEndTicks / 1000.0, 1.0)));
        }
    }
    CHECK(lanes == 4);

    // Four targets at z [+0.13, -0.13] with depth off.
    int targets = 0;
    for (const DrawCommand& c : cmds)
        if (c.material.texture >= TextureId::TargetRed && c.material.texture <= TextureId::TargetGreen) {
            ++targets;
            CHECK(c.lo[2] == doctest::Approx(0.13f));
            CHECK(c.hi[2] == doctest::Approx(-0.13f));
        }
    CHECK(targets == 4);

    // Gems: the Red exactly at now is struck, so it flashes at the strike
    // line (D48, Q27); the Yellow cymbal and the kick at 1.5 s are drawn
    // farther up.
    std::vector<const DrawCommand*> toms = of_mesh(cmds, MeshId::Tom);
    std::vector<const DrawCommand*> cymbals = of_mesh(cmds, MeshId::Cymbal);
    std::vector<const DrawCommand*> kicks = of_mesh(cmds, MeshId::Kick);
    REQUIRE(toms.size() == 1);
    CHECK(toms[0]->material.kind == MaterialKind::Color);
    CHECK((toms[0]->lo[2] + toms[0]->hi[2]) * 0.5f == doctest::Approx(0.0f));
    REQUIRE(cymbals.size() == 1);
    REQUIRE(kicks.size() == 1);
    const float z15 = static_cast<float>(time_to_z(cfg, now, 1.5, 1.0));
    CHECK(cymbals[0]->lo[0] == doctest::Approx(-0.5f));
    CHECK(cymbals[0]->hi[0] == doctest::Approx(0.0f));
    CHECK(cymbals[0]->lo[1] == doctest::Approx(-1.25f));
    CHECK(cymbals[0]->hi[1] == doctest::Approx(-0.75f));
    CHECK(cymbals[0]->lo[2] == doctest::Approx(z15 - 0.25f));
    CHECK(cymbals[0]->hi[2] == doctest::Approx(z15 + 0.25f));
    CHECK(cymbals[0]->material.texture == TextureId::CymbalYellow);
    CHECK(cymbals[0]->light == LightKind::GemOffset);
    CHECK(kicks[0]->lo[0] == doctest::Approx(-1.0f));
    CHECK(kicks[0]->hi[0] == doctest::Approx(1.0f));
    CHECK(kicks[0]->lo[1] == doctest::Approx(-2.0f));
    CHECK(kicks[0]->hi[1] == doctest::Approx(0.0f));
    CHECK(kicks[0]->lo[2] == doctest::Approx(z15 - 1.0f));
    CHECK(kicks[0]->material.texture == TextureId::LongKick);
}

TEST_CASE("build_highway_draws: a Red gem just ahead fills Onyx's box") {
    PreviewConfig cfg = shipped_preview_config();
    PreviewScene scene;
    scene.notes = {note(1001.0, PreviewLane::Red)};
    TrackState st = build_track_state(scene, TrackStateOptions{});
    std::vector<DrawCommand> cmds = build_highway_draws(st, cfg, 1.0, 1.0);
    std::vector<const DrawCommand*> toms = of_mesh(cmds, MeshId::Tom);
    REQUIRE(toms.size() == 1);
    const float z = static_cast<float>(time_to_z(cfg, 1.0, 1.001, 1.0));
    CHECK(toms[0]->lo[0] == doctest::Approx(-1.0f));
    CHECK(toms[0]->hi[0] == doctest::Approx(-0.5f));
    CHECK(toms[0]->lo[1] == doctest::Approx(-1.25f));
    CHECK(toms[0]->hi[1] == doctest::Approx(-0.75f));
    CHECK(toms[0]->lo[2] == doctest::Approx(z - 0.25f));
    CHECK(toms[0]->hi[2] == doctest::Approx(z + 0.25f));
    CHECK(toms[0]->material.kind == MaterialKind::Texture);
    CHECK(toms[0]->material.texture == TextureId::BoxRed);
}

TEST_CASE("build_highway_draws: ghost shrinks 70% and overlays; accent overlays") {
    PreviewConfig cfg = shipped_preview_config();
    PreviewScene scene;
    scene.notes = {note(1100.0, PreviewLane::Blue, false, true),
                   note(1200.0, PreviewLane::Green, true, false, true)};
    TrackState st = build_track_state(scene, TrackStateOptions{});
    std::vector<DrawCommand> cmds = build_highway_draws(st, cfg, 1.0, 1.0);
    std::vector<const DrawCommand*> toms = of_mesh(cmds, MeshId::Tom);
    std::vector<const DrawCommand*> cyms = of_mesh(cmds, MeshId::Cymbal);
    REQUIRE(toms.size() == 1);
    REQUIRE(cyms.size() == 1);
    // Blue lane is [0, 0.5]; centre 0.25; 70% -> [0.075, 0.425].
    CHECK(toms[0]->lo[0] == doctest::Approx(0.075f));
    CHECK(toms[0]->hi[0] == doctest::Approx(0.425f));
    CHECK(toms[0]->material.kind == MaterialKind::TextureOverlay);
    CHECK(toms[0]->material.texture == TextureId::BoxBlue);
    CHECK(toms[0]->material.overlay == TextureId::OverlayGhost);
    CHECK(cyms[0]->material.overlay == TextureId::OverlayAccent);
    CHECK(cyms[0]->material.texture == TextureId::CymbalGreen);
    CHECK(cyms[0]->lo[0] == doctest::Approx(0.5f));
}

TEST_CASE("build_highway_draws: a ghost kick keeps full width and overlays") {
    // Hydra departs from Onyx here: Onyx shrinks every ghost to 70%, but a
    // shrunken kick reads as a bar that stops short of the highway edge.
    PreviewConfig cfg = shipped_preview_config();
    PreviewScene scene;
    scene.notes = {note(1100.0, PreviewLane::Kick, false, true)};
    TrackState st = build_track_state(scene, TrackStateOptions{});
    std::vector<DrawCommand> cmds = build_highway_draws(st, cfg, 1.0, 1.0);
    std::vector<const DrawCommand*> kicks = of_mesh(cmds, MeshId::Kick);
    REQUIRE(kicks.size() == 1);
    CHECK(kicks[0]->lo[0] == doctest::Approx(-1.0f));
    CHECK(kicks[0]->hi[0] == doctest::Approx(1.0f));
    CHECK(kicks[0]->lo[1] == doctest::Approx(-2.0f));
    CHECK(kicks[0]->hi[1] == doctest::Approx(0.0f));
    CHECK(kicks[0]->material.kind == MaterialKind::TextureOverlay);
    CHECK(kicks[0]->material.texture == TextureId::LongKick);
    CHECK(kicks[0]->material.overlay == TextureId::OverlayGhost);
}

TEST_CASE("build_highway_draws: hit flash and target glow after a note passes") {
    PreviewConfig cfg = shipped_preview_config();
    PreviewScene scene;
    scene.notes = {note(1000.0, PreviewLane::Green), note(1000.0, PreviewLane::Kick),
                   note(5000.0, PreviewLane::Red)};
    TrackState st = build_track_state(scene, TrackStateOptions{});

    // 50 ms after the hit: the gems flash white at the strike line at alpha
    // 0.5, and only the Green target glows (kicks light nothing).
    std::vector<DrawCommand> cmds = build_highway_draws(st, cfg, 1.05, 1.0);
    std::vector<const DrawCommand*> toms = of_mesh(cmds, MeshId::Tom);
    std::vector<const DrawCommand*> kicks = of_mesh(cmds, MeshId::Kick);
    REQUIRE(toms.size() == 1);
    REQUIRE(kicks.size() == 1);
    CHECK(toms[0]->material.kind == MaterialKind::Color);
    CHECK(toms[0]->material.color.r == doctest::Approx(1.0f));
    CHECK(toms[0]->alpha == doctest::Approx(0.5f));
    CHECK((toms[0]->lo[2] + toms[0]->hi[2]) * 0.5f == doctest::Approx(0.0f));
    CHECK(kicks[0]->alpha == doctest::Approx(0.5f));
    int glows = 0;
    for (const DrawCommand& c : cmds)
        if (c.material.texture == TextureId::TargetGreenLight) {
            ++glows;
            // Pinned from one run on 2026-10-05; the fade length is the
            // shipped json's targets_secs_light.
            CHECK(c.alpha == doctest::Approx(0.7f));
        }
    CHECK(glows == 1);
    for (const DrawCommand& c : cmds) CHECK(c.material.texture != TextureId::TargetRedLight);

    // 150 ms after: the flash is gone, the glow is dimmer.
    cmds = build_highway_draws(st, cfg, 1.15, 1.0);
    CHECK(of_mesh(cmds, MeshId::Tom).empty());
    glows = 0;
    for (const DrawCommand& c : cmds)
        if (c.material.texture == TextureId::TargetGreenLight) {
            ++glows;
            CHECK(c.alpha == doctest::Approx(0.1f));  // same run, same owner
        }
    CHECK(glows == 1);

    // 200 ms after: no glow either.
    cmds = build_highway_draws(st, cfg, 1.2, 1.0);
    for (const DrawCommand& c : cmds) CHECK(c.material.texture != TextureId::TargetGreenLight);
}

TEST_CASE("build_highway_draws: a chord exactly on the playhead is lit") {
    // The same chord as above, with the playhead exactly on it: a jump to an
    // activation lands here. It counts as struck (struck_at), so the gem
    // flashes at full strength and the Green target glows at full strength
    // (D48, Q27).
    PreviewConfig cfg = shipped_preview_config();
    PreviewScene scene;
    scene.notes = {note(1000.0, PreviewLane::Green), note(1000.0, PreviewLane::Kick),
                   note(5000.0, PreviewLane::Red)};
    TrackState st = build_track_state(scene, TrackStateOptions{});

    std::vector<DrawCommand> cmds = build_highway_draws(st, cfg, 1.0, 1.0);
    std::vector<const DrawCommand*> toms = of_mesh(cmds, MeshId::Tom);
    REQUIRE(toms.size() == 1);
    CHECK(toms[0]->material.kind == MaterialKind::Color);
    CHECK(toms[0]->material.color.r == doctest::Approx(1.0f));
    CHECK(toms[0]->alpha == doctest::Approx(1.0f));
    int glows = 0;
    for (const DrawCommand& c : cmds)
        if (c.material.texture == TextureId::TargetGreenLight) {
            ++glows;
            CHECK(c.alpha == doctest::Approx(1.0f));
        }
    CHECK(glows == 1);
}

TEST_CASE("build_highway_draws: energy gems inside an SP phrase, tinted floor in an active window") {
    PreviewConfig cfg = shipped_preview_config();
    PreviewScene scene = timed_scene();
    scene.notes = {note(1100.0, PreviewLane::Red), note(1200.0, PreviewLane::Yellow, true),
                   note(1200.0, PreviewLane::Kick), note(1300.0, PreviewLane::Blue)};
    scene.sp_phrases = {span(1100.0, 1200.0)};  // ends on the yellow/kick chord
    PreviewActivation a;
    a.tick = 1300;
    a.ms = 1300.0;
    a.has_sp_end = true;
    a.sp_end_ms = 2000.0;
    scene.activations = {a};
    TrackState st = build_track_state(scene, TrackStateOptions{});
    std::vector<DrawCommand> cmds = build_highway_draws(st, cfg, 1.0, 1.0);

    int energy = 0, plain = 0;
    for (const DrawCommand& c : cmds) {
        if (c.material.texture == TextureId::BoxEnergy || c.material.texture == TextureId::CymbalEnergy ||
            c.material.texture == TextureId::LongEnergy)
            ++energy;
        if (c.material.texture == TextureId::BoxBlue) ++plain;
    }
    CHECK(energy == 3);  // red tom, yellow cymbal, kick — the end chord included
    CHECK(plain == 1);   // the blue after the phrase

    // The floor between 1.3 s and the far edge is the darkened energy colour.
    bool tinted = false;
    for (const DrawCommand& c : cmds) {
        if (c.mesh != MeshId::Flat || c.material.kind != MaterialKind::Color) continue;
        if (c.lo[2] <= time_to_z(cfg, 1.0, 1.3, 1.0) + 1e-4 && c.material.color.g > c.material.color.r)
            tinted = true;
    }
    CHECK(tinted);
}

TEST_CASE("build_highway_draws: the taken fill lights its lane with the lit target") {
    PreviewConfig cfg = shipped_preview_config();
    PreviewScene scene = timed_scene();
    scene.notes = {note(1000.0, PreviewLane::Red), note(1500.0, PreviewLane::Green)};
    scene.fills = {fill(span(1300.0, 1500.0), PreviewFillState::Taken)};
    PreviewActivation a;
    a.tick = 1500;
    a.ms = 1500.0;
    a.has_lane = true;
    a.lane = PreviewLane::Green;
    a.taken_fill = 0;
    scene.activations = {a};
    TrackState st = build_track_state(scene, TrackStateOptions{});
    std::vector<DrawCommand> cmds = build_highway_draws(st, cfg, 1.0, 1.0);
    int lanes = 0, lit_green = 0;
    for (const DrawCommand& c : cmds) {
        if (is_lane_tex(c.material.texture)) {
            ++lanes;
            CHECK(c.alpha == doctest::Approx(1.0f));  // taken fills are full strength
        }
        if (c.material.texture == TextureId::TargetGreenLight) {
            ++lit_green;
            // The extra pass covers the fill's stretch, not the strike line.
            CHECK(c.lo[2] == doctest::Approx(time_to_z(cfg, 1.0, 1.3, 1.0)));
            // kSpanEndTicks past the 1.5 s note, at one tick per millisecond.
            CHECK(c.hi[2] ==
                  doctest::Approx(time_to_z(cfg, 1.0, 1.5 + kSpanEndTicks / 1000.0, 1.0)));
            CHECK(c.lo[0] == doctest::Approx(0.5f));  // the green lane's x span
            CHECK(c.hi[0] == doctest::Approx(1.0f));
        }
    }
    CHECK(lanes == 4);      // all four lanes light, Onyx's BRE look
    CHECK(lit_green == 1);  // the activation lane, drawn once more on top
}

// D53 item 3: when two taken fills touch, each lights its own lane colour. The
// scene is test_track_state.cpp's "make_lane_bounds: two touching taken fills
// each light their own lane": fills on ticks 1000-2000 and 2000-3000, the
// first activated on Green at its end tick 2000, the second on Yellow at 3000.
// Hand check at now 2.0 s: the window runs from 1.775 s (z_past) to 3.35 s
// (z_future), so the Green lane is lit from 1.775 s to 2.0005 s (the first
// fill's end, kSpanEndTicks past its last note) and the Yellow lane from
// 2.0005 s to 3.0005 s. No note sits near the strike line, so no target glows
// and every
// lit-target draw here is a lane strip.
TEST_CASE("build_highway_draws: two touching taken fills light two lanes") {
    const PreviewConfig cfg = shipped_preview_config();
    PreviewScene scene = timed_scene();
    scene.notes = {note(0.0, PreviewLane::Red), note(4000.0, PreviewLane::Red)};
    scene.fills = {fill(span(1000.0, 2000.0), PreviewFillState::Taken),
                   fill(span(2000.0, 3000.0), PreviewFillState::Taken)};
    PreviewActivation green;
    green.tick = 2000;
    green.ms = 2000.0;
    green.has_lane = true;
    green.lane = PreviewLane::Green;
    green.taken_fill = 0;
    PreviewActivation yellow = green;
    yellow.tick = 3000;
    yellow.ms = 3000.0;
    yellow.lane = PreviewLane::Yellow;
    yellow.taken_fill = 1;
    scene.activations = {green, yellow};
    TrackState st = build_track_state(scene, TrackStateOptions{});

    const double now = 2.0;
    const double near_t = z_to_time(cfg, now, cfg.track.z_past, 1.0);
    const double far_t = z_to_time(cfg, now, cfg.track.z_future, 1.0);
    std::vector<LaneSpan> stretches = st.make_lane_bounds(st.window(near_t, far_t), near_t, far_t);
    REQUIRE(stretches.size() == 2);
    CHECK(stretches[0].pad == Pad::Green);
    CHECK(stretches[1].pad == Pad::Yellow);

    std::vector<DrawCommand> cmds = build_highway_draws(st, cfg, now, 1.0);
    std::vector<const DrawCommand*> lit;
    for (const DrawCommand& c : cmds)
        if (c.material.texture >= TextureId::TargetRedLight &&
            c.material.texture <= TextureId::TargetGreenLight)
            lit.push_back(&c);
    REQUIRE(lit.size() == 2);
    CHECK(lit[0]->material.texture == TextureId::TargetGreenLight);
    CHECK(lit[1]->material.texture == TextureId::TargetYellowLight);
    for (size_t i = 0; i < 2; ++i) {
        CHECK(lit[i]->lo[2] == doctest::Approx(time_to_z(cfg, now, stretches[i].t1, 1.0)));
        CHECK(lit[i]->hi[2] == doctest::Approx(time_to_z(cfg, now, stretches[i].t2, 1.0)));
    }
}

TEST_CASE("build_highway_draws: an offered fill's strips are dimmed") {
    PreviewConfig cfg = shipped_preview_config();
    PreviewScene scene = timed_scene();
    scene.notes = {note(1000.0, PreviewLane::Red), note(1500.0, PreviewLane::Green)};
    scene.fills = {fill(span(1300.0, 1500.0), PreviewFillState::Offered)};
    TrackState st = build_track_state(scene, TrackStateOptions{});
    std::vector<DrawCommand> cmds = build_highway_draws(st, cfg, 1.0, 1.0);
    int lanes = 0, lit = 0;
    for (const DrawCommand& c : cmds) {
        if (is_lane_tex(c.material.texture)) {
            ++lanes;
            CHECK(c.alpha == doctest::Approx(cfg.hydra.fill_offered_alpha));
        }
        if (c.material.texture == TextureId::TargetGreenLight) ++lit;
    }
    CHECK(lanes == 4);
    CHECK(lit == 0);  // nothing was activated here
}

TEST_CASE("build_highway_draws: a hidden fill draws no lane strips") {
    PreviewConfig cfg = shipped_preview_config();
    PreviewScene scene;
    scene.notes = {note(1000.0, PreviewLane::Red), note(1500.0, PreviewLane::Green)};
    scene.fills = {fill(span(1300.0, 1500.0), PreviewFillState::Hidden)};
    TrackState st = build_track_state(scene, TrackStateOptions{});
    std::vector<DrawCommand> cmds = build_highway_draws(st, cfg, 1.0, 1.0);
    int lanes = 0;
    for (const DrawCommand& c : cmds)
        if (is_lane_tex(c.material.texture)) ++lanes;
    CHECK(lanes == 0);
}

namespace {

bool same_draw(const DrawCommand& a, const DrawCommand& b) {
    for (int k = 0; k < 3; ++k)
        if (a.lo[k] != b.lo[k] || a.hi[k] != b.hi[k]) return false;
    return a.mesh == b.mesh && a.material.kind == b.material.kind &&
           a.material.color.r == b.material.color.r && a.material.color.g == b.material.color.g &&
           a.material.color.b == b.material.color.b && a.material.color.a == b.material.color.a &&
           a.material.texture == b.material.texture && a.material.overlay == b.material.overlay &&
           a.alpha == b.alpha && a.light == b.light && a.depth == b.depth;
}

}  // namespace

// A single Yellow drum lane over [1.2 s, 1.4 s] and a double Red + Blue lane
// over [1.5 s, 1.7 s], with the playhead at 1.0 s. Each pad's strip is its own
// lane texture at full strength, depth off, over exactly its lane's span (the
// end kSpanEndTicks past the end tick, one tick per millisecond).
TEST_CASE("build_highway_draws: drum lanes light their own pads in the lane texture") {
    const PreviewConfig cfg = shipped_preview_config();
    PreviewScene scene = timed_scene();
    scene.notes = {note(1000.0, PreviewLane::Red), note(1800.0, PreviewLane::Green)};
    scene.drum_lanes = {drum_lane(span(1200.0, 1400.0), {PreviewLane::Yellow}),
                        drum_lane(span(1500.0, 1700.0), {PreviewLane::Red, PreviewLane::Blue})};
    TrackState st = build_track_state(scene, TrackStateOptions{});
    const double now = 1.0;
    std::vector<DrawCommand> cmds = build_highway_draws(st, cfg, now, 1.0);

    std::vector<const DrawCommand*> strips;
    for (const DrawCommand& c : cmds)
        if (is_lane_tex(c.material.texture)) strips.push_back(&c);
    // Pads drawn left to right: Red, Yellow, Blue.
    REQUIRE(strips.size() == 3);
    struct Want {
        TextureId tex;
        Pad pad;
        double t1, t2;
    };
    const double end = kSpanEndTicks / 1000.0;  // one tick per millisecond
    const Want want[] = {{TextureId::LaneRed, Pad::Red, 1.5, 1.7 + end},
                         {TextureId::LaneYellow, Pad::Yellow, 1.2, 1.4 + end},
                         {TextureId::LaneBlue, Pad::Blue, 1.5, 1.7 + end}};
    for (size_t i = 0; i < 3; ++i) {
        const DrawCommand& c = *strips[i];
        INFO("strip " << i);
        CHECK(c.material.texture == want[i].tex);
        CHECK(c.alpha == doctest::Approx(1.0f));
        CHECK(c.depth == DepthMode::Always);
        float x1, x2;
        pad_x(cfg, want[i].pad, x1, x2);
        CHECK(c.lo[0] == doctest::Approx(x1));
        CHECK(c.hi[0] == doctest::Approx(x2));
        CHECK(c.lo[2] == doctest::Approx(time_to_z(cfg, now, want[i].t1, 1.0)));
        CHECK(c.hi[2] == doctest::Approx(time_to_z(cfg, now, want[i].t2, 1.0)));
    }
}

// Onyx's drawLanes draws the roll lanes before the BRE strips: a drum lane
// inside an offered fill comes first in the draw list.
TEST_CASE("build_highway_draws: drum lane strips come before the fill strips") {
    const PreviewConfig cfg = shipped_preview_config();
    PreviewScene scene = timed_scene();
    scene.notes = {note(1000.0, PreviewLane::Red), note(1500.0, PreviewLane::Green)};
    scene.fills = {fill(span(1300.0, 1500.0), PreviewFillState::Offered)};
    scene.drum_lanes = {drum_lane(span(1300.0, 1500.0), {PreviewLane::Green})};
    TrackState st = build_track_state(scene, TrackStateOptions{});
    std::vector<DrawCommand> cmds = build_highway_draws(st, cfg, 1.0, 1.0);
    std::vector<const DrawCommand*> strips;
    for (const DrawCommand& c : cmds)
        if (is_lane_tex(c.material.texture)) strips.push_back(&c);
    REQUIRE(strips.size() == 5);  // the drum lane's one, then the fill's four
    CHECK(strips[0]->material.texture == TextureId::LaneGreen);
    CHECK(strips[0]->alpha == doctest::Approx(1.0f));
    for (size_t i = 1; i < 5; ++i)
        CHECK(strips[i]->alpha == doctest::Approx(cfg.hydra.fill_offered_alpha));
}

// A lane on the kick column lights nothing: the frame is the one the scene
// draws with no lanes at all.
TEST_CASE("build_highway_draws: a kick drum lane draws what no lane draws") {
    const PreviewConfig cfg = shipped_preview_config();
    PreviewScene plain = timed_scene();
    plain.notes = {note(1000.0, PreviewLane::Red), note(1500.0, PreviewLane::Kick)};
    plain.fills = {fill(span(1300.0, 1500.0), PreviewFillState::Offered)};
    PreviewScene kick = plain;
    kick.drum_lanes = {drum_lane(span(1100.0, 1600.0), {PreviewLane::Kick})};
    const std::vector<DrawCommand> want =
        build_highway_draws(build_track_state(plain, TrackStateOptions{}), cfg, 1.0, 1.0);
    const std::vector<DrawCommand> got =
        build_highway_draws(build_track_state(kick, TrackStateOptions{}), cfg, 1.0, 1.0);
    REQUIRE(got.size() == want.size());
    for (size_t i = 0; i < want.size(); ++i) {
        INFO("draw " << i);
        CHECK(same_draw(got[i], want[i]));
    }
}

namespace {

// The draws `got` adds to `want`, where `got` is `want` with one run of extra
// draws inserted somewhere: everything outside that run must match.
std::vector<DrawCommand> inserted_run(const std::vector<DrawCommand>& got,
                                      const std::vector<DrawCommand>& want) {
    REQUIRE(got.size() >= want.size());
    size_t at = 0;
    while (at < want.size() && same_draw(got[at], want[at])) ++at;
    const size_t n = got.size() - want.size();
    for (size_t i = at; i < want.size(); ++i) {
        INFO("draw " << i << " after the run");
        CHECK(same_draw(got[i + n], want[i]));
    }
    return {got.begin() + static_cast<std::ptrdiff_t>(at),
            got.begin() + static_cast<std::ptrdiff_t>(at + n)};
}

// One outline line: a flat on the floor in `pad`'s lane texture at the offered
// strip's alpha, depth off, over [x1, x2] and [z1, z2].
void check_outline_line(const DrawCommand& c, const PreviewConfig& cfg, Pad pad, float x1,
                        float x2, double z1, double z2) {
    CHECK(c.mesh == MeshId::Flat);
    CHECK(c.material.kind == MaterialKind::Texture);
    CHECK(c.material.texture == (pad == Pad::Red      ? TextureId::LaneRed
                                 : pad == Pad::Yellow ? TextureId::LaneYellow
                                 : pad == Pad::Blue   ? TextureId::LaneBlue
                                                      : TextureId::LaneGreen));
    CHECK(c.alpha == doctest::Approx(cfg.hydra.fill_offered_alpha));
    CHECK(c.depth == DepthMode::Always);
    CHECK(c.lo[1] == doctest::Approx(cfg.track.y));
    CHECK(c.lo[0] == doctest::Approx(x1));
    CHECK(c.hi[0] == doctest::Approx(x2));
    CHECK(c.lo[2] == doctest::Approx(z1));
    CHECK(c.hi[2] == doctest::Approx(z2));
}

// The outline's line width in the shipped config: a beat line's depth.
constexpr double kShippedOutlineWidth = 0.1;

}  // namespace

// A ghost path's fill over [1.3 s, 1.5 s] (hidden for the drawn path) and an
// offered fill over [1.8 s, 2.0 s] that is no ghost's, the playhead at 1.0 s.
// The ghost adds exactly its outline: the near and far edge across each lane,
// then the two sides between them, and leaves every other draw as it was.
TEST_CASE("build_highway_draws: a ghost fill draws the offered strip's outline, nothing else does") {
    const PreviewConfig cfg = shipped_preview_config();
    PreviewScene plain = timed_scene();
    plain.notes = {note(1000.0, PreviewLane::Red), note(1500.0, PreviewLane::Green),
                   note(2000.0, PreviewLane::Green)};
    plain.fills = {fill(span(1300.0, 1500.0), PreviewFillState::Hidden),
                   fill(span(1800.0, 2000.0), PreviewFillState::Offered)};
    PreviewScene ghosted = plain;
    ghosted.ghost_fills = {0};
    const double now = 1.0;
    const std::vector<DrawCommand> want =
        build_highway_draws(build_track_state(plain, TrackStateOptions{}), cfg, now, 1.0);
    const std::vector<DrawCommand> got =
        build_highway_draws(build_track_state(ghosted, TrackStateOptions{}), cfg, now, 1.0);

    const std::vector<DrawCommand> outline = inserted_run(got, want);
    REQUIRE(outline.size() == 10);
    const double w = kShippedOutlineWidth;
    const double end = kSpanEndTicks / 1000.0;  // one tick per millisecond
    const double z_near = time_to_z(cfg, now, 1.3, 1.0);
    const double z_far = time_to_z(cfg, now, 1.5 + end, 1.0);
    for (size_t k = 0; k < kPads.size(); ++k) {
        const Pad p = kPads[k];
        float x1, x2;
        pad_x(cfg, p, x1, x2);
        INFO("pad " << k);
        check_outline_line(outline[2 * k], cfg, p, x1, x2, z_near, z_near - w);
        check_outline_line(outline[2 * k + 1], cfg, p, x1, x2, z_far + w, z_far);
    }
    const float xl = cfg.track.x_left, xr = cfg.track.x_right;
    check_outline_line(outline[8], cfg, Pad::Red, xl, xl + static_cast<float>(w), z_near - w,
                       z_far + w);
    check_outline_line(outline[9], cfg, Pad::Green, xr - static_cast<float>(w), xr, z_near - w,
                       z_far + w);
}

// A ghost fill the window cuts has no edge at the cut: past the far end it
// has its near edge and sides only, and once its start is behind the near end
// its far edge and sides only.
TEST_CASE("build_highway_draws: a ghost fill cut by the window has no edge at the cut") {
    const PreviewConfig cfg = shipped_preview_config();
    PreviewScene plain = timed_scene();
    plain.notes = {note(1000.0, PreviewLane::Red), note(3000.0, PreviewLane::Green)};
    plain.fills = {fill(span(2000.0, 3000.0), PreviewFillState::Hidden)};
    PreviewScene ghosted = plain;
    ghosted.ghost_fills = {0};
    const double w = kShippedOutlineWidth;
    const double end = kSpanEndTicks / 1000.0;

    {
        const double now = 1.0;  // the far end is at 2.35 s
        const std::vector<DrawCommand> outline = inserted_run(
            build_highway_draws(build_track_state(ghosted, TrackStateOptions{}), cfg, now, 1.0),
            build_highway_draws(build_track_state(plain, TrackStateOptions{}), cfg, now, 1.0));
        REQUIRE(outline.size() == 6);
        const double z_near = time_to_z(cfg, now, 2.0, 1.0);
        for (size_t k = 0; k < kPads.size(); ++k) {
            float x1, x2;
            pad_x(cfg, kPads[k], x1, x2);
            check_outline_line(outline[k], cfg, kPads[k], x1, x2, z_near, z_near - w);
        }
        const float xl = cfg.track.x_left;
        check_outline_line(outline[4], cfg, Pad::Red, xl, xl + static_cast<float>(w), z_near - w,
                           cfg.track.z_future);
    }
    {
        const double now = 2.8;  // the near end is at 2.575 s
        const std::vector<DrawCommand> outline = inserted_run(
            build_highway_draws(build_track_state(ghosted, TrackStateOptions{}), cfg, now, 1.0),
            build_highway_draws(build_track_state(plain, TrackStateOptions{}), cfg, now, 1.0));
        REQUIRE(outline.size() == 6);
        const double z_far = time_to_z(cfg, now, 3.0 + end, 1.0);
        for (size_t k = 0; k < kPads.size(); ++k) {
            float x1, x2;
            pad_x(cfg, kPads[k], x1, x2);
            check_outline_line(outline[k], cfg, kPads[k], x1, x2, z_far + w, z_far);
        }
        const float xr = cfg.track.x_right;
        check_outline_line(outline[5], cfg, Pad::Green, xr - static_cast<float>(w), xr,
                           cfg.track.z_past, z_far + w);
    }
}

TEST_CASE("build_highway_draws: an empty window still draws floor, railings and targets") {
    PreviewConfig cfg = shipped_preview_config();
    PreviewScene scene;
    scene.notes = {note(10000.0, PreviewLane::Red)};
    TrackState st = build_track_state(scene, TrackStateOptions{});
    std::vector<DrawCommand> cmds = build_highway_draws(st, cfg, 1.0, 1.0);
    // One floor flat (no solo/SP cuts), two railings, four targets.
    CHECK(cmds.size() == 1 + 2 + 4);
    CHECK(cmds[0].lo[2] == doctest::Approx(2.0f));
    CHECK(cmds[0].hi[2] == doctest::Approx(-12.0f));
}

TEST_CASE("texture_file names every texture") {
    for (int i = 1; i < static_cast<int>(TextureId::Count); ++i)
        CHECK(texture_file(static_cast<TextureId>(i))[0] != '\0');
    CHECK(std::string(texture_file(TextureId::LongKick)) == "long-kick.jpg");
}

TEST_CASE("build_highway_draws: a chord one tick after a phrase ends draws plain") {
    PreviewConfig cfg = shipped_preview_config();
    PreviewScene scene =
        phrase_to_tick_480_scene({{480, PreviewLane::Yellow}, {481, PreviewLane::Blue}});
    TrackState st = build_track_state(scene, TrackStateOptions{});
    std::vector<DrawCommand> cmds = build_highway_draws(st, cfg, 0.0, 1.0);

    int energy = 0, plain_blue = 0;
    for (const DrawCommand& c : cmds) {
        if (c.material.texture == TextureId::BoxEnergy) ++energy;
        if (c.material.texture == TextureId::BoxBlue) ++plain_blue;
    }
    CHECK(energy == 1);      // the yellow on the phrase's last tick
    CHECK(plain_blue == 1);  // the blue one tick later
}

// D81: where an active SP window ends, a bright edge across the floor and a
// triangle beside each railing, in the mark's own colour (sp_end_color).
namespace {

bool is_sp_end_mark(const DrawCommand& c, const PreviewConfig& cfg) {
    const Color& mark = cfg.hydra.sp_end_color;
    return c.material.kind == MaterialKind::Color && c.material.color.r == mark.r &&
           c.material.color.g == mark.g && c.material.color.b == mark.b;
}

bool is_gem(const DrawCommand& c) {
    return c.mesh == MeshId::Tom || c.mesh == MeshId::Cymbal || c.mesh == MeshId::Kick;
}

PreviewActivation activation_until(double ms, double sp_end_ms) {
    PreviewActivation a;
    a.tick = static_cast<int64_t>(ms);
    a.ms = ms;
    a.has_sp_end = true;
    a.sp_end_tick = static_cast<int64_t>(sp_end_ms);
    a.sp_end_ms = sp_end_ms;
    return a;
}

// The Ministry of Lost Souls shape: SP runs out 26 ms after a kick + blue
// cymbal, and the next chord is 57 ms later.
PreviewScene sp_end_scene() {
    PreviewScene scene = timed_scene();
    scene.notes = {note(1100.0, PreviewLane::Red),  note(1300.0, PreviewLane::Yellow, true),
                   note(1974.0, PreviewLane::Kick), note(1974.0, PreviewLane::Blue, true),
                   note(2031.0, PreviewLane::Kick), note(2031.0, PreviewLane::Green, true)};
    scene.activations = {activation_until(1300.0, 2000.0)};
    return scene;
}

}  // namespace

TEST_CASE("sp_active_ends: each window's end inside the window, back-to-back ones apart") {
    PreviewScene scene = timed_scene();
    scene.notes = {note(1000.0, PreviewLane::Red)};
    scene.activations = {activation_until(1300.0, 2000.0), activation_until(2000.0, 2600.0)};
    TrackState st = build_track_state(scene, TrackStateOptions{});
    CHECK(st.sp_active_ends(st.window(1.0, 3.0)) == std::vector<double>{2.0, 2.6});
    CHECK(st.sp_active_ends(st.window(2.1, 3.0)) == std::vector<double>{2.6});
    // An end on either edge is outside the window, so it is not listed.
    CHECK(st.sp_active_ends(st.window(2.0, 2.6)).empty());
    CHECK(st.sp_active_ends(st.window(0.0, 1.9)).empty());
    // Overlapping windows run SP on to the later end; only that one counts.
    scene.activations = {activation_until(1300.0, 2000.0), activation_until(1800.0, 2600.0)};
    TrackState overlap = build_track_state(scene, TrackStateOptions{});
    CHECK(overlap.sp_active_ends(overlap.window(1.0, 3.0)) == std::vector<double>{2.6});
}

TEST_CASE("build_highway_draws: an SP end gets a floor edge and two rail triangles, depth off") {
    const PreviewConfig cfg = shipped_preview_config();
    TrackState st = build_track_state(sp_end_scene(), TrackStateOptions{});
    const double now = 1.0;  // far edge at 2.35 s, so the 2.0 s end is in view
    std::vector<DrawCommand> cmds = build_highway_draws(st, cfg, now, 1.0);
    const float z_end = static_cast<float>(time_to_z(cfg, now, 2.0, 1.0));

    std::vector<size_t> marks;
    size_t last_floor = 0, first_gem = cmds.size();
    for (size_t i = 0; i < cmds.size(); ++i) {
        if (is_sp_end_mark(cmds[i], cfg)) marks.push_back(i);
        if (cmds[i].mesh == MeshId::Flat && cmds[i].depth == DepthMode::Less) last_floor = i;
        if (is_gem(cmds[i]) && first_gem == cmds.size()) first_gem = i;
    }
    REQUIRE(marks.size() == 3);
    REQUIRE(first_gem < cmds.size());
    int edges = 0, lefts = 0, rights = 0;
    for (size_t i : marks) {
        const DrawCommand& c = cmds[i];
        CHECK(i > last_floor);
        CHECK(i < first_gem);
        CHECK(c.depth == DepthMode::Always);
        CHECK((c.lo[2] + c.hi[2]) * 0.5f == doctest::Approx(z_end));
        // Unlit, so the edge and the triangles show the same colour.
        CHECK(c.light == LightKind::Unlit);
        if (c.mesh == MeshId::Flat) {
            ++edges;
            CHECK(c.lo[0] == doctest::Approx(-1.0f));  // the whole floor width
            CHECK(c.hi[0] == doctest::Approx(1.0f));
            CHECK(c.lo[1] == doctest::Approx(-1.0f));  // on the floor
            CHECK(c.hi[1] == doctest::Approx(-1.0f));
            CHECK(c.lo[2] - c.hi[2] == doctest::Approx(0.1f));  // sp_end_edge_depth
        } else {
            // Its base rests on the railing's outer edge (x -1.09 or 1.09)
            // and it reaches 0.07 further out, 0.07 tall and centred on the
            // floor's height.
            CHECK(std::fmin(c.lo[1], c.hi[1]) == doctest::Approx(-1.035f));
            CHECK(std::fmax(c.lo[1], c.hi[1]) == doctest::Approx(-0.965f));
            if (c.mesh == MeshId::TriangleLeft) {
                ++lefts;
                CHECK(c.lo[0] == doctest::Approx(-1.16f));
                CHECK(c.hi[0] == doctest::Approx(-1.09f));
            } else {
                REQUIRE(c.mesh == MeshId::TriangleRight);
                ++rights;
                CHECK(c.lo[0] == doctest::Approx(1.09f));
                CHECK(c.hi[0] == doctest::Approx(1.16f));
            }
        }
    }
    CHECK(edges == 1);
    CHECK(lefts == 1);
    CHECK(rights == 1);
}

// The triangles stand facing the camera with their front side, so back-face
// culling keeps them, and on screen each apex points away from the lanes.
TEST_CASE("build_highway_draws: the SP end triangles face the camera and point outward") {
    const PreviewConfig cfg = shipped_preview_config();
    TrackState st = build_track_state(sp_end_scene(), TrackStateOptions{});
    const double now = 1.0;
    const int w = 1200, h = 400;
    int seen = 0;
    for (const DrawCommand& c : build_highway_draws(st, cfg, now, 1.0)) {
        if (c.mesh != MeshId::TriangleLeft && c.mesh != MeshId::TriangleRight) continue;
        ++seen;
        const bool right = c.mesh == MeshId::TriangleRight;
        const ObjMesh mesh = make_triangle(right);
        REQUIRE(mesh.triangle_count() == 1);
        float sx[3], sy[3];
        for (int k = 0; k < 3; ++k) {
            const ObjVertex& v = mesh.vertices[k];
            const XMVECTOR wp = XMVector3TransformCoord(
                XMVectorSet(v.pos[0], v.pos[1], v.pos[2], 1.0f), stretch_matrix(c));
            const ImagePoint p = project_to_image(
                cfg, w, h, Vec3{XMVectorGetX(wp), XMVectorGetY(wp), XMVectorGetZ(wp)});
            sx[k] = p.x;
            sy[k] = p.y;
        }
        // Image pixels run y down, so a negative signed area here is
        // counter-clockwise as the viewer sees it: the front face the
        // renderer keeps.
        const float area2 = (sx[1] - sx[0]) * (sy[2] - sy[0]) - (sx[2] - sx[0]) * (sy[1] - sy[0]);
        CHECK(area2 < 0.0f);
        // The apex is the vertex at the side away from the base.
        size_t apex = 0;
        for (size_t k = 0; k < 3; ++k)
            if (mesh.vertices[k].pos[1] == 0.0f) apex = k;
        for (size_t k = 0; k < 3; ++k) {
            if (k == apex) continue;
            if (right)
                CHECK(sx[apex] > sx[k]);
            else
                CHECK(sx[apex] < sx[k]);
        }
    }
    CHECK(seen == 2);
}

TEST_CASE("build_highway_draws: no SP end mark past the far edge or behind the near edge") {
    const PreviewConfig cfg = shipped_preview_config();
    TrackState st = build_track_state(sp_end_scene(), TrackStateOptions{});
    // now 0.5 s: the far edge is 1.85 s, short of the 2.0 s end, so the window
    // is only clipped there. now 2.3 s: the near edge is 2.075 s, past it.
    for (double now : {0.5, 2.3}) {
        CAPTURE(now);
        int marks = 0;
        for (const DrawCommand& c : build_highway_draws(st, cfg, now, 1.0))
            if (is_sp_end_mark(c, cfg)) ++marks;
        CHECK(marks == 0);
    }
}

TEST_CASE("build_highway_draws: two back-to-back SP windows give two edges") {
    const PreviewConfig cfg = shipped_preview_config();
    PreviewScene scene = timed_scene();
    scene.notes = {note(1000.0, PreviewLane::Red), note(2000.0, PreviewLane::Yellow)};
    scene.activations = {activation_until(1300.0, 2000.0), activation_until(2000.0, 2600.0)};
    TrackState st = build_track_state(scene, TrackStateOptions{});
    const double now = 1.5;  // the window runs from 1.275 s to 2.85 s
    std::vector<float> edge_z;
    int triangles = 0;
    for (const DrawCommand& c : build_highway_draws(st, cfg, now, 1.0)) {
        if (!is_sp_end_mark(c, cfg)) continue;
        if (c.mesh == MeshId::Flat)
            edge_z.push_back((c.lo[2] + c.hi[2]) * 0.5f);
        else
            ++triangles;
    }
    REQUIRE(edge_z.size() == 2);
    CHECK(edge_z[0] == doctest::Approx(time_to_z(cfg, now, 2.0, 1.0)));
    CHECK(edge_z[1] == doctest::Approx(time_to_z(cfg, now, 2.6, 1.0)));
    CHECK(triangles == 4);
}
