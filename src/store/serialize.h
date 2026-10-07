// Small binary primitives for the store's own formats (the rules_fp column,
// record_store.cpp). hydra.db stores no records (D87), so nothing here reads
// a record back.

#ifndef HYDRA_STORE_SERIALIZE_H
#define HYDRA_STORE_SERIALIZE_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace hydra::store {

// Not a general-purpose format: just enough structure for the store's own
// columns.
class BinaryWriter {
public:
    std::vector<uint8_t> bytes;

    void u8(uint8_t v) { bytes.push_back(v); }
    void boolean(bool v) { u8(v ? 1 : 0); }
    void u32(uint32_t v);
    void i32(int32_t v) { u32(static_cast<uint32_t>(v)); }
    void u64(uint64_t v);
    void i64(int64_t v) { u64(static_cast<uint64_t>(v)); }
    void f64(double v);
    void str(const std::string& s);

    void opt_i32(const std::optional<int>& v);
    void opt_i64(const std::optional<int64_t>& v);
    void opt_f64(const std::optional<double>& v);
    void opt_str(const std::optional<std::string>& v);
};

}  // namespace hydra::store

#endif  // HYDRA_STORE_SERIALIZE_H
