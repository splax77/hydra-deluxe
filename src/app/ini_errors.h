// How hydra_settings.ini and hydra_rules.ini word a line they refuse, and the
// whole-number test both files read through (D270 call 4). Each loader throws
// its own error type (SettingsFileError in app/config.h, RulesFileError in
// app/rules_file.h) with a text built here, so the two files' errors have one
// wording. Each text starts with the file's path, as the loader was given it.

#ifndef HYDRA_APP_INI_ERRORS_H
#define HYDRA_APP_INI_ERRORS_H

#include <cmath>
#include <limits>
#include <optional>
#include <string>
#include <string_view>

#include "core/strutil.h"

namespace hydra::app {

// A line that is not blank, not a comment and has no `=`. `text` is the line
// as ini_line_text (core/strutil.h) reads it.
inline std::string ini_not_key_value_text(const std::string& path, std::string_view text) {
    return path + ": \"" + std::string(text) + "\" is not key = value";
}

// A key the file's loader neither reads nor lists as retired.
inline std::string ini_unknown_key_text(const std::string& path, const std::string& key) {
    return path + ": unknown key \"" + key + "\"";
}

// A value its key can't take. `want` names what the key takes, read after
// "is not": "a whole number", "0 or 1".
inline std::string ini_bad_value_text(const std::string& path, const std::string& key,
                                      const std::string& value, std::string_view want) {
    return path + ": " + key + " = \"" + value + "\" is not " + std::string(want);
}

// What a whole-number key wants, in ini_bad_value_text's words.
inline constexpr std::string_view kWantWholeNumber = "a whole number";

// The whole number `text` spells, or nothing. A number is what
// parse_finite_number reads; it must have no fractional part and fit an int.
// Both files' whole-number keys and hydra_replay's --cap read through here.
inline std::optional<int> parse_whole_number(std::string_view text) {
    const std::optional<double> d = parse_finite_number(text);
    if (!d || *d != std::trunc(*d) || *d < std::numeric_limits<int>::min() ||
        *d > std::numeric_limits<int>::max())
        return std::nullopt;
    return static_cast<int>(*d);
}

}  // namespace hydra::app

#endif  // HYDRA_APP_INI_ERRORS_H
