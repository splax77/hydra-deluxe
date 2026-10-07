#include "store/serialize.h"

#include <cstring>

#include "core/little_endian.h"

namespace hydra::store {

void BinaryWriter::u32(uint32_t v) { core::append_le_u32(bytes, v); }
void BinaryWriter::u64(uint64_t v) { core::append_le_u64(bytes, v); }
void BinaryWriter::f64(double v) {
    uint64_t bits;
    std::memcpy(&bits, &v, sizeof(bits));
    u64(bits);
}
void BinaryWriter::str(const std::string& s) {
    u32(static_cast<uint32_t>(s.size()));
    bytes.insert(bytes.end(), s.begin(), s.end());
}
void BinaryWriter::opt_i32(const std::optional<int>& v) {
    boolean(v.has_value());
    if (v) i32(*v);
}
void BinaryWriter::opt_i64(const std::optional<int64_t>& v) {
    boolean(v.has_value());
    if (v) i64(*v);
}
void BinaryWriter::opt_f64(const std::optional<double>& v) {
    boolean(v.has_value());
    if (v) f64(*v);
}
void BinaryWriter::opt_str(const std::optional<std::string>& v) {
    boolean(v.has_value());
    if (v) str(*v);
}

}  // namespace hydra::store
