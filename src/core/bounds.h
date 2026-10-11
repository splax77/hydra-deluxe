// The one "do n bytes fit here" test for the binary readers (the .sng and
// .srb parsers and the Preview's .srb audio walk). It sits beside
// core/little_endian.h, whose readers check no bounds themselves.
//
// It answers yes or no only. What a "no" means stays with each caller, because
// each one handles short input its own way: some stop and keep what they have
// read, some throw.

#ifndef HYDRA_CORE_BOUNDS_H
#define HYDRA_CORE_BOUNDS_H

#include <cstddef>
#include <cstdint>
#include <vector>

namespace hydra::core {

// n bytes starting at pos lie inside something of `size` bytes. Safe for any
// pos or n: it never adds them, so nothing can wrap.
constexpr bool fits(uint64_t size, uint64_t pos, uint64_t n) {
    return pos <= size && n <= size - pos;
}
inline bool fits(const std::vector<uint8_t>& buf, uint64_t pos, uint64_t n) {
    return fits(buf.size(), pos, n);
}

}  // namespace hydra::core

#endif  // HYDRA_CORE_BOUNDS_H
