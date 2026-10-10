// The golden-image test: Hydra's Preview is a faithful port of Onyx's 3D drum
// previewer, so a Hydra frame of a chart must match a screenshot of Onyx
// showing the same chart at the same time.
//
// Fixture (testdata/preview/golden.json, captured by hand from Onyx — see
// docs/adr/0008 for the procedure):
//   { "chart": "notes.mid", "time_ms": 32000, "width": 900, "height": 800,
//     "pro": true, "bass2x": true, "tolerance": 20,
//     "mask": [[x, y, w, h], ...] }   // rectangles to ignore (Onyx's text)
// plus golden_onyx.png next to it. Without the fixture the test reports that
// it skipped, so the suite passes before the capture exists.
//
// Dev aid: set HYDRA_PREVIEW_DUMP="<chart path>|<time ms>|<out.bmp>" to
// render any chart at any time to a BMP (the failing golden comparison also
// writes build-cpp/preview_actual.bmp).

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/preview_view.h"
#include "bytes_util.h"
#include "image/decode.h"
#include "json.hpp"
#include "parse/song.h"
#include "core/winstr.h"
#include "env_util.h"
#include "render/preview_renderer.h"
#include "render/track_state.h"  // note_in_span
#include "ui/preview_load_job.h"  // track_options
#include "warp_util.h"

#ifndef HYDRA_ASSET_DIR
#error "HYDRA_ASSET_DIR must be defined (see CMakeLists.txt)"
#endif
#ifndef HYDRA_TESTDATA_DIR
#error "HYDRA_TESTDATA_DIR must be defined (see CMakeLists.txt)"
#endif

using namespace hydra;
using namespace hydra::render;
using Microsoft::WRL::ComPtr;

