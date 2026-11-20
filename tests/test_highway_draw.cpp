// Tests for render/highway_draw: the numbers and order of Onyx's drawDrumPlay,
// checked without a GPU.

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"  // Settings: the app's default analysis settings
#include "app/preview_view.h"
#include "render/highway_draw.h"
#include "render/track_state.h"
#include "corpus_util.h"
#include "preview_config_util.h"
#include "preview_scene_fixtures.h"
#include "record_fixtures.h"  // test::path_named

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

// Pitch Dark (testdata/input/test_pitch_dark_ratio) under the app's default
// settings, drawn three ways. Its record's path "3+ 2+ 1" skips fills, so the
// path scene has offered fills beside its taken ones, and its activations
// carry SP windows. The chart has one solo and no drum lane.
struct PitchDarkScenes {
    PreviewScene bare;     // no path: every fill Offered, no activation
    PreviewScene path;     // with the record's path "3+ 2+ 1"
    PreviewScene ghosted;  // the same path, with the record's best path as the ghost
};

const PitchDarkScenes& pitch_dark_scenes() {
    static const PitchDarkScenes scenes = [] {
        const AnalysisSettings cfg = Settings().to_analysis_settings();
        const std::string chart = corpus::chart_named("test_pitch_dark_ratio");
        if (chart.empty()) throw std::runtime_error("Pitch Dark is not in the corpus");
        const Song& song = corpus::song(chart, cfg.reading(), cfg.rules);
        const HydraRecord& record = corpus::analyzed(chart, cfg);
        const Path* path = test::path_named(record.all_paths(), "3+ 2+ 1");
        if (path == nullptr) throw std::runtime_error("Pitch Dark has no path 3+ 2+ 1");
        PitchDarkScenes s;
        s.bare = build_preview_scene(song, nullptr, cfg.sp_cap, cfg.rules);
        s.path = build_preview_scene(song, path, cfg.sp_cap, cfg.rules);
        s.ghosted = build_preview_scene(song, path, cfg.sp_cap, cfg.rules, std::nullopt,
                                        &record.best_path());
        return s;
    }();
    return scenes;
}

// Jack Wall - Snakeskin Boots (testdata/input, Summer Blast _25 Tier 3), the
// song alone: its Expert drums carry one drum lane (an "S 65" over a run of
// Red notes at tick 19328) and no fill near it.
const PreviewScene& snakeskin_base() {
    static const PreviewScene scene = [] {
        const AnalysisSettings cfg = Settings().to_analysis_settings();
        const std::string chart = corpus::chart_named("Snakeskin Boots");
        if (chart.empty()) throw std::runtime_error("Snakeskin Boots is not in the corpus");
        return build_preview_base(corpus::song(chart, cfg.reading(), cfg.rules));
    }();
    return scene;
}

// The visible window at a playhead and speed, the way build_highway_draws
// reads it: from the time at z_past to far_time.
struct HighwayView {
    double now = 0.0, speed = 1.0, near_t = 0.0, far_t = 0.0;
};

HighwayView view_at(const PreviewConfig& cfg, double now, double speed = 1.0) {
    return {now, speed, z_to_time(cfg, now, cfg.track.z_past, speed), far_time(cfg, now, speed)};
}

// The track state's stretches of one span field over the view.
std::vector<ToggleSpan> bounds(const TrackState& st, const HighwayView& v, Toggle TrackInstant::*field) {
    return st.make_toggle_bounds(st.window(v.near_t, v.far_t), v.near_t, v.far_t, field);
}

bool is_lit_target_tex(TextureId t) {
    return t >= TextureId::TargetRedLight && t <= TextureId::TargetGreenLight;
}

// The lane strips (lane or lit-target texture, depth off) that sit exactly on
// `pad`'s lane over the depths of [t1, t2] in view `v`.
std::vector<const DrawCommand*> strips_over(const std::vector<DrawCommand>& cmds,
                                            const PreviewConfig& cfg, const HighwayView& v, Pad pad,
                                            double t1, double t2) {
    float x1, x2;
    pad_x(cfg, pad, x1, x2);
    const double z1 = time_to_z(cfg, v.now, t1, v.speed);
    const double z2 = time_to_z(cfg, v.now, t2, v.speed);
    std::vector<const DrawCommand*> out;
    for (const DrawCommand& c : cmds) {
        if (c.mesh != MeshId::Flat || c.depth != hydra::render::DepthMode::Always) continue;
        if (!is_lane_tex(c.material.texture) && !is_lit_target_tex(c.material.texture)) continue;
        if (c.lo[0] == doctest::Approx(x1) && c.hi[0] == doctest::Approx(x2) &&
            c.lo[2] == doctest::Approx(z1) && c.hi[2] == doctest::Approx(z2))
            out.push_back(&c);
    }
    return out;
}

bool same_color(const Color& a, const Color& b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

// The floor's coloured flats, in draw order (near to far).
std::vector<DrawCommand> floor_flats(const std::vector<DrawCommand>& cmds) {
    std::vector<DrawCommand> out;
    for (const DrawCommand& c : cmds)
        if (c.mesh == MeshId::Flat && c.depth == hydra::render::DepthMode::Less &&
            c.material.kind == MaterialKind::Color)
            out.push_back(c);
    return out;
}

bool is_energy_tex(TextureId t) {
    return t == TextureId::BoxEnergy || t == TextureId::CymbalEnergy ||
           t == TextureId::LongEnergy;
}

// The gems drawn for the chord at `ms`, with the playhead 50 ms before it (a
// free input: ahead of the strike line, so nothing has flashed): every gem
// mesh whose depth centre is the chord's. Each is true when drawn in an
// energy texture.
std::vector<bool> chord_energy(const TrackState& st, const PreviewConfig& cfg, double ms) {
    const double now = ms / 1000.0 - 0.05;
    const float z = static_cast<float>(time_to_z(cfg, now, ms / 1000.0, 1.0));
    std::vector<bool> out;
    for (const DrawCommand& c : build_highway_draws(st, cfg, now, 1.0)) {
        if (c.mesh != MeshId::Tom && c.mesh != MeshId::Cymbal && c.mesh != MeshId::Kick) continue;
        if ((c.lo[2] + c.hi[2]) * 0.5f == doctest::Approx(z)) out.push_back(is_energy_tex(c.material.texture));
    }
    return out;
}

// Gem sizes that are literals in Onyx's code, not keys in its 3d-config.yml
// (mtolly/onyx, haskell/packages/onyx-lib-game/src/Onyx/Game/Graphics.hs,
// drawDrumPlay / drawGem, lines 660-668 at commit 84d5e51). The config holds
// every other number the tests below use.
constexpr float kOnyxPadGemHalfSize = 0.5f / 2.0f;  // Onyx: reference = 0.5 / 2
constexpr float kOnyxGhostWidthScale = 0.7f;        // Onyx: (v - xCenter) * 0.7

// Hail The Sun - Missed Injections (testdata/input, IB24 Tier 4), the song
// alone. Its notes.mid turns dynamics on at tick 0 (ENABLE_CHART_DYNAMICS),
// and its Expert drums carry a ghost kick (velocity 1 on pitch 96, tick
// 269760), ghost Red toms (velocity 1 on 97) and accented Yellow cymbals
// (velocity 127 on 98 with no tom marker, from tick 124320).
const PreviewScene& missed_injections_base() {
    static const PreviewScene scene = [] {
        const AnalysisSettings cfg = Settings().to_analysis_settings();
        const std::string chart = corpus::chart_named("Missed Injections");
        if (chart.empty()) throw std::runtime_error("Missed Injections is not in the corpus");
        return build_preview_base(corpus::song(chart, cfg.reading(), cfg.rules));
    }();
    return scene;
}

// Is the note at `tick` inside one of the scene's SP phrases (its end tick
// included)?
bool in_sp_phrase(const PreviewScene& scene, int64_t tick) {
    for (const PreviewSpan& ph : scene.sp_phrases)
        if (tick >= ph.start_tick && tick <= ph.end_tick) return true;
    return false;
}

// The first note of the scene on `lane` that passes `want` and sits outside
// every SP phrase, so its gem draws in its own pad's texture.
template <typename Want>
const PreviewNote* first_plain_note(const PreviewScene& scene, PreviewLane lane, Want&& want) {
    for (const PreviewNote& n : scene.notes)
        if (n.lane == lane && want(n) && !in_sp_phrase(scene, n.tick)) return &n;
    return nullptr;
}

// The gem draws of mesh `mesh` for the chord at `ms`, with the playhead 50 ms
// before it (a free input: ahead of the strike line, so nothing has flashed),
// whose depth centre is the chord's and whose x centre is `x_centre`.
std::vector<DrawCommand> gems_at(const TrackState& st, const PreviewConfig& cfg, double ms,
                                 MeshId mesh, float x_centre) {
    const double now = ms / 1000.0 - 0.05;
    const float z = static_cast<float>(time_to_z(cfg, now, ms / 1000.0, 1.0));
    std::vector<DrawCommand> out;
    for (const DrawCommand& c : build_highway_draws(st, cfg, now, 1.0))
        if (c.mesh == mesh && (c.lo[2] + c.hi[2]) * 0.5f == doctest::Approx(z) &&
            (c.lo[0] + c.hi[0]) * 0.5f == doctest::Approx(x_centre))
            out.push_back(c);
    return out;
}

// The centre of `pad`'s lane.
float pad_centre(const PreviewConfig& cfg, Pad pad) {
    float x1, x2;
    pad_x(cfg, pad, x1, x2);
    return (x1 + x2) * 0.5f;
}

// The frame "order and geometry" tests below share: a Red tom on the
// playhead, a Yellow cymbal and a kick at 1.5 s, a solo, an offered fill
// from 1.3 s to the 1.5 s chord, a measure line on the playhead and a half
// beat 0.25 s on, drawn at now 1.0 s and speed 1.
constexpr double kFrameNow = 1.0;

TrackState frame_state() {
    PreviewScene scene = timed_scene();
    scene.notes = {note(1000.0, PreviewLane::Red), note(1500.0, PreviewLane::Yellow, true),
                   note(1500.0, PreviewLane::Kick)};
    scene.solos = {span(900.0, 1200.0)};
    scene.fills = {fill(span(1300.0, 1500.0), PreviewFillState::Offered)};
    scene.beats = {{0, 1000.0, PreviewBeatKind::MeasureLine}, {0, 1250.0, PreviewBeatKind::Half}};
    return build_track_state(scene, TrackStateOptions{});
}

std::vector<DrawCommand> frame_draws(const PreviewConfig& cfg) {
    return build_highway_draws(frame_state(), cfg, kFrameNow, 1.0);
}

bool is_target_tex(TextureId t) { return t >= TextureId::TargetRed && t <= TextureId::TargetGreen; }

}  // namespace

