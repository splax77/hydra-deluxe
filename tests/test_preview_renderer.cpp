// Tests for render/preview_renderer: the D3D11 executor of the Onyx port, run
// on a headless WARP device with the real assets (HYDRA_ASSET_DIR). Each case
// renders, reads the pixels back, and checks what Onyx's layout implies: the
// background colour outside the highway, a lit floor at the strike line, the
// horizon fade across the top of the track, gems adding pixels, MSAA, resize.

#include "doctest.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

#include "app/preview_view.h"
#include "app/user_messages.h"
#include "core/winstr.h"  // os_path
#include "render/highway_draw.h"  // track_height, build_highway_draws
#include "render/overlay_layout.h"  // project_to_image
#include "render/preview_renderer.h"
#include "preview_scene_fixtures.h"
#include "temp_util.h"
#include "ui/preview_load_job.h"  // track_options
#include "warp_util.h"

#ifndef HYDRA_ASSET_DIR
#error "HYDRA_ASSET_DIR must be defined (see CMakeLists.txt)"
#endif

using namespace hydra::render;
using hydra::app::PreviewLane;
using hydra::app::PreviewScene;
using hydra::test_fixtures::note;
using hydra::test_fixtures::span;
using hydra::test_fixtures::timed_scene;
using Microsoft::WRL::ComPtr;

namespace {

const std::string kAssets = HYDRA_ASSET_DIR;

// Is the pixel the renderer's background colour (view.background in the
// shipped 3d-config.json), each channel within 2 of it as a byte.
bool is_background(const PreviewConfig& cfg, const uint8_t* p) {
    const Color& bg = cfg.view.background;
    auto within = [](uint8_t got, float want) {
        return std::abs(got - static_cast<int>(std::lround(want * 255.0f))) <= 2;
    };
    return within(p[0], bg.r) && within(p[1], bg.g) && within(p[2], bg.b);
}

// One pixel of a w x h image, by column and row.
struct PixelAt {
    int x = 0;
    int y = 0;
};

// The pixel world point `p` lands on, through the renderer's camera
// (project_to_image). It must lie inside the image.
PixelAt projected_pixel(const PreviewConfig& cfg, int w, int h, const Vec3& p) {
    const ImagePoint ip = project_to_image(cfg, w, h, p);
    const PixelAt at{static_cast<int>(ip.x), static_cast<int>(ip.y)};
    REQUIRE(at.x >= 0);
    REQUIRE(at.x < w);
    REQUIRE(at.y >= 0);
    REQUIRE(at.y < h);
    return at;
}

// Where the centre of the one gem drawn with texture `tex` in `scene` at
// `now_ms` lands: its draw box from build_highway_draws, projected. The
// renderer draws at 1x speed (kPlaybackSpeed in render/preview_renderer.h).
PixelAt gem_pixel(const PreviewConfig& cfg, const PreviewScene& scene, double now_ms, TextureId tex, int w,
                  int h) {
    const TrackState state = build_track_state(scene, TrackStateOptions{});
    const std::vector<DrawCommand> draws = build_highway_draws(state, cfg, now_ms / 1000.0, kPlaybackSpeed);
    const DrawCommand* gem = nullptr;
    for (const DrawCommand& c : draws)
        if (c.light == LightKind::GemOffset && c.material.texture == tex) {
            REQUIRE(gem == nullptr);
            gem = &c;
        }
    REQUIRE(gem != nullptr);
    const Vec3 mid{(gem->lo[0] + gem->hi[0]) * 0.5f, (gem->lo[1] + gem->hi[1]) * 0.5f,
                   (gem->lo[2] + gem->hi[2]) * 0.5f};
    return projected_pixel(cfg, w, h, mid);
}

// The pixel at that gem's centre in `img`.
const uint8_t* gem_centre(const PreviewConfig& cfg, const PreviewScene& scene, double now_ms,
                          TextureId tex, const std::vector<uint8_t>& img, int w, int h) {
    const PixelAt at = gem_pixel(cfg, scene, now_ms, tex, w, h);
    return warp::pixel(img, w, at.x, at.y);
}

// A pixel of the floor just past the strike line, toward the player: on the
// floor (track.y), in the note area's middle, halfway down from the near edge
// of the strike-line targets (track.targets_z_past) to the image's bottom
// row. No gem in these scenes has reached the strike line, so only the floor
// is drawn there.
PixelAt floor_near_strike(const PreviewConfig& cfg, int w, int h) {
    const Vec3 near_edge{(cfg.track.x_left + cfg.track.x_right) * 0.5f, cfg.track.y, cfg.track.targets_z_past};
    const PixelAt edge = projected_pixel(cfg, w, h, near_edge);
    REQUIRE(edge.y < h - 1);
    return PixelAt{edge.x, (edge.y + h) / 2};
}

const uint8_t* floor_pixel(const PreviewConfig& cfg, const std::vector<uint8_t>& img, int w, int h) {
    const PixelAt at = floor_near_strike(cfg, w, h);
    return warp::pixel(img, w, at.x, at.y);
}

// Every row of a w x h image past the horizon fade's end is background, in
// every column: the rows above the track rectangle (track_rect), and the
// track's rows at or past view.track_fade_top of its height from the bottom,
// where fade.hlsl draws nothing. A row counts when its lower edge is past the
// fade's end, so every pixel centre in it is.
void check_past_fade_is_background(const PreviewConfig& cfg, const std::vector<uint8_t>& img, int w, int h) {
    const TrackRect rect = track_rect(cfg, w, h);
    const float fade_end_rows = cfg.view.track_fade_top * static_cast<float>(rect.track_height);
    int checked = 0;
    for (int y = 0; y < h; ++y) {
        if (static_cast<float>(h - 1 - y) < fade_end_rows) continue;
        ++checked;
        for (int x = 0; x < w; ++x) {
            INFO("pixel " << x << "," << y << " of " << w << "x" << h);
            CHECK(is_background(cfg, warp::pixel(img, w, x, y)));
        }
    }
    REQUIRE(checked > 0);
}

}  // namespace

