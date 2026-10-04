// The one test wrapper that counts what a load reads: the container tests use
// it to prove a .sng or .srb load reads only its header and notes.

#ifndef HYDRA_TESTS_BYTE_SOURCE_UTIL_H
#define HYDRA_TESTS_BYTE_SOURCE_UTIL_H

#include <cstddef>
#include <cstdint>
#include <vector>

#include "core/winstr.h"

namespace testbytes {

// `inner`, adding every byte it hands back to `bytes_read`, which must outlive
// the returned source.
inline hydra::ByteSource counting(hydra::ByteSource inner, uint64_t& bytes_read) {
    hydra::ByteSource out;
    out.size = inner.size;
    out.read = [inner, &bytes_read](uint64_t offset, size_t length) {
        std::vector<uint8_t> b = inner.read(offset, length);
        bytes_read += b.size();
        return b;
    };
    return out;
}

}  // namespace testbytes

#endif  // HYDRA_TESTS_BYTE_SOURCE_UTIL_H
