// The one reader of a little-endian number from bytes. The store's
// BinaryReader, the .sng and .srb parsers and the Preview's SRB audio walk all
// read through it. It lives in core/ because parse/ cannot include store/.
//
// It reads at the pointer it is given and checks no bounds. Each caller checks
// its own first, because each one handles short input its own way.

#ifndef HYDRA_CORE_LITTLE_ENDIAN_H
#define HYDRA_CORE_LITTLE_ENDIAN_H

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace hydra::core {

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

}  // namespace hydra::core

#endif  // HYDRA_CORE_LITTLE_ENDIAN_H