TEST_CASE("PreviewRenderer: bare highway — background outside, lit floor, horizon fade (WARP)") {
    ComPtr<ID3D11Device> dev;
    ComPtr<ID3D11DeviceContext> ctx;
    REQUIRE(warp::make_device(dev, ctx));

    // Wider than tall: the track rect is the full target (h < w * 1.1666).
    const int W = 160, H = 120;
    PreviewRenderer r(dev.Get(), ctx.Get(), kAssets);
    r.resize(W, H);
    REQUIRE(r.texture_srv() != nullptr);
    CHECK(r.config().hydra.msaa == 4);

    PreviewScene empty;
    r.set_scene(empty);
    r.render(1000.0);
    std::vector<uint8_t> img = warp::read_pixels(dev.Get(), ctx.Get(), r.texture_srv(), W, H);

    // Every pixel is opaque (ImGui composites by alpha).
    for (size_t i = 3; i < img.size(); i += 4) REQUIRE(img[i] == 255);

    // Past the horizon fade's end, every pixel is background.
    check_past_fade_is_background(r.config(), img, W, H);

    // The floor by the strike line is drawn.
    CHECK_FALSE(is_background(r.config(), floor_pixel(r.config(), img, W, H)));
}

TEST_CASE("PreviewRenderer: a gem at the strike line adds drawn pixels (WARP)") {
    ComPtr<ID3D11Device> dev;
    ComPtr<ID3D11DeviceContext> ctx;
    REQUIRE(warp::make_device(dev, ctx));

    const int W = 160, H = 120;
    PreviewRenderer r(dev.Get(), ctx.Get(), kAssets);
    r.resize(W, H);

    PreviewScene empty;
    r.set_scene(empty);
    r.render(1000.0);
    std::vector<uint8_t> bare = warp::read_pixels(dev.Get(), ctx.Get(), r.texture_srv(), W, H);

    PreviewScene with_notes;
    with_notes.has_notes = true;
    with_notes.notes.push_back(note(1100.0, PreviewLane::Kick));
    with_notes.notes.push_back(note(1100.0, PreviewLane::Red));
    with_notes.notes.push_back(note(1300.0, PreviewLane::Yellow, true));
    with_notes.song_length_ms = 1300.0;
    r.set_scene(with_notes);
    r.render(1000.0);
    std::vector<uint8_t> gems = warp::read_pixels(dev.Get(), ctx.Get(), r.texture_srv(), W, H);

    // The kick is drawn at its own centre: that pixel is not the bare
    // highway's.
    const PixelAt kick = gem_pixel(r.config(), with_notes, 1000.0, TextureId::LongKick, W, H);
    const uint8_t* kick_now = warp::pixel(gems, W, kick.x, kick.y);
    const uint8_t* kick_bare = warp::pixel(bare, W, kick.x, kick.y);
    CHECK_FALSE((kick_now[0] == kick_bare[0] && kick_now[1] == kick_bare[1] && kick_now[2] == kick_bare[2]));

    // Each pad gem is drawn in its own lane and colour: red leads at the red
    // gem's centre, and at the yellow cymbal's red and green both beat blue.
    const uint8_t* red = gem_centre(r.config(), with_notes, 1000.0, TextureId::BoxRed, gems, W, H);
    CHECK(red[0] > red[1]);
    CHECK(red[0] > red[2]);
    const uint8_t* yellow =
        gem_centre(r.config(), with_notes, 1000.0, TextureId::CymbalYellow, gems, W, H);
    CHECK(yellow[0] > yellow[2]);
    CHECK(yellow[1] > yellow[2]);
}