// The map is stated on the config's own fields: the shipped values themselves
// belong to test_preview_config.cpp. The playhead sits at z_now, the far time
// (secs_future ahead, stretched by speed) at z_future, and depth is a straight
// line in time, so z_to_time undoes time_to_z everywhere.
TEST_CASE("time_to_z: Onyx's linear time->depth map") {
    const PreviewConfig cfg = shipped_preview_config();
    const auto& T = cfg.track;
    // Free inputs: a playhead and two speeds.
    const double now = 10.0;
    for (const double speed : {1.0, 2.0}) {
        CAPTURE(speed);
        // far_time is now plus speed times secs_future.
        const double far_t = far_time(cfg, now, speed);
        CHECK(far_t == doctest::Approx(now + speed * T.secs_future));
        CHECK(time_to_z(cfg, now, now, speed) == doctest::Approx(T.z_now));
        CHECK(time_to_z(cfg, now, far_t, speed) == doctest::Approx(T.z_future));
        CHECK(z_to_time(cfg, now, T.z_future, speed) == doctest::Approx(far_t));
        CHECK(z_to_time(cfg, now, T.z_now, speed) == doctest::Approx(now));
        // The near edge is behind the playhead, and the map runs through it.
        const double near_t = z_to_time(cfg, now, T.z_past, speed);
        CHECK(near_t < now);
        CHECK(time_to_z(cfg, now, near_t, speed) == doctest::Approx(T.z_past));
        // A straight line: equal steps in time are equal steps in depth.
        const double step = time_to_z(cfg, now, now + 0.1, speed) - time_to_z(cfg, now, now, speed);
        for (const double t : {near_t, now - 0.05, now + 0.5, far_t}) {
            CAPTURE(t);
            CHECK(z_to_time(cfg, now, time_to_z(cfg, now, t, speed), speed) == doctest::Approx(t));
            CHECK(time_to_z(cfg, now, t + 0.1, speed) - time_to_z(cfg, now, t, speed) ==
                  doctest::Approx(step));
        }
    }
}

// The lanes tile the note area: the first starts at x_left, each ends where
// the next starts, the last ends at x_right, all four are one width, and they
// run Red, Yellow, Blue, Green from left to right.
TEST_CASE("pad_x: the note area split into four lanes, left to right") {
    const PreviewConfig& cfg = shipped_preview_config();
    REQUIRE(cfg.track.x_right > cfg.track.x_left);
    float first_x1, first_x2;
    pad_x(cfg, Pad::Red, first_x1, first_x2);
    const float width = first_x2 - first_x1;
    CHECK(width > 0.0f);
    float edge = cfg.track.x_left;
    for (const Pad p : {Pad::Red, Pad::Yellow, Pad::Blue, Pad::Green}) {
        CAPTURE(static_cast<int>(p));
        float x1, x2;
        pad_x(cfg, p, x1, x2);
        CHECK(x1 == doctest::Approx(edge));
        CHECK(x2 - x1 == doctest::Approx(width));
        edge = x2;
    }
    CHECK(edge == doctest::Approx(cfg.track.x_right));
}

