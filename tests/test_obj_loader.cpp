// Tests for render/obj_loader: Onyx-compatible .obj parsing (fan triangulation,
// far-first sort), the built-in shapes, and the real copied drum models.

#include "doctest.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "app/user_messages.h"
#include "core/error_kind.h"
#include "core/winstr.h"
#include "render/obj_loader.h"

#ifndef HYDRA_ASSET_DIR
#error "HYDRA_ASSET_DIR must be defined (see CMakeLists.txt)"
#endif

using namespace hydra::render;

namespace {

// Onyx's drum-kick.obj, verbatim (4 quads, per-face normals, s off).
const char kKickObj[] =
    "g default\n"
    "v 0.5 0.01 0.015\nv 0.5 0.01 -0.015\nv 0.5 0.0 0.015\nv 0.5 0.0 -0.015\n"
    "v -0.5 0.01 0.015\nv -0.5 0.01 -0.015\nv -0.5 0.0 0.015\nv -0.5 0.0 -0.015\n"
    "vt 0.0 1.0\nvt 1.0 1.0\nvt 0.0 0.5\nvt 1.0 0.5\nvt 0.0 0.5\n"
    "vt 0.0 0.0\nvt 0.0 0.0\nvt 1.0 0.0\nvt 1.0 0.5\nvt 1.0 0.0\n"
    "vn  0.0 1.0 0.0\nvn -1.0 0.0 0.0\nvn  0.0 0.0 1.0\nvn  1.0 0.0 0.0\n"
    "s off\ng box_mesh\n"
    "f 6/1/1 5/3/1 1/4/1 2/2/1\n"
    "f 6/5/2 8/6/2 7/7/2 5/3/2\n"
    "f 5/3/3 7/7/3 3/8/3 1/4/3\n"
    "f 1/4/4 3/8/4 4/10/4 2/9/4\n";

float zsum(const ObjMesh& m, size_t tri) {
    return m.vertices[tri * 3].pos[2] + m.vertices[tri * 3 + 1].pos[2] +
           m.vertices[tri * 3 + 2].pos[2];
}

// Each triangle's three corners (position, normal, uv), sorted: the mesh's
// triangles as a set, whatever order they are in.
std::vector<std::array<float, 24>> triangle_keys(const ObjMesh& m) {
    std::vector<std::array<float, 24>> keys;
    for (size_t t = 0; t < m.triangle_count(); ++t) {
        std::array<float, 24> key{};
        size_t at = 0;
        for (size_t k = 0; k < 3; ++k) {
            const ObjVertex& v = m.vertices[t * 3 + k];
            for (float f : v.pos) key[at++] = f;
            for (float f : v.normal) key[at++] = f;
            for (float f : v.uv) key[at++] = f;
        }
        keys.push_back(key);
    }
    std::sort(keys.begin(), keys.end());
    return keys;
}

}  // namespace

TEST_CASE("load_obj fan-triangulates quads and keeps per-corner attributes") {
    ObjMesh m = load_obj(kKickObj);
    CHECK(m.triangle_count() == 8);
    CHECK(m.vertices.size() == 24);
    // Every vertex carries a unit normal and a uv inside [0,1].
    for (const ObjVertex& v : m.vertices) {
        float len = std::sqrt(v.normal[0] * v.normal[0] + v.normal[1] * v.normal[1] +
                              v.normal[2] * v.normal[2]);
        CHECK(len == doctest::Approx(1.0f));
        CHECK(v.uv[0] >= 0.0f);
        CHECK(v.uv[0] <= 1.0f);
        CHECK(v.uv[1] >= 0.0f);
        CHECK(v.uv[1] <= 1.0f);
    }
    float mn[3], mx[3];
    mesh_bounds(m, mn, mx);
    CHECK(mn[0] == doctest::Approx(-0.5f));
    CHECK(mx[0] == doctest::Approx(0.5f));
    CHECK(mn[1] == doctest::Approx(0.0f));
    CHECK(mx[1] == doctest::Approx(0.01f));
    CHECK(mn[2] == doctest::Approx(-0.015f));
    CHECK(mx[2] == doctest::Approx(0.015f));
}