TEST_CASE("PreviewRenderer: SP phrase energy gems and active SP floor change pixels (WARP)") {
    ComPtr<ID3D11Device> dev;
    ComPtr<ID3D11DeviceContext> ctx;
    REQUIRE(warp::make_device(dev, ctx));

    const int W = 160, H = 120;
    PreviewRenderer r(dev.Get(), ctx.Get(), kAssets);
    r.resize(W, H);

    PreviewScene plain;
    plain.has_notes = true;
    plain.notes.push_back(note(1200.0, PreviewLane::Green));
    r.set_scene(plain);
    r.render(1000.0);
    std::vector<uint8_t> a = warp::read_pixels(dev.Get(), ctx.Get(), r.texture_srv(), W, H);

    PreviewScene lit = plain;
    // Spans end half a tick past their last note through the song's timing,
    // so a scene with a phrase carries one: timed_scene()'s, which matches
    // note() and span().
    lit.timing = timed_scene().timing;
    lit.sp_phrases.push_back(span(1100.0, 1200.0));
    hydra::app::PreviewActivation act;
    act.ms = 900.0;
    act.has_sp_end = true;
    act.sp_end_ms = 2500.0;
    lit.activations.push_back(act);
    r.set_scene(lit);
    r.render(1000.0);
    std::vector<uint8_t> b = warp::read_pixels(dev.Get(), ctx.Get(), r.texture_srv(), W, H);

    // The floor by the strike line is now greener than red (energy tint).
    const uint8_t* floor = floor_pixel(r.config(), b, W, H);
    CHECK(floor[1] > floor[0]);

    // The plain green gem is green at its lane centre. Inside the phrase the
    // same gem is drawn in the paler energy texture, so its red and blue rise.
    const uint8_t* green = gem_centre(r.config(), plain, 1000.0, TextureId::BoxGreen, a, W, H);
    CHECK(green[1] > green[0]);
    CHECK(green[1] > green[2]);
    const uint8_t* energy = gem_centre(r.config(), lit, 1000.0, TextureId::BoxEnergy, b, W, H);
    CHECK(energy[0] > green[0]);
    CHECK(energy[2] > green[2]);
}

