// Step 2's stamp moves, pinned. Every stamp on stored data lives in
// src/store/stored_versions.h (ADR 0018). Step 2 changed what the parser
// counts (D19 disco, D20 2x kicks, D24 the dynamics tag), so saved Dynamics
// counts from before it must read as missing; T5 added two fields to the
// Dynamics blob, so an old blob must not decode; and D23 says the rule for
// bumping the results stamp names the chart readers.

#include "doctest.h"

#include <cstdint>
#include <vector>

#include "app/dynamics_breakdown.h"
#include "store/stored_versions.h"

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

// D23 (the results stamp's bump rule names the chart readers) is a comment, so
// it is checked by reading the source, in test_single_owner.cpp, the one test
// that reads the tree.
