// libFuzzer target for .mid files; the body lives in harness.h.
#include "harness.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    return hydra::fuzz::run_one(hydra::fuzz::parse_mid, data, size);
}