// The Preview builds the timeline on a worker and hands it over ready-made.
// That overload must draw exactly what the building one draws, pixel for
// pixel, in both the pro (cymbal) and 4-lane looks.
TEST_CASE("PreviewRenderer: a prebuilt track state draws the same pixels (WARP)") {
    ComPtr<ID3D11Device> dev;
    ComPtr<ID3D11DeviceContext> ctx;
    REQUIRE(warp::make_device(dev, ctx));

    const int W = 160, H = 120;
    PreviewScene scene;
    scene.has_notes = true;
    scene.notes.push_back(note(1100.0, PreviewLane::Kick));
    scene.notes.push_back(note(1150.0, PreviewLane::Yellow, true));
    scene.notes.push_back(note(1200.0, PreviewLane::Green));
    scene.timing = timed_scene().timing;
    scene.sp_phrases.push_back(span(1100.0, 1200.0));
    hydra::app::PreviewActivation act;
    act.ms = 900.0;
    act.has_sp_end = true;
    act.sp_end_ms = 2500.0;
    scene.activations.push_back(act);
    scene.song_length_ms = 2500.0;

    for (bool pro : {true, false}) {
        CAPTURE(pro);
        const TrackStateOptions opts = hydra::ui::track_options(pro);

        PreviewRenderer built(dev.Get(), ctx.Get(), kAssets);
        built.resize(W, H);
        built.set_scene(scene, opts);
        built.render(1000.0);
        std::vector<uint8_t> a =
            warp::read_pixels(dev.Get(), ctx.Get(), built.texture_srv(), W, H);

        PreviewRenderer prebuilt(dev.Get(), ctx.Get(), kAssets);
        prebuilt.resize(W, H);
        prebuilt.set_scene(scene, build_track_state(scene, opts));
        prebuilt.render(1000.0);
        std::vector<uint8_t> b =
            warp::read_pixels(dev.Get(), ctx.Get(), prebuilt.texture_srv(), W, H);

        REQUIRE(a.size() == b.size());
        CHECK(a == b);
    }
}

TEST_CASE("PreviewRenderer: resize and a tall target keep the track at the bottom (WARP)") {
    ComPtr<ID3D11Device> dev;
    ComPtr<ID3D11DeviceContext> ctx;
    REQUIRE(warp::make_device(dev, ctx));

    PreviewRenderer r(dev.Get(), ctx.Get(), kAssets);
    r.resize(128, 128);
    r.resize(100, 300);  // taller than the track is: the top is background
    CHECK(r.width() == 100);
    CHECK(r.height() == 300);
    PreviewScene scene;
    r.set_scene(scene);
    r.render(0.0);
    std::vector<uint8_t> img = warp::read_pixels(dev.Get(), ctx.Get(), r.texture_srv(), 100, 300);
    CHECK(img.size() == static_cast<size_t>(100) * 300 * 4);
    // The track rectangle hugs the bottom edge: it starts below the top row,
    // everything above the horizon fade's end is background, and the floor
    // by the strike line is drawn.
    CHECK(track_rect(r.config(), 100, 300).top > 0);
    check_past_fade_is_background(r.config(), img, 100, 300);
    CHECK_FALSE(is_background(r.config(), floor_pixel(r.config(), img, 100, 300)));
}

