#include "checks.h"

#include <stdexcept>

namespace precheck {

const std::vector<Check> kChecks = {
    {"Helpers defined twice (kind C)", run_check_1},
    {"Recompute spellings in tests (kind B)", run_check_2},
    {"New numbers with no decision (kind D)", run_check_3},
    {"Scans outside the scan file (kind E)", run_check_4},
    {"Second homes in src/ (kind F)", run_check_5},
};

std::set<int> parse_disable(const std::vector<std::string>& values) {
    std::set<int> out;
    const std::string limit = std::to_string(kChecks.size());
    for (const std::string& v : values) {
        size_t at = 0;
        while (at <= v.size()) {
            size_t comma = v.find(',', at);
            if (comma == std::string::npos) comma = v.size();
            const std::string piece = v.substr(at, comma - at);
            at = comma + 1;
            if (piece.empty()) continue;
            bool digits = piece.size() <= 9;
            for (const char c : piece) digits = digits && c >= '0' && c <= '9';
            const int n = digits ? std::stoi(piece) : 0;
            if (n < 1 || n > static_cast<int>(kChecks.size()))
                throw std::invalid_argument("--disable takes check numbers 1 to " + limit + ", not '" +
                                            piece + "'");
            out.insert(n);
        }
    }
    return out;
}

}  // namespace precheck
