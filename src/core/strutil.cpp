#include "core/strutil.h"
#include <charconv>
#include <cmath>
#include <system_error>

namespace hydra {

namespace {

constexpr std::string_view kSpace = " \t\r\n\v\f";

}  // namespace

char lower_ascii(char c) {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

std::string to_lower_ascii(std::string_view s) {
    std::string out(s);
    for (char& c : out) c = lower_ascii(c);
    return out;
}

bool is_ascii_space(char c) {
    return kSpace.find(c) != std::string_view::npos;
}

bool equals_ci(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (lower_ascii(a[i]) != lower_ascii(b[i])) return false;
    return true;
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

std::string replace_all(std::string s, std::string_view from, std::string_view to) {
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from.size(), to);
        pos += to.size();
    }
    return s;
}

bool starts_with(std::string_view s, std::string_view prefix) {
    return s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix;
}

bool starts_with_ci(std::string_view s, std::string_view prefix) {
    return s.size() >= prefix.size() && equals_ci(s.substr(0, prefix.size()), prefix);
}

bool ends_with(std::string_view s, std::string_view suffix) {
    return s.size() >= suffix.size() && s.substr(s.size() - suffix.size()) == suffix;
}

bool ends_with_ci(std::string_view s, std::string_view suffix) {
    return s.size() >= suffix.size() && equals_ci(s.substr(s.size() - suffix.size()), suffix);
}

std::string relative_slash_path(std::string_view path, std::string_view root) {
    if (!root.empty() && path.size() > root.size() && starts_with(path, root))
        path = path.substr(root.size() + 1);
    std::string out(path);
    for (char& c : out)
        if (c == '\\') c = '/';
    return out;
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

std::string_view ini_line_text(std::string_view line) {
    return trim_view(line.substr(0, line.find('#')));
}

std::optional<IniPair> split_ini_line(std::string_view line) {
    const std::string_view text = ini_line_text(line);
    const size_t eq = text.find('=');
    if (eq == std::string_view::npos) return std::nullopt;
    // `text` lies inside `line`, so the = sits at the same place in both.
    const size_t eq_in_line = static_cast<size_t>(text.data() - line.data()) + eq;
    return IniPair{trim(text.substr(0, eq)), trim(text.substr(eq + 1)),
                   trim(line.substr(eq_in_line + 1))};
}

std::optional<bool> parse_bool(std::string_view text) {
    if (text == "1") return true;
    if (text == "0") return false;
    return std::nullopt;
}

}  // namespace hydra