TEST_CASE("load_obj sorts triangles far-first (ascending z sum)") {
    ObjMesh m = load_obj(kKickObj);
    for (size_t i = 1; i < m.triangle_count(); ++i) CHECK(zsum(m, i - 1) <= zsum(m, i));
    // The sort only reorders: the loaded triangles, put in reverse order and
    // sorted again, are the same triangles, none lost, none made up.
    ObjMesh reversed;
    for (size_t t = m.triangle_count(); t-- > 0;)
        for (size_t k = 0; k < 3; ++k) reversed.vertices.push_back(m.vertices[t * 3 + k]);
    sort_far_first(reversed);
    REQUIRE(reversed.triangle_count() == m.triangle_count());
    for (size_t i = 1; i < reversed.triangle_count(); ++i)
        CHECK(zsum(reversed, i - 1) <= zsum(reversed, i));
    CHECK(triangle_keys(reversed) == triangle_keys(m));
}

TEST_CASE("load_obj handles v//vn, bare v, negative indices, and CRLF") {
    const char* text =
        "v 0 0 0\r\nv 1 0 0\r\nv 0 1 0\r\nvn 0 0 1\r\n"
        "f 1//1 2//1 3//1\r\n"
        "f -3 -2 -1\r\n";
    ObjMesh m = load_obj(text);
    CHECK(m.triangle_count() == 2);
    CHECK(m.vertices[0].normal[2] == doctest::Approx(1.0f));
    CHECK(m.vertices[3].normal[2] == doctest::Approx(0.0f));  // no vn on face 2
    CHECK(m.vertices[4].pos[0] == doctest::Approx(1.0f));
}

TEST_CASE("an .obj with no faces reads as a Preview asset problem") {
    try {
        load_obj("v 0 0 0\nv 1 0 0\nv 0 1 0\n");
        FAIL("an .obj with no faces loaded");
    } catch (const std::exception& e) {
        CHECK(hydra::app::plain_error(e) ==
              "Some of Hydra's Preview files are missing. Reinstall Hydra to restore them.");
        CHECK(std::string(e.what()) == "obj: no faces");
    }
}

TEST_CASE("load_obj rejects a face index past the vertex list as a Preview asset problem") {
    std::optional<hydra::ErrorKind> kind;
    try {
        load_obj("v 0 0 0\nf 1 2 3\n");
    } catch (const hydra::KindedError& e) {
        kind = e.kind();
    }
    REQUIRE_MESSAGE(kind.has_value(), "the bad index did not throw a KindedError");
    CHECK(*kind == hydra::ErrorKind::PreviewAssets);
}

TEST_CASE("make_flat_quad: unit square at y=0, +Y normal, Onyx UV layout") {
    ObjMesh m = make_flat_quad();
    CHECK(m.triangle_count() == 2);
    float mn[3], mx[3];
    mesh_bounds(m, mn, mx);
    CHECK(mn[0] == doctest::Approx(-0.5f));
    CHECK(mx[0] == doctest::Approx(0.5f));
    CHECK(mn[1] == doctest::Approx(0.0f));
    CHECK(mx[1] == doctest::Approx(0.0f));
    CHECK(mn[2] == doctest::Approx(-0.5f));
    CHECK(mx[2] == doctest::Approx(0.5f));
    for (const ObjVertex& v : m.vertices) {
        CHECK(v.normal[1] == doctest::Approx(1.0f));
        // u grows with +x, v grows with -z.
        CHECK(v.uv[0] == doctest::Approx(v.pos[0] + 0.5f));
        CHECK(v.uv[1] == doctest::Approx(0.5f - v.pos[2]));
    }
    // Counter-clockwise seen from +Y: the first triangle's normal via cross
    // product points up.
    const ObjVertex &a = m.vertices[0], &b = m.vertices[1], &c = m.vertices[2];
    float e1[3] = {b.pos[0] - a.pos[0], b.pos[1] - a.pos[1], b.pos[2] - a.pos[2]};
    float e2[3] = {c.pos[0] - a.pos[0], c.pos[1] - a.pos[1], c.pos[2] - a.pos[2]};
    float ny = e1[2] * e2[0] - e1[0] * e2[2];
    CHECK(ny > 0.0f);
}

