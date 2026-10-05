// The test fixture builders' little-endian writer, forwarding to the one
// writer in core/little_endian.h. The WAV writer (audio_util.h), the .sng
// writer (sng_util.h) and the .srb writer (srb_util.h) all append their
// numbers through here.

#ifndef HYDRA_TESTS_BYTES_UTIL_H
#define HYDRA_TESTS_BYTES_UTIL_H

#include <cstdint>
#include <vector>

#include "core/little_endian.h"

namespace testbytes {

inline void put_le(std::vector<uint8_t>& out, uint64_t v, int bytes) {
    hydra::core::append_le(out, v, bytes);
}

inline void put_u16(std::vector<uint8_t>& out, uint64_t v) { put_le(out, v, 2); }
inline void put_u32(std::vector<uint8_t>& out, uint64_t v) { put_le(out, v, 4); }
inline void put_u64(std::vector<uint8_t>& out, uint64_t v) { put_le(out, v, 8); }

}  // namespace testbytes

#endif  // HYDRA_TESTS_BYTES_UTIL_H