namespace {

const std::string kFixtureDir = std::string(HYDRA_TESTDATA_DIR) + "/preview";

// Write tightly-packed RGBA (top row first) as a 24-bit BMP.
void write_bmp(const std::string& path, const std::vector<uint8_t>& rgba, int w, int h) {
    std::ofstream f(path, std::ios::binary);
    if (!f) return;
    const int row = (w * 3 + 3) & ~3;
    const uint32_t data = static_cast<uint32_t>(row) * h;
    const uint32_t size = 54 + data;
    std::vector<uint8_t> hdr = {'B', 'M'};
    testbytes::put_u32(hdr, size);
    testbytes::put_u32(hdr, 0);  // reserved
    testbytes::put_u32(hdr, 54);
    testbytes::put_u32(hdr, 40);
    testbytes::put_u32(hdr, static_cast<uint32_t>(w));
    testbytes::put_u32(hdr, static_cast<uint32_t>(h));
    testbytes::put_u16(hdr, 1);
    testbytes::put_u16(hdr, 24);
    testbytes::put_u32(hdr, 0);  // no compression
    testbytes::put_u32(hdr, data);
    hdr.resize(54, 0);  // the resolution and palette fields stay zero
    f.write(reinterpret_cast<const char*>(hdr.data()), static_cast<std::streamsize>(hdr.size()));
    std::vector<uint8_t> line(static_cast<size_t>(row), 0);
    for (int y = h - 1; y >= 0; --y) {
        for (int x = 0; x < w; ++x) {
            const uint8_t* p = &rgba[(static_cast<size_t>(y) * w + x) * 4];
            line[static_cast<size_t>(x) * 3] = p[2];
            line[static_cast<size_t>(x) * 3 + 1] = p[1];
            line[static_cast<size_t>(x) * 3 + 2] = p[0];
        }
        f.write(reinterpret_cast<const char*>(line.data()), row);
    }
}

// Render `chart` at `time_ms` into W x H with the real assets on WARP.
std::vector<uint8_t> render_chart(const std::string& chart, double time_ms, int w, int h,
                                  bool pro, bool bass2x) {
    ComPtr<ID3D11Device> dev;
    ComPtr<ID3D11DeviceContext> ctx;
    REQUIRE(warp::make_device(dev, ctx));

    // Analyze so the path overlay (active SP windows, fills) is present.
    app::AnalysisSettings settings;
    settings.depth_mode = DepthMode::Scores;
    settings.depth_value = 10;
    settings.ms_filter = 10.0;
    settings.prodrums = pro;
    settings.bass2x = bass2x;
    app::PreviewScene scene;
    try {
        app::AnalysisResult r = app::analyze_chart_file(chart, settings);
        const Path* best = r.record.paths.empty() ? nullptr : &r.record.best_path();
        scene = app::build_preview_scene(r.song, best);
    } catch (const std::exception&) {
        scene = app::build_preview_scene(load_songpath(chart, pro, bass2x), nullptr);
    }

    PreviewRenderer r(dev.Get(), ctx.Get(), HYDRA_ASSET_DIR);
    r.resize(w, h);
    r.set_scene(scene, hydra::ui::track_options(pro));
    r.render(time_ms);
    return warp::read_pixels(dev.Get(), ctx.Get(), r.texture_srv(), w, h);
}

// Box-filter downsample by 2 (RGB only).
std::vector<float> half_res(const std::vector<uint8_t>& rgba, int w, int h, int& ow, int& oh) {
    ow = w / 2;
    oh = h / 2;
    std::vector<float> out(static_cast<size_t>(ow) * oh * 3);
    for (int y = 0; y < oh; ++y)
        for (int x = 0; x < ow; ++x)
            for (int c = 0; c < 3; ++c) {
                float s = 0.0f;
                for (int dy = 0; dy < 2; ++dy)
                    for (int dx = 0; dx < 2; ++dx)
                        s += rgba[((static_cast<size_t>(y) * 2 + dy) * w + (x * 2 + dx)) * 4 + c];
                out[(static_cast<size_t>(y) * ow + x) * 3 + c] = s / 4.0f;
            }
    return out;
}

// Each half-res pixel's largest channel delta between `a` and `g`, or -1 for a
// pixel the caller masks out. `masked` takes half-res coordinates.
template <typename Masked>
std::vector<float> max_channel_deltas(const std::vector<float>& a, const std::vector<float>& g, int ow,
                                      int oh, Masked masked) {
    std::vector<float> out(static_cast<size_t>(ow) * oh, -1.0f);
    for (int y = 0; y < oh; ++y)
        for (int x = 0; x < ow; ++x) {
            if (masked(x, y)) continue;
            const size_t i = static_cast<size_t>(y) * ow + x;
            float m = 0.0f;
            for (int c = 0; c < 3; ++c) m = (std::max)(m,std::fabs(a[i * 3 + c] - g[i * 3 + c]));
            out[i] = m;
        }
    return out;
}

constexpr int kTile = 16;  // tile side in half-res pixels (32 at full size), from the plan's Task 2

// For one per-pixel delta D: the share of unmasked pixels whose delta is over D,
// in the worst kTile by kTile tile and in the whole frame. Edge tiles are
// partial; a tile's share is over its own unmasked pixels.
struct OverShare {
    double worst_tile_percent = 0.0;
    int worst_tx = -1, worst_ty = -1;  // tile column and row
    double frame_percent = 0.0;
};

// One tile's counts: unmasked pixels, and those over D.
struct TileCount {
    size_t unmasked = 0, over = 0;
};

std::vector<TileCount> tile_counts(const std::vector<float>& delta, int ow, int oh, float d, int& tw,
                                   int& th) {
    tw = (ow + kTile - 1) / kTile;
    th = (oh + kTile - 1) / kTile;
    std::vector<TileCount> t(static_cast<size_t>(tw) * th);
    for (int y = 0; y < oh; ++y)
        for (int x = 0; x < ow; ++x) {
            const float v = delta[static_cast<size_t>(y) * ow + x];
            if (v < 0.0f) continue;
            TileCount& c = t[static_cast<size_t>(y / kTile) * tw + x / kTile];
            ++c.unmasked;
            if (v > d) ++c.over;
        }
    return t;
}

OverShare over_share(const std::vector<float>& delta, int ow, int oh, float d) {
    int tw, th;
    const std::vector<TileCount> t = tile_counts(delta, ow, oh, d, tw, th);
    OverShare s;
    size_t unmasked = 0, over = 0;
    for (int ty = 0; ty < th; ++ty)
        for (int tx = 0; tx < tw; ++tx) {
            const TileCount& c = t[static_cast<size_t>(ty) * tw + tx];
            unmasked += c.unmasked;
            over += c.over;
            if (c.unmasked == 0) continue;
            const double p = 100.0 * static_cast<double>(c.over) / static_cast<double>(c.unmasked);
            if (p > s.worst_tile_percent || s.worst_tx < 0) {
                s.worst_tile_percent = p;
                s.worst_tx = tx;
                s.worst_ty = ty;
            }
        }
    s.frame_percent = unmasked ? 100.0 * static_cast<double>(over) / static_cast<double>(unmasked) : 0.0;
    return s;
}

// The delta at percentile `q` (0..100) of the unmasked pixels.
float delta_percentile(const std::vector<float>& delta, double q) {
    std::vector<float> v;
    for (float x : delta)
        if (x >= 0.0f) v.push_back(x);
    if (v.empty()) return 0.0f;
    const size_t k = (std::min)(v.size() - 1, static_cast<size_t>(q / 100.0 * static_cast<double>(v.size())));
    std::nth_element(v.begin(), v.begin() + static_cast<std::ptrdiff_t>(k), v.end());
    return v[k];
}

// The D values the measurement reports (named in the tf-t2 brief).
constexpr float kMeasureD[] = {16.0f, 24.0f, 32.0f, 48.0f, 64.0f};

// Dev aid: the per-tile map (one over-D percent column per kMeasureD value) and
// the histogram of per-pixel deltas (integer bins), as CSV in the working folder.
void write_delta_csvs(const std::vector<float>& delta, int ow, int oh) {
    std::ofstream tiles("preview_tiles.csv");
    tiles << "tile_x,tile_y,half_x,half_y,unmasked";
    for (float d : kMeasureD) tiles << ",pct_over_" << static_cast<int>(d);
    tiles << "\n";
    std::vector<std::vector<TileCount>> per_d;
    int tw = 0, th = 0;
    for (float d : kMeasureD) per_d.push_back(tile_counts(delta, ow, oh, d, tw, th));
    for (int ty = 0; ty < th; ++ty)
        for (int tx = 0; tx < tw; ++tx) {
            const size_t i = static_cast<size_t>(ty) * tw + tx;
            const size_t unmasked = per_d[0][i].unmasked;
            tiles << tx << "," << ty << "," << tx * kTile << "," << ty * kTile << "," << unmasked;
            for (const auto& t : per_d)
                tiles << ","
                      << (unmasked ? 100.0 * static_cast<double>(t[i].over) / static_cast<double>(unmasked)
                                   : 0.0);
            tiles << "\n";
        }
    std::vector<size_t> hist(256, 0);
    for (float x : delta)
        if (x >= 0.0f) ++hist[(std::min<size_t>)(255, static_cast<size_t>(x))];
    std::ofstream h("preview_delta_hist.csv");
    h << "delta,pixels\n";
    for (size_t i = 0; i < hist.size(); ++i) h << i << "," << hist[i] << "\n";
}

}  // namespace