// D95 call 4: the closed Preview frees its targets, and the next resize draws
// exactly the pixels it drew before.
TEST_CASE("PreviewRenderer: released targets are freed and come back drawing the same (WARP)") {
    ComPtr<ID3D11Device> dev;
    ComPtr<ID3D11DeviceContext> ctx;
    REQUIRE(warp::make_device(dev, ctx));

    const int W = 160, H = 120;
    PreviewRenderer r(dev.Get(), ctx.Get(), kAssets);
    r.resize(W, H);
    PreviewScene scene;
    scene.has_notes = true;
    scene.notes.push_back(note(1100.0, PreviewLane::Kick));
    scene.notes.push_back(note(1300.0, PreviewLane::Yellow, true));
    scene.song_length_ms = 1300.0;
    r.set_scene(scene);
    r.render(1000.0);
    const std::vector<uint8_t> before =
        warp::read_pixels(dev.Get(), ctx.Get(), r.texture_srv(), W, H);

    // Hold the shown texture: once released, this is its only reference.
    ComPtr<ID3D11Resource> shown;
    r.texture_srv()->GetResource(&shown);
    r.release_targets();
    shown->AddRef();
    CHECK(shown->Release() == 1);  // Release returns the count left
    CHECK(r.texture_srv() == nullptr);
    CHECK(r.width() == 0);
    CHECK(r.height() == 0);
    r.render(1000.0);  // nothing to draw into: a no-op, not a crash

    r.resize(W, H);
    r.render(1000.0);
    REQUIRE(r.texture_srv() != nullptr);
    CHECK(warp::read_pixels(dev.Get(), ctx.Get(), r.texture_srv(), W, H) == before);
}

TEST_CASE("PreviewRenderer: a missing asset dir is a clear error") {
    ComPtr<ID3D11Device> dev;
    ComPtr<ID3D11DeviceContext> ctx;
    REQUIRE(warp::make_device(dev, ctx));
    CHECK_THROWS_AS(PreviewRenderer(dev.Get(), ctx.Get(), "C:\\no\\such\\dir"), std::runtime_error);
    try {
        PreviewRenderer(dev.Get(), ctx.Get(), "C:\\no\\such\\dir");
        FAIL("a missing asset dir made a renderer");
    } catch (const std::exception& e) {
        CHECK(hydra::app::plain_error(e) ==
              "Some of Hydra's Preview files are missing. Reinstall Hydra to restore them.");
    }
}

// The memory audit's one real leak (D95; docs/handoffs/2026-10-07-memory-audit.md (removed; git show 444631e6:docs/handoffs/2026-10-07-memory-audit.md)):
// a constructor that throws after building shaders, buffers and meshes must
// release them. Every D3D object it made holds a reference on the device, so
// the device's reference count shows anything left behind.
TEST_CASE("PreviewRenderer: setup that fails part-way releases what it made (WARP)") {
    namespace fs = std::filesystem;
    ComPtr<ID3D11Device> dev;
    ComPtr<ID3D11DeviceContext> ctx;
    REQUIRE(warp::make_device(dev, ctx));

    // A copy of the shipped assets with the textures gone: the constructor
    // gets through everything else before the first texture throws.
    const std::string dir_utf8 = testtemp::temp_dir("preview_no_textures");
    const fs::path dir = hydra::os_path(dir_utf8);
    fs::remove_all(dir);
    fs::copy(fs::path(kAssets), dir, fs::copy_options::recursive);
    fs::remove_all(dir / "textures");

    auto device_refs = [&] {
        dev->AddRef();
        return dev->Release();
    };
    const ULONG before = device_refs();
    CHECK_THROWS_AS(PreviewRenderer(dev.Get(), ctx.Get(), dir_utf8), std::runtime_error);
    CHECK(device_refs() == before);
    fs::remove_all(dir);
}

// Hydra runs from wherever the user installed it, so the assets can sit under
// a folder like C:\Users\Zoë\... The other non-ASCII folder readers are in
// test_utf8_paths.cpp.
TEST_CASE("the Preview renderer loads its assets from a non-ASCII folder (WARP)") {
    namespace fs = std::filesystem;
    ComPtr<ID3D11Device> dev;
    ComPtr<ID3D11DeviceContext> ctx;
    REQUIRE(warp::make_device(dev, ctx));
    const std::string dir = testtemp::non_ascii_dir("assets");
    fs::copy(fs::u8path(kAssets), hydra::os_path(dir),
             fs::copy_options::recursive | fs::copy_options::overwrite_existing);

    // An unread config throws "PreviewRenderer: missing ...\3d-config.json".
    PreviewRenderer r(dev.Get(), ctx.Get(), dir);
    CHECK(r.config().hydra.msaa == 4);
    testtemp::remove_dir(dir);
}