TEST_CASE("make_camera: Onyx's tilted view and right-handed projection") {
    const PreviewConfig& cfg = shipped_preview_config();
    const auto& T = cfg.track;
    HighwayCamera cam = make_camera(cfg, 1.0f);
    CHECK(cam.view_pos.y == doctest::Approx(cfg.view.camera_position.y));
    // The highway's centre line on the floor, at the strike line and at the
    // far end, seen from the camera.
    const XMFLOAT3 p = transform(cam.view, 0.0f, T.y, T.z_now);
    const XMFLOAT3 far = transform(cam.view, 0.0f, T.y, T.z_future);
    // The far end of the highway sits above the near end on screen.
    CHECK(far.y > p.y);
    CHECK(far.z < p.z);  // farther in front of the camera (RH: -Z)
    // Both project inside the clip volume with positive w.
    XMMATRIX vp = XMLoadFloat4x4(&cam.view) * XMLoadFloat4x4(&cam.proj);
    XMVECTOR clip = XMVector4Transform(XMVectorSet(0.0f, T.y, T.z_future, 1.0f), vp);
    CHECK(XMVectorGetW(clip) > 0.0f);
    CHECK(std::fabs(XMVectorGetY(clip) / XMVectorGetW(clip)) < 1.0f);
    XMVECTOR clip_near = XMVector4Transform(XMVectorSet(0.0f, T.y, T.z_now, 1.0f), vp);
    CHECK(XMVectorGetW(clip_near) > 0.0f);
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
    CHECK(XMVectorGetX(v) == doctest::Approx(c.hi[0]));
    CHECK(XMVectorGetY(v) == doctest::Approx(c.hi[1]));
    CHECK(XMVectorGetZ(v) == doctest::Approx(c.hi[2]));

    // A gem's light sits at the config's offset from the box's top centre.
    const PreviewConfig& cfg = shipped_preview_config();
    const float centre_x = (c.lo[0] + c.hi[0]) * 0.5f;
    const float top_y = std::max(c.lo[1], c.hi[1]);
    const float centre_z = (c.lo[2] + c.hi[2]) * 0.5f;
    c.light = LightKind::GemOffset;
    LightConfig l = light_for(cfg, c);
    CHECK(l.position.x == doctest::Approx(centre_x + cfg.gems.light.position.x));
    CHECK(l.position.y == doctest::Approx(top_y + cfg.gems.light.position.y));
    CHECK(l.position.z == doctest::Approx(centre_z + cfg.gems.light.position.z));
    // The highway light is the config's, wherever the box is.
    c.light = LightKind::Global;
    const LightConfig g = light_for(cfg, c);
    CHECK(g.position.x == doctest::Approx(cfg.track.light.position.x));
    CHECK(g.position.y == doctest::Approx(cfg.track.light.position.y));
    CHECK(g.position.z == doctest::Approx(cfg.track.light.position.z));
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

// One frame draws, in order, floor flats, two railing boxes, beat lines, lane
// strips, four targets, then gems.
TEST_CASE("build_highway_draws: a frame draws floor, railings, beat lines, strips, targets, gems in order") {
    const PreviewConfig& cfg = shipped_preview_config();
    const std::vector<DrawCommand> cmds = frame_draws(cfg);
    REQUIRE(!cmds.empty());
    // The first index of each kind.
    size_t first_box = cmds.size(), first_always = cmds.size(), first_gem = cmds.size();
    for (size_t i = 0; i < cmds.size(); ++i) {
        if (cmds[i].mesh == MeshId::Box && first_box == cmds.size()) first_box = i;
        if (cmds[i].depth == hydra::render::DepthMode::Always && first_always == cmds.size()) first_always = i;
        if ((cmds[i].mesh == MeshId::Tom || cmds[i].mesh == MeshId::Cymbal ||
             cmds[i].mesh == MeshId::Kick) && first_gem == cmds.size())
            first_gem = i;
    }
    CHECK(first_box < first_always);
    CHECK(first_always < first_gem);
    CHECK(cmds[0].mesh == MeshId::Flat);
    CHECK(cmds[0].depth == hydra::render::DepthMode::Less);
    CHECK(cmds[0].material.kind == MaterialKind::Color);
    // Every floor flat comes before the first railing box.
    for (size_t i = 0; i < first_box; ++i) {
        CAPTURE(i);
        CHECK(cmds[i].mesh == MeshId::Flat);
        CHECK(cmds[i].depth == hydra::render::DepthMode::Less);
    }
}

// The floor covers the view from z_past to z_future, and the solo's stretch
// is drawn in the solo colour.
TEST_CASE("build_highway_draws: the floor spans the view and tints the solo") {
    const PreviewConfig& cfg = shipped_preview_config();
    const std::vector<DrawCommand> floor = floor_flats(frame_draws(cfg));
    REQUIRE_FALSE(floor.empty());
    CHECK(floor.front().lo[2] == doctest::Approx(cfg.track.z_past));
    CHECK(floor.back().hi[2] == doctest::Approx(cfg.track.z_future));
    bool saw_solo = false;
    for (const DrawCommand& f : floor)
        if (same_color(f.material.color, cfg.track.color_solo)) saw_solo = true;
    CHECK(saw_solo);
}

// The two railings sit where railing_x puts them, at the config's height,
// left first.
TEST_CASE("build_highway_draws: the railings sit at the highway's edges") {
    const PreviewConfig& cfg = shipped_preview_config();
    const std::vector<DrawCommand> cmds = frame_draws(cfg);
    const std::vector<const DrawCommand*> boxes = of_mesh(cmds, MeshId::Box);
    REQUIRE(boxes.size() == 2);
    for (const bool right : {false, true}) {
        CAPTURE(right);
        const DrawCommand& rail = *boxes[right ? 1 : 0];
        float x1, x2;
        railing_x(cfg, right, x1, x2);
        CHECK(rail.lo[0] == doctest::Approx(x1));
        CHECK(rail.hi[0] == doctest::Approx(x2));
        CHECK(rail.lo[1] == doctest::Approx(cfg.track.railing_y_top));
        CHECK(rail.hi[1] == doctest::Approx(cfg.track.railing_y_bottom));
    }
}

// The measure line on the playhead is drawn once in the measure texture over
// the config's beat-line depth, depth test off; the half beat once in its own.
TEST_CASE("build_highway_draws: each beat line is drawn once at its own depth") {
    const PreviewConfig& cfg = shipped_preview_config();
    const float z_bar = static_cast<float>(time_to_z(cfg, kFrameNow, 1.0, 1.0));
    int bars = 0, halves = 0;
    for (const DrawCommand& c : frame_draws(cfg)) {
        if (c.material.texture == TextureId::Line1) {
            ++bars;
            CHECK(c.lo[2] == doctest::Approx(z_bar + cfg.track.beats_z_past));
            CHECK(c.hi[2] == doctest::Approx(z_bar + cfg.track.beats_z_future));
            CHECK(c.depth == hydra::render::DepthMode::Always);
        }
        if (c.material.texture == TextureId::Line3) ++halves;
    }
    CHECK(bars == 1);
    CHECK(halves == 1);
}

// The offered fill draws one lane strip on each pad, from its start to
// kSpanEndTicks past its 1.5 s end note (timed_scene() has one tick per
// millisecond, hence the 1000), and no other lane strip.
TEST_CASE("build_highway_draws: an offered fill draws one strip on each lane over its span") {
    const PreviewConfig& cfg = shipped_preview_config();
    const std::vector<DrawCommand> cmds = frame_draws(cfg);
    const HighwayView v = view_at(cfg, kFrameNow);
    for (const Pad p : kPads) {
        CAPTURE(static_cast<int>(p));
        CHECK(strips_over(cmds, cfg, v, p, 1.3, 1.5 + kSpanEndTicks / 1000.0).size() == 1);
    }
    int lanes = 0;
    for (const DrawCommand& c : cmds)
        if (is_lane_tex(c.material.texture)) ++lanes;
    CHECK(lanes == static_cast<int>(kPads.size()));
}

// One target per pad on its own lane, over the strike line's depths, depth
// test off.
TEST_CASE("build_highway_draws: each pad has one target on the strike line") {
    const PreviewConfig& cfg = shipped_preview_config();
    const auto& T = cfg.track;
    std::vector<const DrawCommand*> targets;
    for (const DrawCommand& c : frame_draws(cfg))
        if (is_target_tex(c.material.texture)) targets.push_back(&c);
    REQUIRE(targets.size() == kPads.size());
    for (size_t i = 0; i < kPads.size(); ++i) {
        CAPTURE(i);
        float x1, x2;
        pad_x(cfg, kPads[i], x1, x2);
        CHECK(targets[i]->lo[0] == doctest::Approx(x1));
        CHECK(targets[i]->hi[0] == doctest::Approx(x2));
        CHECK(targets[i]->lo[2] == doctest::Approx(T.z_now + T.targets_z_past));
        CHECK(targets[i]->hi[2] == doctest::Approx(T.z_now + T.targets_z_future));
        CHECK(targets[i]->depth == hydra::render::DepthMode::Always);
    }
}

// The Red tom exactly on the playhead is struck, so it flashes at the strike
// line; the Yellow cymbal and the kick at 1.5 s are drawn at their own depth
// in their own textures.
TEST_CASE("build_highway_draws: a struck gem flashes at the strike line, later ones draw at their time") {
    const PreviewConfig& cfg = shipped_preview_config();
    const std::vector<DrawCommand> cmds = frame_draws(cfg);
    const std::vector<const DrawCommand*> toms = of_mesh(cmds, MeshId::Tom);
    const std::vector<const DrawCommand*> cymbals = of_mesh(cmds, MeshId::Cymbal);
    const std::vector<const DrawCommand*> kicks = of_mesh(cmds, MeshId::Kick);
    REQUIRE(toms.size() == 1);
    CHECK(toms[0]->material.kind == MaterialKind::Color);
    CHECK((toms[0]->lo[2] + toms[0]->hi[2]) * 0.5f == doctest::Approx(cfg.track.z_now));
    REQUIRE(cymbals.size() == 1);
    REQUIRE(kicks.size() == 1);
    const float z15 = static_cast<float>(time_to_z(cfg, kFrameNow, 1.5, 1.0));
    CHECK((cymbals[0]->lo[2] + cymbals[0]->hi[2]) * 0.5f == doctest::Approx(z15));
    CHECK((cymbals[0]->lo[0] + cymbals[0]->hi[0]) * 0.5f ==
          doctest::Approx(pad_centre(cfg, Pad::Yellow)));
    CHECK(cymbals[0]->material.texture == TextureId::CymbalYellow);
    CHECK(cymbals[0]->light == LightKind::GemOffset);
    CHECK((kicks[0]->lo[2] + kicks[0]->hi[2]) * 0.5f == doctest::Approx(z15));
    CHECK(kicks[0]->material.texture == TextureId::LongKick);
}

// A pad gem is Onyx's box: its lane's width, kOnyxPadGemHalfSize above and
// below the floor and either side of its own depth, in its pad's texture.
TEST_CASE("build_highway_draws: a Red gem just ahead fills Onyx's box") {
    const PreviewConfig& cfg = shipped_preview_config();
    PreviewScene scene;
    scene.notes = {note(1001.0, PreviewLane::Red)};
    TrackState st = build_track_state(scene, TrackStateOptions{});
    std::vector<DrawCommand> cmds = build_highway_draws(st, cfg, 1.0, 1.0);
    std::vector<const DrawCommand*> toms = of_mesh(cmds, MeshId::Tom);
    REQUIRE(toms.size() == 1);
    float x1, x2;
    pad_x(cfg, Pad::Red, x1, x2);
    const float z = static_cast<float>(time_to_z(cfg, 1.0, 1.001, 1.0));
    CHECK(toms[0]->lo[0] == doctest::Approx(x1));
    CHECK(toms[0]->hi[0] == doctest::Approx(x2));
    CHECK(toms[0]->lo[1] == doctest::Approx(cfg.track.y - kOnyxPadGemHalfSize));
    CHECK(toms[0]->hi[1] == doctest::Approx(cfg.track.y + kOnyxPadGemHalfSize));
    CHECK(toms[0]->lo[2] == doctest::Approx(z - kOnyxPadGemHalfSize));
    CHECK(toms[0]->hi[2] == doctest::Approx(z + kOnyxPadGemHalfSize));
    CHECK(toms[0]->material.kind == MaterialKind::Texture);
    CHECK(toms[0]->material.texture == TextureId::BoxRed);
}

// Missed Injections' first ghost Red tom and first accented Yellow cymbal
// outside SP phrases. A ghost tom is drawn kOnyxGhostWidthScale of its lane's
// width about the lane's centre, with the ghost overlay on its pad's box; an
// accent cymbal keeps its whole lane and takes the accent overlay.
TEST_CASE("build_highway_draws: ghost shrinks 70% and overlays; accent overlays") {
    const PreviewConfig& cfg = shipped_preview_config();
    const PreviewScene& scene = missed_injections_base();
    const TrackState st = build_track_state(scene, TrackStateOptions{});

    const PreviewNote* ghost = first_plain_note(scene, PreviewLane::Red,
                                                [](const PreviewNote& n) { return n.ghost; });
    REQUIRE(ghost != nullptr);
    CAPTURE(ghost->tick);
    float x1, x2;
    pad_x(cfg, Pad::Red, x1, x2);
    const float centre = (x1 + x2) * 0.5f;
    const float half = (x2 - x1) * 0.5f * kOnyxGhostWidthScale;
    const std::vector<DrawCommand> toms = gems_at(st, cfg, ghost->ms, MeshId::Tom, centre);
    REQUIRE(toms.size() == 1);
    CHECK(toms[0].lo[0] == doctest::Approx(centre - half));
    CHECK(toms[0].hi[0] == doctest::Approx(centre + half));
    CHECK(toms[0].material.kind == MaterialKind::TextureOverlay);
    CHECK(toms[0].material.texture == TextureId::BoxRed);
    CHECK(toms[0].material.overlay == TextureId::OverlayGhost);

    const PreviewNote* accent = first_plain_note(
        scene, PreviewLane::Yellow, [](const PreviewNote& n) { return n.accent && n.cymbal; });
    REQUIRE(accent != nullptr);
    CAPTURE(accent->tick);
    pad_x(cfg, Pad::Yellow, x1, x2);
    const std::vector<DrawCommand> cyms =
        gems_at(st, cfg, accent->ms, MeshId::Cymbal, pad_centre(cfg, Pad::Yellow));
    REQUIRE(cyms.size() == 1);
    CHECK(cyms[0].lo[0] == doctest::Approx(x1));
    CHECK(cyms[0].hi[0] == doctest::Approx(x2));
    CHECK(cyms[0].material.kind == MaterialKind::TextureOverlay);
    CHECK(cyms[0].material.texture == TextureId::CymbalYellow);
    CHECK(cyms[0].material.overlay == TextureId::OverlayAccent);
}

// Missed Injections' ghost kick (tick 269760). Hydra departs from Onyx here:
// Onyx shrinks every ghost to 70%, but a shrunken kick reads as a bar that
// stops short of the highway edge, so a ghost kick keeps the note area's
// width (x_left to x_right) and shows only the ghost overlay. Its box is as
// tall as it is wide, as every kick's is (Onyx: reference = (x2' - x1') / 2).
TEST_CASE("build_highway_draws: a ghost kick keeps full width and overlays") {
    const PreviewConfig& cfg = shipped_preview_config();
    const auto& T = cfg.track;
    const PreviewScene& scene = missed_injections_base();
    const TrackState st = build_track_state(scene, TrackStateOptions{});
    const PreviewNote* ghost = nullptr;
    for (const PreviewNote& n : scene.notes)
        if (n.lane == PreviewLane::Kick && n.ghost) {
            ghost = &n;
            break;
        }
    REQUIRE(ghost != nullptr);
    CAPTURE(ghost->tick);
    const std::vector<DrawCommand> kicks =
        gems_at(st, cfg, ghost->ms, MeshId::Kick, (T.x_left + T.x_right) * 0.5f);
    REQUIRE(kicks.size() == 1);
    const float half = (T.x_right - T.x_left) * 0.5f;
    CHECK(kicks[0].lo[0] == doctest::Approx(T.x_left));
    CHECK(kicks[0].hi[0] == doctest::Approx(T.x_right));
    CHECK(kicks[0].lo[1] == doctest::Approx(T.y - half));
    CHECK(kicks[0].hi[1] == doctest::Approx(T.y + half));
    CHECK(kicks[0].material.kind == MaterialKind::TextureOverlay);
    CHECK(kicks[0].material.texture ==
          (in_sp_phrase(scene, ghost->tick) ? TextureId::LongEnergy : TextureId::LongKick));
    CHECK(kicks[0].material.overlay == TextureId::OverlayGhost);
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
            // Pinned from one run; the fade length is the shipped json's
            // targets_secs_light.
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

// D53 item 3: each taken fill lights its own activation lane, even where two
// taken fills touch. The stretches and their pads are the track state's
// make_lane_bounds; on each of Pitch Dark's activations through a fill, the
// frame draws one lit-target strip over each stretch, on that stretch's lane,
// and the activation's own lane is among them. The lit lane is the pad the
// activation lane names (PreviewLane Red..Green after Kick, Pad Red..Green).
TEST_CASE("build_highway_draws: each taken fill lights its own lane over its stretch") {
    const PreviewConfig cfg = shipped_preview_config();
    const PreviewScene& scene = pitch_dark_scenes().path;
    const TrackState st = build_track_state(scene, TrackStateOptions{});
    int checked = 0;
    for (size_t i = 0; i < scene.activations.size(); ++i) {
        const PreviewActivation& a = scene.activations[i];
        if (!a.taken_fill || !a.has_lane || a.lane == PreviewLane::Kick) continue;
        CAPTURE(i);
        const HighwayView v = view_at(cfg, scene.fills[*a.taken_fill].span.start_ms / 1000.0);
        const std::vector<LaneSpan> stretches =
            st.make_lane_bounds(st.window(v.near_t, v.far_t), v.near_t, v.far_t);
        REQUIRE_FALSE(stretches.empty());
        const std::vector<DrawCommand> cmds = build_highway_draws(st, cfg, v.now, v.speed);
        bool own_lane = false;
        for (const LaneSpan& s : stretches) {
            int lit = 0;
            for (const DrawCommand* c : strips_over(cmds, cfg, v, s.pad, s.t1, s.t2))
                if (is_lit_target_tex(c->material.texture)) {
                    ++lit;
                    CHECK(c->alpha == doctest::Approx(1.0f));
                }
            CHECK(lit == 1);
            if (static_cast<int>(s.pad) + 1 == static_cast<int>(a.lane)) own_lane = true;
        }
        CHECK(own_lane);
        ++checked;
    }
    // Every Clone Hero drum activation is on a fill, and "3+ 2+ 1" has three.
    CHECK(checked > 0);
}

// Pitch Dark's path skips fills ("3+ 2+ 1" passes over offered ones): each
// offered fill's window draws its four lane strips at fill_offered_alpha, and
// no lit lane, since nothing activates there.
TEST_CASE("build_highway_draws: an offered fill's strips are dimmed") {
    const PreviewConfig cfg = shipped_preview_config();
    const PreviewScene& scene = pitch_dark_scenes().path;
    const TrackState st = build_track_state(scene, TrackStateOptions{});
    int checked = 0;
    for (size_t i = 0; i < scene.fills.size(); ++i) {
        if (scene.fills[i].state != PreviewFillState::Offered) continue;
        CAPTURE(i);
        const HighwayView v = view_at(cfg, scene.fills[i].span.start_ms / 1000.0);
        const std::vector<DrawCommand> cmds = build_highway_draws(st, cfg, v.now, v.speed);
        for (const ToggleSpan& s : bounds(st, v, &TrackInstant::fill)) {
            if (!s.on) continue;
            for (const Pad p : kPads) {
                const std::vector<const DrawCommand*> on = strips_over(cmds, cfg, v, p, s.t1, s.t2);
                REQUIRE(on.size() == 1);
                CHECK(is_lane_tex(on[0]->material.texture));
                CHECK(on[0]->alpha == doctest::Approx(cfg.hydra.fill_offered_alpha));
            }
            ++checked;
        }
    }
    CHECK(checked > 0);
}

// Pitch Dark's path "3+ 2+ 1" leaves fills Hidden. The overlay marks only the
// fills each activation passed over and the one it stands on
// (apply_preview_overlay), so a fill under a running activation, or before SP
// is ready again, stays Hidden. The record's first activation, at tick 109440,
// runs to 132240, and SP is ready for the next at 151440
// (testdata/replay/test_pitch_dark_ratio-schedule.json); fills fall in
// between. The chart has no drum lane, and every lane strip a fill draws sits
// on an offered or taken fill's span, so a frame on a hidden fill draws no
// lane strip of its own over that fill's stretch.
//
// One thing in these textures is not a fill's: the glow after a hit. It is
// drawn in the lit target texture on the strike line's fixed depths, not over
// a stretch of time, and a playhead on the fill's start sits on that line.
// The draw code tells it apart by those depths, so the count skips a strip on
// exactly the strike line's depths.
TEST_CASE("build_highway_draws: a hidden fill draws no lane strips") {
    const PreviewConfig cfg = shipped_preview_config();
    const PreviewScene& scene = pitch_dark_scenes().path;
    REQUIRE(scene.drum_lanes.empty());
    const TrackState st = build_track_state(scene, TrackStateOptions{});
    const double glow_a = cfg.track.z_now + cfg.track.targets_z_past;
    const double glow_b = cfg.track.z_now + cfg.track.targets_z_future;
    const double glow_lo = std::min(glow_a, glow_b);
    const double glow_hi = std::max(glow_a, glow_b);
    int checked = 0;
    for (size_t i = 0; i < scene.fills.size(); ++i) {
        if (scene.fills[i].state != PreviewFillState::Hidden) continue;
        CAPTURE(i);
        const HighwayView v = view_at(cfg, scene.fills[i].span.start_ms / 1000.0);
        const double t1 = std::max(v.near_t, scene.fills[i].span.start_ms / 1000.0);
        const double t2 = std::min(v.far_t, scene.fills[i].span.end_ms / 1000.0);
        REQUIRE(t2 > t1);
        const double za = time_to_z(cfg, v.now, t1, v.speed);
        const double zb = time_to_z(cfg, v.now, t2, v.speed);
        const double z_lo = std::min(za, zb);
        const double z_hi = std::max(za, zb);
        int over = 0;
        for (const DrawCommand& c : build_highway_draws(st, cfg, v.now, v.speed)) {
            if (!is_lane_tex(c.material.texture) && !is_lit_target_tex(c.material.texture)) continue;
            const double c_lo = std::min<double>(c.lo[2], c.hi[2]);
            const double c_hi = std::max<double>(c.lo[2], c.hi[2]);
            const bool glow = is_lit_target_tex(c.material.texture) &&
                              c_lo == doctest::Approx(glow_lo) && c_hi == doctest::Approx(glow_hi);
            if (glow) continue;
            // A strip that only touches the hidden fill's edge (equal depth,
            // as strips_over compares depths) belongs to a neighbouring fill.
            const bool touches = c_hi == doctest::Approx(z_lo) || c_lo == doctest::Approx(z_hi);
            if (c_hi > z_lo && c_lo < z_hi && !touches) ++over;
        }
        CHECK(over == 0);
        ++checked;
    }
    CHECK(checked > 0);
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

// Snakeskin Boots' drum lane, with the playhead on its start. Each pad the
// track state lights (its drum-lane span field) draws one strip in the lane
// texture at full strength, depth off, over exactly that stretch; the song has
// no fill there, so these are the frame's only lane strips.
TEST_CASE("build_highway_draws: drum lanes light their own pads in the lane texture") {
    const PreviewConfig cfg = shipped_preview_config();
    const PreviewScene& scene = snakeskin_base();
    REQUIRE_FALSE(scene.drum_lanes.empty());
    const TrackState st = build_track_state(scene, TrackStateOptions{});
    int checked = 0;
    for (size_t i = 0; i < scene.drum_lanes.size(); ++i) {
        CAPTURE(i);
        const HighwayView v = view_at(cfg, scene.drum_lanes[i].span.start_ms / 1000.0);
        const std::vector<DrawCommand> cmds = build_highway_draws(st, cfg, v.now, v.speed);
        int lit = 0;
        for (const Pad p : kPads) {
            for (const ToggleSpan& s : bounds(st, v, drum_lane_field(p))) {
                if (!s.on) continue;
                const std::vector<const DrawCommand*> on = strips_over(cmds, cfg, v, p, s.t1, s.t2);
                REQUIRE(on.size() == 1);
                CHECK(is_lane_tex(on[0]->material.texture));
                CHECK(on[0]->alpha == doctest::Approx(1.0f));
                ++lit;
            }
        }
        CHECK(lit > 0);
        int lanes = 0;
        for (const DrawCommand& c : cmds)
            if (is_lane_tex(c.material.texture)) ++lanes;
        CHECK(lanes == lit);
        checked += lit;
    }
    CHECK(checked > 0);
}

// Where a drum lane and a fill are drawn together, the lane's strips come
// first in the draw list: Onyx's drawLanes draws its roll lanes before its BRE
// strips. Two corpus charts have a drum lane within a couple of beats of a
// fill (White Mist, The Decade Of Statues); with no path, every fill is
// Offered. A lane can be longer than the whole view (view_at's near_t to
// far_t), so a pair is checked with each only partly in view. The
// playhead sits 50 ms (a free input) before the earlier of the two ends, so
// the earlier one still shows at the near end, and a pair counts when the
// later one starts before the far end. In that frame every strip of the
// track state's lit-lane stretches precedes every strip of its fill
// stretches.
TEST_CASE("build_highway_draws: drum lane strips come before the fill strips") {
    const PreviewConfig cfg = shipped_preview_config();
    const AnalysisSettings as = Settings().to_analysis_settings();
    int checked = 0;
    for (const char* name : {"White Mist", "The Decade Of Statues"}) {
        CAPTURE(name);
        const std::string chart = corpus::chart_named(name);
        if (chart.empty()) throw std::runtime_error(std::string(name) + " is not in the corpus");
        const PreviewScene scene = build_preview_scene(corpus::song(chart, as.reading(), as.rules),
                                                       nullptr, as.sp_cap, as.rules);
        const TrackState st = build_track_state(scene, TrackStateOptions{});
        for (const PreviewDrumLane& lane : scene.drum_lanes) {
            for (const PreviewFill& fill : scene.fills) {
                if (fill.state != PreviewFillState::Offered) continue;
                const bool lane_first = lane.span.start_ms <= fill.span.start_ms;
                const PreviewSpan& earlier = lane_first ? lane.span : fill.span;
                const PreviewSpan& later = lane_first ? fill.span : lane.span;
                const HighwayView v = view_at(cfg, earlier.end_ms / 1000.0 - 0.05);
                // Not in view together: the later one starts past the far
                // end, or sits inside the earlier one and ends before the
                // near end.
                if (later.start_ms / 1000.0 >= v.far_t || later.end_ms / 1000.0 <= v.near_t)
                    continue;
                CAPTURE(lane.span.start_ms);
                CAPTURE(fill.span.start_ms);
                const std::vector<DrawCommand> cmds = build_highway_draws(st, cfg, v.now, v.speed);
                long last_lane = -1, first_fill = static_cast<long>(cmds.size());
                for (const Pad p : kPads) {
                    for (const ToggleSpan& s : bounds(st, v, drum_lane_field(p))) {
                        if (!s.on) continue;
                        for (const DrawCommand* c : strips_over(cmds, cfg, v, p, s.t1, s.t2))
                            last_lane = std::max(last_lane, static_cast<long>(c - cmds.data()));
                    }
                    for (const ToggleSpan& s : bounds(st, v, &TrackInstant::fill)) {
                        if (!s.on) continue;
                        for (const DrawCommand* c : strips_over(cmds, cfg, v, p, s.t1, s.t2))
                            first_fill = std::min(first_fill, static_cast<long>(c - cmds.data()));
                    }
                }
                if (last_lane < 0) continue;  // a kick-only lane lights no pad
                REQUIRE(first_fill < static_cast<long>(cmds.size()));
                CHECK(last_lane < first_fill);
                ++checked;
            }
        }
    }
    CHECK(checked > 0);
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
    CHECK(c.depth == hydra::render::DepthMode::Always);
    CHECK(c.lo[1] == doctest::Approx(cfg.track.y));
    CHECK(c.lo[0] == doctest::Approx(x1));
    CHECK(c.hi[0] == doctest::Approx(x2));
    CHECK(c.lo[2] == doctest::Approx(z1));
    CHECK(c.hi[2] == doctest::Approx(z2));
}

// The outline's line width: a beat line's depth in the config.
double outline_width(const PreviewConfig& cfg) {
    return cfg.track.beats_z_past - cfg.track.beats_z_future;
}

// What one frame's ghost outline showed.
struct OutlineSeen {
    int whole = 0;     // ghost stretches drawn with both edges
    int near_cut = 0;  // stretches the near end cuts (no near edge)
    int far_cut = 0;   // stretches the far end cuts (no far edge)
};

// Pitch Dark's path drawn with and without its ghost at view `v`: the ghost
// adds exactly one outline per lit ghost stretch of the track state
// (fill_ghost), and leaves every other draw as it was. Each stretch has its
// near edge across each lane when its start is in view, its far edge when
// its end is, then the two sides between them.
OutlineSeen check_ghost_outline(const TrackState& ghosted, const TrackState& plain,
                                const PreviewConfig& cfg, const HighwayView& v) {
    const std::vector<DrawCommand> outline =
        inserted_run(build_highway_draws(ghosted, cfg, v.now, v.speed),
                     build_highway_draws(plain, cfg, v.now, v.speed));
    const double w = outline_width(cfg);
    const float xl = cfg.track.x_left, xr = cfg.track.x_right;
    OutlineSeen seen;
    size_t at = 0;
    for (const ToggleSpan& s : bounds(ghosted, v, &TrackInstant::fill_ghost)) {
        if (!s.on) continue;
        const bool has_near = s.t1 > v.near_t, has_far = s.t2 < v.far_t;
        const double z_near = time_to_z(cfg, v.now, s.t1, v.speed);
        const double z_far = time_to_z(cfg, v.now, s.t2, v.speed);
        // A real fill is longer than two line widths, so its sides are drawn.
        const double side_near = has_near ? z_near - w : z_near;
        const double side_far = has_far ? z_far + w : z_far;
        REQUIRE(side_near > side_far);
        const size_t lines = kPads.size() * ((has_near ? 1 : 0) + (has_far ? 1 : 0)) + 2;
        REQUIRE(outline.size() >= at + lines);
        for (const Pad p : kPads) {
            float x1, x2;
            pad_x(cfg, p, x1, x2);
            if (has_near) check_outline_line(outline[at++], cfg, p, x1, x2, z_near, z_near - w);
            if (has_far) check_outline_line(outline[at++], cfg, p, x1, x2, z_far + w, z_far);
        }
        check_outline_line(outline[at++], cfg, Pad::Red, xl, xl + static_cast<float>(w),
                           side_near, side_far);
        check_outline_line(outline[at++], cfg, Pad::Green, xr - static_cast<float>(w), xr,
                           side_near, side_far);
        if (has_near && has_far) ++seen.whole;
        if (!has_near) ++seen.near_cut;
        if (!has_far) ++seen.far_cut;
    }
    CHECK(outline.size() == at);
    return seen;
}

}  // namespace

// Each of the ghost path's fills on Pitch Dark, with the playhead just before
// it and the highway sped up so the whole fill is in view: the ghost adds the
// offered strip's outline, the near and far edge across each lane and the two
// sides, and nothing else changes. The speed is a free input, chosen so the
// far end passes the fill's end.
TEST_CASE("build_highway_draws: a ghost fill draws the offered strip's outline, nothing else does") {
    const PreviewConfig cfg = shipped_preview_config();
    const PitchDarkScenes& pd = pitch_dark_scenes();
    REQUIRE_FALSE(pd.ghosted.ghost_fills.empty());
    const TrackState ghosted = build_track_state(pd.ghosted, TrackStateOptions{});
    const TrackState plain = build_track_state(pd.path, TrackStateOptions{});
    int whole = 0;
    for (const size_t g : pd.ghosted.ghost_fills) {
        CAPTURE(g);
        const PreviewSpan& f = pd.ghosted.fills[g].span;
        const double now = f.start_ms / 1000.0 - 0.05;  // a free input: 50 ms before it
        const double ahead = f.end_ms / 1000.0 + 0.1 - now;
        const double speed = std::max(1.0, 2.0 * ahead / cfg.track.secs_future);
        whole += check_ghost_outline(ghosted, plain, cfg, view_at(cfg, now, speed)).whole;
    }
    CHECK(whole > 0);
}

// A ghost fill the window cuts has no edge at the cut. With the far end on the
// middle of each ghost fill it has no far edge, and with the near end there it
// has no near edge; the sides stop at the cut.
TEST_CASE("build_highway_draws: a ghost fill cut by the window has no edge at the cut") {
    const PreviewConfig cfg = shipped_preview_config();
    const PitchDarkScenes& pd = pitch_dark_scenes();
    REQUIRE_FALSE(pd.ghosted.ghost_fills.empty());
    const TrackState ghosted = build_track_state(pd.ghosted, TrackStateOptions{});
    const TrackState plain = build_track_state(pd.path, TrackStateOptions{});
    // How far the far and near ends sit from the playhead at speed 1.
    const HighwayView at0 = view_at(cfg, 0.0);
    int near_cut = 0, far_cut = 0;
    for (const size_t g : pd.ghosted.ghost_fills) {
        CAPTURE(g);
        const PreviewSpan& f = pd.ghosted.fills[g].span;
        const double mid = (f.start_ms + f.end_ms) / 2000.0;
        far_cut += check_ghost_outline(ghosted, plain, cfg, view_at(cfg, mid - at0.far_t)).far_cut;
        near_cut += check_ghost_outline(ghosted, plain, cfg, view_at(cfg, mid - at0.near_t)).near_cut;
    }
    CHECK(far_cut > 0);
    CHECK(near_cut > 0);
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

// On each of Pitch Dark's SP phrases, the last chord inside it (its tick at or
// before the phrase's end tick) draws every gem in an energy texture, and the
// first chord after it draws every gem plain.
TEST_CASE("build_highway_draws: the chord after a phrase ends draws plain") {
    const PreviewConfig cfg = shipped_preview_config();
    const PreviewScene& scene = pitch_dark_scenes().bare;
    REQUIRE_FALSE(scene.sp_phrases.empty());
    const TrackState st = build_track_state(scene, TrackStateOptions{});
    int checked = 0;
    for (size_t k = 0; k < scene.sp_phrases.size(); ++k) {
        CAPTURE(k);
        const PreviewSpan& ph = scene.sp_phrases[k];
        const PreviewNote* last = nullptr;
        const PreviewNote* next = nullptr;
        for (const PreviewNote& n : scene.notes) {
            if (n.tick >= ph.start_tick && n.tick <= ph.end_tick &&
                (last == nullptr || n.tick > last->tick))
                last = &n;
            if (n.tick > ph.end_tick && (next == nullptr || n.tick < next->tick)) next = &n;
        }
        REQUIRE(last != nullptr);
        const std::vector<bool> inside = chord_energy(st, cfg, last->ms);
        REQUIRE_FALSE(inside.empty());
        for (const bool e : inside) CHECK(e);
        if (next == nullptr) continue;  // a phrase on the song's last chord
        const std::vector<bool> after = chord_energy(st, cfg, next->ms);
        REQUIRE_FALSE(after.empty());
        // The chord after may open the next phrase; only one outside every
        // phrase must draw plain.
        bool in_a_phrase = false;
        for (const PreviewSpan& other : scene.sp_phrases)
            if (next->tick >= other.start_tick && next->tick <= other.end_tick) in_a_phrase = true;
        if (in_a_phrase) continue;
        for (const bool e : after) CHECK_FALSE(e);
        ++checked;
    }
    CHECK(checked > 0);
}

// Option B of the SP end mocks: the railings draw in the SP colour while
// active SP is on, so they change colour where it runs out, and nothing marks
// the end on the lanes.
namespace {

// One side's railing boxes, in draw order (near to far).
std::vector<DrawCommand> rail_boxes(const std::vector<DrawCommand>& cmds, bool right) {
    std::vector<DrawCommand> out;
    for (const DrawCommand& c : cmds)
        if (c.mesh == MeshId::Box && (c.lo[0] > 0.0f) == right) out.push_back(c);
    return out;
}

// The depths where a run of boxes or flats changes colour from one to the
// next.
std::vector<float> color_cuts(const std::vector<DrawCommand>& run) {
    std::vector<float> cuts;
    for (size_t i = 0; i + 1 < run.size(); ++i)
        if (!same_color(run[i].material.color, run[i + 1].material.color))
            cuts.push_back(run[i].hi[2]);
    return cuts;
}

// Checks one railing box runs from depth z1 to z2 in colour `col`.
void check_rail(const DrawCommand& c, float z1, float z2, const Color& col) {
    CHECK(c.lo[2] == doctest::Approx(z1));
    CHECK(c.hi[2] == doctest::Approx(z2));
    CHECK(c.material.kind == MaterialKind::Color);
    CHECK(same_color(c.material.color, col));
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

namespace {

// The rails of one frame against the track state's active-SP stretches
// (sp_active) over view `v`: on each side one box per stretch, near to far,
// over exactly its depths, in the SP colour while SP runs and grey otherwise.
// Every depth where a rail changes colour is also a cut in the floor, and a
// lit box has a grey box's size, mesh, depth test, light and alpha. Returns
// the stretches.
std::vector<ToggleSpan> check_rails_follow_sp(const TrackState& st, const PreviewConfig& cfg,
                                              const HighwayView& v) {
    const std::vector<DrawCommand> cmds = build_highway_draws(st, cfg, v.now, v.speed);
    const std::vector<ToggleSpan> sp = bounds(st, v, &TrackInstant::sp_active);
    const std::vector<float> floor_cuts = color_cuts(floor_flats(cmds));
    for (bool right : {false, true}) {
        CAPTURE(right);
        const std::vector<DrawCommand> rails = rail_boxes(cmds, right);
        REQUIRE(rails.size() == sp.size());
        for (size_t k = 0; k < sp.size(); ++k) {
            CAPTURE(k);
            check_rail(rails[k], static_cast<float>(time_to_z(cfg, v.now, sp[k].t1, v.speed)),
                       static_cast<float>(time_to_z(cfg, v.now, sp[k].t2, v.speed)),
                       sp[k].on ? cfg.hydra.sp_active_color : cfg.track.railing_color);
            CHECK(rails[k].mesh == rails[0].mesh);
            CHECK(rails[k].lo[0] == rails[0].lo[0]);
            CHECK(rails[k].hi[0] == rails[0].hi[0]);
            CHECK(rails[k].lo[1] == rails[0].lo[1]);
            CHECK(rails[k].hi[1] == rails[0].hi[1]);
            CHECK(rails[k].depth == rails[0].depth);
            CHECK(rails[k].light == rails[0].light);
            CHECK(rails[k].alpha == rails[0].alpha);
        }
        for (const float cut : color_cuts(rails)) {
            CAPTURE(cut);
            bool in_floor = false;
            for (const float f : floor_cuts)
                if (f == doctest::Approx(cut)) in_floor = true;
            CHECK(in_floor);
        }
    }
    // The first two boxes are still the left rail, then the right.
    const std::vector<const DrawCommand*> boxes = of_mesh(cmds, MeshId::Box);
    REQUIRE(boxes.size() == 2 * sp.size());
    CHECK(boxes[0]->lo[0] < 0.0f);
    CHECK(boxes[1]->lo[0] > 0.0f);
    return sp;
}

}  // namespace

// Each of Pitch Dark's SP windows on its path, with the playhead just before
// the window starts and again just before it ends (free inputs: 200 ms): the
// rails turn the SP colour where SP starts and grey where it runs out.
TEST_CASE("build_highway_draws: the railings turn the SP colour inside an active window") {
    const PreviewConfig cfg = shipped_preview_config();
    const PreviewScene& scene = pitch_dark_scenes().path;
    const TrackState st = build_track_state(scene, TrackStateOptions{});
    int starts = 0, ends = 0;
    for (size_t i = 0; i < scene.activations.size(); ++i) {
        const std::optional<std::pair<double, double>> w = scene.activations[i].sp_window();
        if (!w) continue;
        CAPTURE(i);
        const std::vector<ToggleSpan> at_start =
            check_rails_follow_sp(st, cfg, view_at(cfg, w->first / 1000.0 - 0.2));
        // The window's start is in view: a grey rail, then a lit one that
        // starts there.
        bool lights_up = false;
        for (size_t k = 0; k + 1 < at_start.size(); ++k)
            if (!at_start[k].on && at_start[k + 1].on &&
                at_start[k + 1].t1 == doctest::Approx(w->first / 1000.0))
                lights_up = true;
        CHECK(lights_up);
        ++starts;
        const std::vector<ToggleSpan> at_end =
            check_rails_follow_sp(st, cfg, view_at(cfg, w->second / 1000.0 - 0.2));
        // SP runs out in view: some lit rail is followed by a grey one.
        bool runs_out = false;
        for (size_t k = 0; k + 1 < at_end.size(); ++k)
            if (at_end[k].on && !at_end[k + 1].on) runs_out = true;
        CHECK(runs_out);
        ++ends;
    }
    CHECK(starts > 0);
    CHECK(ends > 0);
}

// Pitch Dark with no path has no SP window: on every phrase, both rails are
// one grey box from the near end to the far end.
TEST_CASE("build_highway_draws: no SP window in view leaves two grey railings") {
    const PreviewConfig cfg = shipped_preview_config();
    const PreviewScene& scene = pitch_dark_scenes().bare;
    REQUIRE(scene.activations.empty());
    REQUIRE_FALSE(scene.sp_phrases.empty());
    const TrackState st = build_track_state(scene, TrackStateOptions{});
    for (size_t k = 0; k < scene.sp_phrases.size(); ++k) {
        CAPTURE(k);
        const std::vector<DrawCommand> cmds =
            build_highway_draws(st, cfg, scene.sp_phrases[k].start_ms / 1000.0, 1.0);
        const std::vector<const DrawCommand*> boxes = of_mesh(cmds, MeshId::Box);
        REQUIRE(boxes.size() == 2);
        CHECK(boxes[0]->lo[0] < 0.0f);
        CHECK(boxes[1]->lo[0] > 0.0f);
        for (const DrawCommand* b : boxes)
            check_rail(*b, cfg.track.z_past, cfg.track.z_future, cfg.track.railing_color);
    }
}

// Each of Pitch Dark's SP windows with the view's far end and then its near
// end on the window's start and end: where the view cuts a window, the rail
// box simply stops at the view's edge, with no extra cut there. With the near
// end inside a window the nearest box is lit; with the far end inside one the
// farthest is.
TEST_CASE("build_highway_draws: an SP window clipped by the view shows no change at the clip") {
    const PreviewConfig cfg = shipped_preview_config();
    const PreviewScene& scene = pitch_dark_scenes().path;
    const TrackState st = build_track_state(scene, TrackStateOptions{});
    // How far the far and near ends sit from the playhead at speed 1.
    const HighwayView at0 = view_at(cfg, 0.0);
    int clips = 0;
    for (size_t i = 0; i < scene.activations.size(); ++i) {
        const std::optional<std::pair<double, double>> w = scene.activations[i].sp_window();
        if (!w) continue;
        CAPTURE(i);
        // A free input: 50 ms inside the window from each end.
        const double inside_start = w->first / 1000.0 + 0.05;
        const double inside_end = w->second / 1000.0 - 0.05;
        // The far end just inside the window's start: the farthest box is lit.
        const std::vector<ToggleSpan> far_in =
            check_rails_follow_sp(st, cfg, view_at(cfg, inside_start - at0.far_t));
        REQUIRE_FALSE(far_in.empty());
        CHECK(far_in.back().on);
        // The near end just inside the window's end: the nearest box is lit.
        const std::vector<ToggleSpan> near_in =
            check_rails_follow_sp(st, cfg, view_at(cfg, inside_end - at0.near_t));
        REQUIRE_FALSE(near_in.empty());
        CHECK(near_in.front().on);
        ++clips;
    }
    CHECK(clips > 0);
}

// Pitch Dark has one solo. With no path, a frame on it shows the solo on the
// floor and two grey rails. On the path, frames on the solo's start, middle
// and end draw rails that follow active SP alone.
TEST_CASE("build_highway_draws: a solo neither cuts nor colours the railings") {
    const PreviewConfig cfg = shipped_preview_config();
    const PitchDarkScenes& pd = pitch_dark_scenes();
    REQUIRE_FALSE(pd.bare.solos.empty());
    const TrackState bare = build_track_state(pd.bare, TrackStateOptions{});
    const TrackState path = build_track_state(pd.path, TrackStateOptions{});
    for (size_t k = 0; k < pd.bare.solos.size(); ++k) {
        CAPTURE(k);
        const PreviewSpan& solo = pd.bare.solos[k];
        const double now = solo.start_ms / 1000.0;
        const std::vector<DrawCommand> cmds = build_highway_draws(bare, cfg, now, 1.0);
        bool saw_solo = false;
        for (const DrawCommand& f : floor_flats(cmds))
            if (same_color(f.material.color, cfg.track.color_solo)) saw_solo = true;
        CHECK(saw_solo);
        const std::vector<const DrawCommand*> boxes = of_mesh(cmds, MeshId::Box);
        REQUIRE(boxes.size() == 2);
        for (const DrawCommand* b : boxes)
            check_rail(*b, cfg.track.z_past, cfg.track.z_future, cfg.track.railing_color);
        for (const double t : {solo.start_ms, (solo.start_ms + solo.end_ms) / 2.0, solo.end_ms}) {
            CAPTURE(t);
            check_rails_follow_sp(path, cfg, view_at(cfg, t / 1000.0));
        }
    }
}

// Pitch Dark's path drawn with and without the record's best path as its
// ghost: on each of the ghost's SP windows, the rails and the floor's
// coloured flats are the same draws either way.
TEST_CASE("build_highway_draws: a ghost path's SP windows do not reach the highway") {
    const PreviewConfig cfg = shipped_preview_config();
    const PitchDarkScenes& pd = pitch_dark_scenes();
    REQUIRE_FALSE(pd.ghosted.ghost_windows.empty());
    const TrackState ghosted = build_track_state(pd.ghosted, TrackStateOptions{});
    const TrackState plain = build_track_state(pd.path, TrackStateOptions{});
    for (size_t k = 0; k < pd.ghosted.ghost_windows.size(); ++k) {
        CAPTURE(k);
        const std::pair<double, double>& w = pd.ghosted.ghost_windows[k];
        for (const double t : {w.first, (w.first + w.second) / 2.0}) {
            CAPTURE(t);
            const std::vector<DrawCommand> got = build_highway_draws(ghosted, cfg, t / 1000.0, 1.0);
            const std::vector<DrawCommand> want = build_highway_draws(plain, cfg, t / 1000.0, 1.0);
            const std::vector<const DrawCommand*> got_boxes = of_mesh(got, MeshId::Box);
            const std::vector<const DrawCommand*> want_boxes = of_mesh(want, MeshId::Box);
            REQUIRE(got_boxes.size() == want_boxes.size());
            for (size_t i = 0; i < got_boxes.size(); ++i) CHECK(same_draw(*got_boxes[i], *want_boxes[i]));
            const std::vector<DrawCommand> got_floor = floor_flats(got);
            const std::vector<DrawCommand> want_floor = floor_flats(want);
            REQUIRE(got_floor.size() == want_floor.size());
            for (size_t i = 0; i < got_floor.size(); ++i) CHECK(same_draw(got_floor[i], want_floor[i]));
        }
    }
}

TEST_CASE("build_highway_draws: nothing marks the SP end on the lanes") {
    const PreviewConfig cfg = shipped_preview_config();
    TrackState st = build_track_state(sp_end_scene(), TrackStateOptions{});
    // now 1.0 s: the 2.0 s end is in view.
    int colored = 0;
    for (const DrawCommand& c : build_highway_draws(st, cfg, 1.0, 1.0)) {
        if (c.material.kind != MaterialKind::Color) continue;
        ++colored;
        const bool floor = c.mesh == MeshId::Flat && c.depth == hydra::render::DepthMode::Less;
        CHECK((floor || c.mesh == MeshId::Box));
    }
    CHECK(colored > 0);
}