TEST_CASE("preview golden: a Hydra frame matches the Onyx screenshot") {
    const std::string spec_path = kFixtureDir + "/golden.json";
    if (!file_exists_utf8(spec_path)) {
        MESSAGE("golden fixture absent (" << spec_path << "): skipped");
        return;
    }
    std::string spec_text = read_file_text(spec_path);
    nlohmann::json spec = nlohmann::json::parse(spec_text);
    const std::string chart = kFixtureDir + "/" + spec.value("chart", "notes.mid");
    double time_ms = spec.value("time_ms", 0.0);
    if (const auto t = read_env("HYDRA_PREVIEW_GOLDEN_TIME")) time_ms = std::atof(t->c_str());  // dev aid
    const int w = spec.value("width", 0), h = spec.value("height", 0);
    const bool pro = spec.value("pro", true), bass2x = spec.value("bass2x", true);
    const double tolerance = spec.value("tolerance", 20.0);
    REQUIRE(w > 0);
    REQUIRE(h > 0);

    std::vector<uint8_t> golden_bytes = read_file_bytes(kFixtureDir + "/golden_onyx.png");
    REQUIRE(!golden_bytes.empty());
    image::DecodedImage golden = image::decode_image(golden_bytes);
    // The screenshot may be the whole Onyx window; "crop": [x, y, w, h]
    // names the highway area inside it (the part Hydra renders).
    if (spec.contains("crop")) {
        std::vector<int> c = spec["crop"].get<std::vector<int>>();
        REQUIRE(c.size() == 4);
        REQUIRE(c[2] == w);
        REQUIRE(c[3] == h);
        REQUIRE(c[0] + w <= golden.width);
        REQUIRE(c[1] + h <= golden.height);
        std::vector<uint8_t> cropped(static_cast<size_t>(w) * h * 4);
        for (int y = 0; y < h; ++y)
            std::memcpy(&cropped[static_cast<size_t>(y) * w * 4],
                        &golden.rgba[((static_cast<size_t>(c[1]) + y) * golden.width + c[0]) * 4],
                        static_cast<size_t>(w) * 4);
        golden.rgba.swap(cropped);
        golden.width = w;
        golden.height = h;
    }
    REQUIRE(golden.width == w);
    REQUIRE(golden.height == h);

    std::vector<uint8_t> actual = render_chart(chart, time_ms, w, h, pro, bass2x);
    const bool dump = read_env("HYDRA_PREVIEW_GOLDEN_DUMP").has_value();
    if (dump) {
        write_bmp("preview_actual.bmp", actual, w, h);
        write_bmp("preview_golden.bmp", golden.rgba, w, h);
    }

    // Masks (in full-res pixels) cover Onyx's on-screen text.
    std::vector<std::vector<int>> masks;
    if (spec.contains("mask"))
        for (const auto& m : spec["mask"]) masks.push_back(m.get<std::vector<int>>());
    auto masked = [&](int x, int y) {
        for (const auto& m : masks)
            if (m.size() == 4 && x >= m[0] && x < m[0] + m[2] && y >= m[1] && y < m[1] + m[3]) return true;
        return false;
    };

    int ow, oh;
    std::vector<float> a = half_res(actual, w, h, ow, oh);
    std::vector<float> g = half_res(golden.rgba, w, h, ow, oh);
    double err = 0.0;
    size_t n = 0;
    for (int y = 0; y < oh; ++y)
        for (int x = 0; x < ow; ++x) {
            if (masked(x * 2, y * 2)) continue;
            for (int c = 0; c < 3; ++c) {
                err += std::fabs(a[(static_cast<size_t>(y) * ow + x) * 3 + c] -
                                 g[(static_cast<size_t>(y) * ow + x) * 3 + c]);
                ++n;
            }
        }
    const double mae = n ? err / static_cast<double>(n) : 0.0;
    MESSAGE("golden mean abs error: " << mae << " / 255 (tolerance " << tolerance << ")");

    // The per-pixel delta and tile measurement: printed, not checked yet.
    const std::vector<float> delta =
        max_channel_deltas(a, g, ow, oh, [&](int x, int y) { return masked(x * 2, y * 2); });
    MESSAGE("max-channel delta percentiles: p50 " << delta_percentile(delta, 50.0) << ", p90 "
                                                  << delta_percentile(delta, 90.0) << ", p99 "
                                                  << delta_percentile(delta, 99.0) << ", p99.9 "
                                                  << delta_percentile(delta, 99.9));
    for (float d : kMeasureD) {
        const OverShare s = over_share(delta, ow, oh, d);
        MESSAGE("D " << d << ": worst tile " << s.worst_tile_percent << "% at tile (" << s.worst_tx << ", "
                     << s.worst_ty << ") = half-res (" << s.worst_tx * kTile << ", " << s.worst_ty * kTile
                     << "); whole frame " << s.frame_percent << "%");
    }
    if (dump) write_delta_csvs(delta, ow, oh);
    if (mae >= tolerance) write_bmp("preview_actual.bmp", actual, w, h);
    CHECK(mae < tolerance);
}

