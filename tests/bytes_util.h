// The one little-endian number writer the test fixture builders share. The
// WAV writer (audio_util.h), the .sng writer (sng_util.h) and the .srb writer
// (srb_util.h) all append their numbers through here. Review of M6-J1c
// finding 3: each kept its own copy of the same loop before.
//
// Each writer appends the low bytes of `v`, least significant first, and
// drops the rest, so a caller may pass a wider value (a size_t length, say).

#ifndef HYDRA_TESTS_BYTES_UTIL_H
#define HYDRA_TESTS_BYTES_UTIL_H

#include <cstdint>
#include <vector>

namespace testbytes {

// Appends the low `bytes` bytes of `v` to `out`, least significant first.
inline void put_le(std::vector<uint8_t>& out, uint64_t v, int bytes) {
    for (int i = 0; i < bytes; ++i) out.push_back(static_cast<uint8_t>(v >> (8 * i)));
}

inline void put_u16(std::vector<uint8_t>& out, uint64_t v) { put_le(out, v, 2); }
inline void put_u32(std::vector<uint8_t>& out, uint64_t v) { put_le(out, v, 4); }
inline void put_u64(std::vector<uint8_t>& out, uint64_t v) { put_le(out, v, 8); }

}  // namespace testbytes

#endif  // HYDRA_TESTS_BYTES_UTIL_H
