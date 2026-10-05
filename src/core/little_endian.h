// The one reader and writer of a little-endian number. Every reader and writer
// calls it; the scan row in tests/test_single_owner.cpp keeps it so. It lives
// in core/ because parse/ cannot include store/.
//
// Neither function checks bounds. Each caller checks its own first, because
// each one handles short input its own way.

#ifndef HYDRA_CORE_LITTLE_ENDIAN_H
#define HYDRA_CORE_LITTLE_ENDIAN_H

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace hydra::core {

// ---- reading ---------------------------------------------------------------

// sizeof(T) bytes at p, lowest byte first.
template <class T>
constexpr T read_le(const uint8_t* p) {
    static_assert(std::is_unsigned_v<T>, "read_le reads unsigned numbers");
    T v = 0;
    for (std::size_t i = 0; i < sizeof(T); ++i) v = static_cast<T>(v | (static_cast<T>(p[i]) << (8 * i)));
    return v;
}

constexpr uint16_t read_le_u16(const uint8_t* p) { return read_le<uint16_t>(p); }
constexpr uint32_t read_le_u32(const uint8_t* p) { return read_le<uint32_t>(p); }
constexpr uint64_t read_le_u64(const uint8_t* p) { return read_le<uint64_t>(p); }

// ---- writing ---------------------------------------------------------------

// Appends the low `bytes` bytes of v, lowest first.
inline void append_le(std::vector<uint8_t>& out, uint64_t v, int bytes) {
    for (int i = 0; i < bytes; ++i) out.push_back(static_cast<uint8_t>(v >> (8 * i)));
}

inline void append_le_u16(std::vector<uint8_t>& out, uint64_t v) { append_le(out, v, 2); }
inline void append_le_u32(std::vector<uint8_t>& out, uint64_t v) { append_le(out, v, 4); }
inline void append_le_u64(std::vector<uint8_t>& out, uint64_t v) { append_le(out, v, 8); }

}  // namespace hydra::core

#endif  // HYDRA_CORE_LITTLE_ENDIAN_H