TEST_CASE("preview dump (dev aid, HYDRA_PREVIEW_DUMP)") {
    const std::optional<std::string> env = read_env("HYDRA_PREVIEW_DUMP");
    if (!env) return;
    const std::string& spec = *env;
    size_t a = spec.find('|'), b = spec.rfind('|');
    REQUIRE(a != std::string::npos);
    REQUIRE(b != a);
    const std::string chart = spec.substr(0, a);
    const double time_ms = std::atof(spec.substr(a + 1, b - a - 1).c_str());
    const std::string out = spec.substr(b + 1);
    const int w = 900, h = 800;
    const bool bass2x = !read_env("HYDRA_PREVIEW_DUMP_NO2X");
    std::vector<uint8_t> px = render_chart(chart, time_ms, w, h, true, bass2x);
    write_bmp(out, px, w, h);
    MESSAGE("wrote " << out);
    // Where Hydra thinks that time is, for lining up with Onyx's time box.
    app::PreviewScene scene = app::build_preview_scene(load_songpath(chart, true, bass2x), nullptr);
    app::PreviewTimeBox box = app::build_time_box(scene, time_ms, app::last_note_ms(scene));
    MESSAGE("time box: " << box.timestamp << " | " << box.position << " of " << box.length
                         << " | " << box.tempo << " | " << box.section_line);
    for (size_t i = 0; i < scene.tempos.size() && i < 6; ++i)
        MESSAGE("tempo[" << i << "] tick " << scene.tempos[i].tick << " ms " << scene.tempos[i].ms
                         << " bpm " << scene.tempos[i].bpm);
    MESSAGE("first note ms " << (scene.notes.empty() ? -1.0 : scene.notes.front().ms)
                             << " tick " << (scene.notes.empty() ? -1 : scene.notes.front().tick));
    // The notes in the visible window, for lining up against an Onyx frame.
    for (const app::PreviewNote& n : scene.notes) {
        if (n.ms < time_ms - 500.0 || n.ms > time_ms + 1500.0) continue;
        bool in_sp = false;
        for (const app::PreviewSpan& s : scene.sp_phrases)
            if (render::note_in_span(scene, s, n)) in_sp = true;
        std::string flags;
        if (n.cymbal) flags += " cymbal";
        if (n.ghost) flags += " ghost";
        if (n.accent) flags += " accent";
        if (in_sp) flags += " SP";
        MESSAGE("note +" << (n.ms - time_ms) << " ms tick " << n.tick << " lane "
                         << static_cast<int>(n.lane) << flags);
    }
    for (const app::PreviewSpan& s : scene.sp_phrases)
        if (s.end_ms >= time_ms - 2000.0 && s.start_ms <= time_ms + 3000.0)
            MESSAGE("sp phrase " << s.start_ms << " .. " << s.end_ms << " ms");
    std::ofstream tcsv(out + ".tempos.csv");
    for (const app::PreviewTempo& t : scene.tempos) tcsv << t.ms << "," << t.tick << "," << t.bpm << "\n";
    // Every note as CSV (ms,tick,lane,cymbal,sp) beside the image.
    std::ofstream csv(out + ".csv");
    for (const app::PreviewNote& n : scene.notes) {
        bool in_sp = false;
        for (const app::PreviewSpan& s : scene.sp_phrases)
            if (render::note_in_span(scene, s, n)) in_sp = true;
        csv << n.ms << "," << n.tick << "," << static_cast<int>(n.lane) << "," << (n.cymbal ? 1 : 0)
            << "," << (in_sp ? 1 : 0) << "\n";
    }
}
