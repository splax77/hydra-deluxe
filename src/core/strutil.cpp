#include "core/strutil.h"
#include <charconv>
#include <cmath>
#include <system_error>

namespace hydra {

namespace {

constexpr std::string_view kSpace = " \t\r\n\v\f";

char lower_ascii(char c) {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

}  // namespace

std::string to_lower_ascii(std::string_view s) {
    std::string out(s);
    for (char& c : out) c = lower_ascii(c);
    return out;
}

std::string_view trim_view(std::string_view s) {
    const size_t a = s.find_first_not_of(kSpace);
    if (a == std::string_view::npos) return {};
    const size_t b = s.find_last_not_of(kSpace);
    return s.substr(a, b - a + 1);
}

std::string trim(std::string_view s) {
    return std::string(trim_view(s));
}

bool ends_with(std::string_view s, std::string_view suffix) {
    return s.size() >= suffix.size() && s.substr(s.size() - suffix.size()) == suffix;
}

bool ends_with_ci(std::string_view s, std::string_view suffix) {
    if (s.size() < suffix.size()) return false;
    const std::string_view tail = s.substr(s.size() - suffix.size());
    for (size_t i = 0; i < suffix.size(); ++i)
        if (lower_ascii(tail[i]) != lower_ascii(suffix[i])) return false;
    return true;
}

std::optional<double> parse_finite_number(std::string_view text) {
    std::string_view s = trim_view(text);
    if (!s.empty() && s.front() == '+') {
        s.remove_prefix(1);
        if (!s.empty() && (s.front() == '+' || s.front() == '-')) return std::nullopt;
    }
    if (s.empty()) return std::nullopt;
    double value = 0.0;
    const char* const last = s.data() + s.size();
    const auto [end, ec] = std::from_chars(s.data(), last, value, std::chars_format::general);
    if (ec != std::errc() || end != last || !std::isfinite(value)) return std::nullopt;
    return value;
}

}  // namespace hydra
