#include "items.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
#include <map>

namespace precheck {

const char* const kSizePrefix = "range size: ";
const char* const kLargeRangePrefix = "large range: ";

const std::string& kind_order() {
    static const std::string order = "CBADEF";
    return order;
}

namespace {

// Each byte's place in the order. The ASCII punctuation and digits come from
// one run of PowerShell 7.6 under en-US sorting every printable ASCII
// character; letters follow, case ignored; every other byte comes after, in
// byte order.
const std::array<int, 256>& ranks() {
    static const std::array<int, 256> table = [] {
        std::array<int, 256> t{};
        const char* measured = " _-,;:!?.'\"()[]{}@*/\\&#%`^+<=>|~$0123456789";
        const int n = static_cast<int>(std::strlen(measured));
        for (int b = 0; b < 256; ++b) t[b] = 1000 + b;
        for (int i = 0; i < n; ++i) t[static_cast<unsigned char>(measured[i])] = i;
        for (int c = 0; c < 26; ++c) {
            t['a' + c] = n + c;
            t['A' + c] = n + c;
        }
        return t;
    }();
    return table;
}

int text_compare(const std::string& a, const std::string& b) {
    const std::array<int, 256>& r = ranks();
    const size_t n = std::min(a.size(), b.size());
    for (size_t i = 0; i < n; ++i) {
        const int x = r[static_cast<unsigned char>(a[i])];
        const int y = r[static_cast<unsigned char>(b[i])];
        if (x != y) return x < y ? -1 : 1;
    }
    if (a.size() == b.size()) return 0;
    return a.size() < b.size() ? -1 : 1;
}

int kind_rank(const std::string& kind) {
    const size_t at = kind.size() == 1 ? kind_order().find(kind[0]) : std::string::npos;
    return at == std::string::npos ? static_cast<int>(kind_order().size()) : static_cast<int>(at);
}

// -1, 0 or 1 over the four sort keys.
int item_compare(const Item& a, const Item& b) {
    const int ka = kind_rank(a.kind), kb = kind_rank(b.kind);
    if (ka != kb) return ka < kb ? -1 : 1;
    if (const int c = text_compare(a.file, b.file)) return c;
    if (a.line != b.line) return a.line < b.line ? -1 : 1;
    return text_compare(a.what, b.what);
}

}  // namespace

bool text_before(const std::string& a, const std::string& b) { return text_compare(a, b) < 0; }

void Items::add(const std::string& kind, const std::string& file, int line, const std::string& what,
                const std::string& why) {
    items_.push_back({kind, file, line, what, why});
}

std::vector<Item> Items::sorted() const {
    std::vector<Item> out = items_;
    std::stable_sort(out.begin(), out.end(),
                     [](const Item& a, const Item& b) { return item_compare(a, b) < 0; });
    out.erase(std::unique(out.begin(), out.end(),
                          [](const Item& a, const Item& b) { return item_compare(a, b) == 0; }),
              out.end());
    return out;
}

std::string item_line(const Item& i) {
    return i.kind + " " + i.file + ":" + std::to_string(i.line) + "  " + i.what + "  -- " + i.why;
}

std::string size_line(const RangeSize& size) {
    std::string excluded;
    for (const std::string& f : size_excluded_folders()) {
        if (!excluded.empty()) excluded += " or ";
        excluded += f + "/";
    }
    return std::string(kSizePrefix) + std::to_string(size.total()) + " changed lines (" +
           std::to_string(size.added) + " added, " + std::to_string(size.deleted) +
           " deleted), not counting " + excluded;
}

std::optional<std::string> large_range_line(const RangeSize& size) {
    if (size.total() <= kLargeRangeLines) return std::nullopt;
    return std::string(kLargeRangePrefix) + std::to_string(size.total()) +
           " changed lines is over the " + std::to_string(kLargeRangeLines) +
           "-line threshold. Split this range into smaller merges before review. This is a "
           "warning, not a gate.";
}

std::string summary_line(const std::vector<Item>& sorted, const Range& range) {
    // Kind counts in letter order, as the script's Group-Object gave them.
    std::map<std::string, size_t> per_kind;
    for (const Item& i : sorted) ++per_kind[i.kind];
    std::string counts;
    for (const auto& [kind, n] : per_kind) {
        if (!counts.empty()) counts += ", ";
        counts += kind + " " + std::to_string(n);
    }
    return "derive_once_precheck: " + std::to_string(sorted.size()) + " items (" +
           (counts.empty() ? "none" : counts) + ") for " +
           (range.whole_tree ? std::string("the whole tree") : range.spelling) + " at " +
           range.tip.substr(0, 7);
}

}  // namespace precheck