TEST_CASE("make_box: four outward faces, no bottom or back") {
    ObjMesh m = make_box();
    CHECK(m.triangle_count() == 8);
    bool saw_top = false, saw_left = false, saw_front = false, saw_right = false;
    for (size_t t = 0; t < m.triangle_count(); ++t) {
        const ObjVertex &a = m.vertices[t * 3], &b = m.vertices[t * 3 + 1],
                        &c = m.vertices[t * 3 + 2];
        float e1[3] = {b.pos[0] - a.pos[0], b.pos[1] - a.pos[1], b.pos[2] - a.pos[2]};
        float e2[3] = {c.pos[0] - a.pos[0], c.pos[1] - a.pos[1], c.pos[2] - a.pos[2]};
        float n[3] = {e1[1] * e2[2] - e1[2] * e2[1], e1[2] * e2[0] - e1[0] * e2[2],
                      e1[0] * e2[1] - e1[1] * e2[0]};
        // The winding normal agrees with the stored normal (outward).
        float dot = n[0] * a.normal[0] + n[1] * a.normal[1] + n[2] * a.normal[2];
        CHECK(dot > 0.0f);
        if (a.normal[1] > 0.5f) saw_top = true;
        if (a.normal[0] < -0.5f) saw_left = true;
        if (a.normal[2] > 0.5f) saw_front = true;
        if (a.normal[0] > 0.5f) saw_right = true;
        CHECK(a.normal[1] > -0.5f);  // no bottom
        CHECK(a.normal[2] > -0.5f);  // no back
    }
    CHECK(saw_top);
    CHECK(saw_left);
    CHECK(saw_front);
    CHECK(saw_right);
}

namespace {

// The fewest decimals any component of a `vn` line in `obj` is written with,
// or -1 when the text has no `vn` line. It reads plain decimals ("0.6442"),
// the form the shipped models use.
int coarsest_normal_decimals(const std::string& obj) {
    std::istringstream in(obj);
    std::string line;
    int fewest = -1;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::istringstream words(line);
        std::string tag;
        if (!(words >> tag) || tag != "vn") continue;
        std::string component;
        while (words >> component) {
            const size_t dot = component.find('.');
            const int decimals = dot == std::string::npos ? 0 : static_cast<int>(component.size() - dot - 1);
            if (fewest < 0 || decimals < fewest) fewest = decimals;
        }
    }
    return fewest;
}

}  // namespace

// The three model files the Preview loads (preview_renderer.cpp's load_model
// calls): each loads whole, every corner carries a unit normal, and each has
// size along every axis.
TEST_CASE("the shipped drum models load whole with unit normals") {
    const std::string dir = std::string(HYDRA_ASSET_DIR) + "/models/";
    for (const char* file : {"drum-kick.obj", "drum-tom.obj", "drum-cymbal.obj"}) {
        const std::string name(file);
        INFO(name);
        const std::string text = hydra::read_file_text(dir + name);
        const ObjMesh mesh = load_obj(text);
        REQUIRE(mesh.triangle_count() > 0);
        CHECK(mesh.vertices.size() == mesh.triangle_count() * 3);
        // load_obj keeps each normal as the file writes it (object.hlsl
        // normalizes it again per pixel), so a normal is as unit as the
        // file's digits allow. A component written to N decimals is less than
        // one unit in its last place, 10^-N, from the value it stands for (a
        // writer that rounds to nearest gets within half that; a whole unit
        // also covers one that truncates). Three components each off by at
        // most 10^-N move the vector by at most sqrt(3) * 10^-N, so its
        // length is within that of 1. The tom and cymbal write 4 decimals:
        // 1.73e-4. The kick writes 1 ("1.0", "0.0"): 0.173.
        const int decimals = coarsest_normal_decimals(text);
        REQUIRE(decimals >= 0);
        const double tolerance = std::sqrt(3.0) * std::pow(10.0, -decimals);
        for (const ObjVertex& v : mesh.vertices) {
            const double x = v.normal[0], y = v.normal[1], z = v.normal[2];
            const double len = std::sqrt(x * x + y * y + z * z);
            const double off_unit = std::fabs(len - 1.0);
            INFO("normal length ", len, ", tolerance ", tolerance);
            CHECK(off_unit <= tolerance);
        }
        float mn[3], mx[3];
        mesh_bounds(mesh, mn, mx);
        for (int axis = 0; axis < 3; ++axis) {
            CHECK(std::isfinite(mn[axis]));
            CHECK(std::isfinite(mx[axis]));
            CHECK(mx[axis] > mn[axis]);
        }
    }
}
