// Small, generic string helpers with no other natural home. Every caller in
// Hydra uses these rather than its own copy.

#ifndef HYDRA_CORE_STRUTIL_H
#define HYDRA_CORE_STRUTIL_H

#include <optional>
#include <string>
#include <string_view>

namespace hydra {

// ASCII-only lowercase: A-Z become a-z and every other byte is left alone, so
// UTF-8 text passes through intact. Chart hashes, file names and ini keys all
// go through here.
std::string to_lower_ascii(std::string_view s);

// Strips ASCII whitespace (space, tab, CR, LF, vertical tab, form feed) from
// both ends. Inner whitespace is kept.
std::string trim(std::string_view s);

// trim() without the copy: the same whitespace set, returned as a view into s
// (empty when s is all whitespace). Valid only while s's storage lives.
std::string_view trim_view(std::string_view s);

// Whether s ends with suffix, byte for byte.
bool ends_with(std::string_view s, std::string_view suffix);

// Whether s ends with suffix, ignoring ASCII case (".MID" matches ".mid").
bool ends_with_ci(std::string_view s, std::string_view suffix);

// The number a chart file's text spells, read one way for every such number
// (.chart Offset, song.ini and .sng delay): spaces at either end are allowed,
// then an optional '+', then one finite decimal number and nothing else.
// Anything else ("500ms", "0.25s", "nan", "inf", "", "0x1F4") is absent.
// Locale-independent.
std::optional<double> parse_finite_number(std::string_view text);

// One line of an INI file (hydra_settings.ini, hydra_rules.ini), read one way
// for both. Everything from a # on is a comment, whole-line or trailing.
//
// ini_line_text: the line with its comment cut off and spaces trimmed. Empty
// for a blank or comment-only line. A view into `line`.
std::string_view ini_line_text(std::string_view line);

// A "key = value" line, split at its first =.
struct IniPair {
    std::string key;    // trimmed
    std::string value;  // trimmed, comment cut off
    // Everything after the first =, trimmed, with any # kept. For a key whose
    // value is free text, such as a folder path, where # is part of a name.
    std::string whole_value;
};

// The key and value of an INI line, or nothing for a blank line, a
// comment-only line, or a line with no = before its comment. Each reader
// decides what a line with no = means: hydra_settings.ini skips it,
// hydra_rules.ini refuses it.
std::optional<IniPair> split_ini_line(std::string_view line);

// What text means on or off in an INI file: exactly "1" is on and exactly
// "0" is off. Anything else ("true", "yes", "", " 1") is absent, and the
// caller keeps what it had.
std::optional<bool> parse_bool(std::string_view text);

}  // namespace hydra

#endif  // HYDRA_CORE_STRUTIL_H
