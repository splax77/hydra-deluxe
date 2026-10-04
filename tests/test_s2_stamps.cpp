// Step 2's stamp moves, pinned. Every stamp on stored data lives in
// src/store/stored_versions.h (ADR 0018). Step 2 changed what the parser
// counts (D19 disco, D20 2x kicks, D24 the dynamics tag), so saved Dynamics
// counts from before it must read as missing; T5 added two fields to the
// Dynamics blob, so an old blob must not decode; and D23 says the rule for
// bumping the results stamp names the chart readers.

#include "doctest.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "app/dynamics_breakdown.h"
#include "store/stored_versions.h"

#ifndef HYDRA_SOURCE_DIR
#error "HYDRA_SOURCE_DIR must be defined (see CMakeLists.txt)"
#endif

using namespace hydra;

TEST_CASE("step 2: Dynamics counts saved before step 2 read as missing") {
    CHECK(store::kDynamicsCountStamp.written == 2);
    CHECK_FALSE(store::kDynamicsCountStamp.is_current(1));
    CHECK_FALSE(store::kDynamicsCountStamp.is_current(0));
}

TEST_CASE("step 2: a Dynamics blob in the old layout does not decode") {
    CHECK(store::kDynamicsBlobStamp.written == 2);
    std::vector<uint8_t> blob = app::encode_dynamics(app::DynamicsBreakdown{});
    REQUIRE(app::decode_dynamics(blob).has_value());
    blob[0] = 1;  // the layout before T5's two fields
    CHECK_FALSE(app::decode_dynamics(blob).has_value());
}

// D23: a change under src/parse that alters what a chart reads as must bump
// the results stamp. The rule is a comment, so this reads the comment: the
// block between the "Results" banner and the kResultsStamp line must name
// src/parse, next to src/search and src/core.
TEST_CASE("step 2: the results stamp's bump rule names the chart readers") {
    namespace fs = std::filesystem;
    const fs::path file =
        fs::u8path(HYDRA_SOURCE_DIR) / "src" / "store" / "stored_versions.h";
    std::ifstream in(file);
    REQUIRE(in.good());
    std::stringstream ss;
    ss << in.rdbuf();
    const std::string text = ss.str();

    const size_t stamp = text.find("kResultsStamp{");
    REQUIRE(stamp != std::string::npos);
    const size_t banner = text.rfind("// ---- Results", stamp);
    REQUIRE(banner != std::string::npos);
    const std::string rule = text.substr(banner, stamp - banner);
    CHECK(rule.find("src/search") != std::string::npos);
    CHECK(rule.find("src/core") != std::string::npos);
    CHECK(rule.find("src/parse") != std::string::npos);
}
