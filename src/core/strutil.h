// Small, generic string helpers with no other natural home. Every caller in
// Hydra uses these rather than its own copy.

#ifndef HYDRA_CORE_STRUTIL_H
#define HYDRA_CORE_STRUTIL_H

#include <optional>
#include <string>
#include <string_view>

namespace hydra {

// One byte lowered the ASCII way: A-Z become a-z and every other byte comes
// back as it was, so a UTF-8 lead or continuation byte is never touched. The
// one case rule every comparison below, and to_lower_ascii, uses. It never
// reads the C locale, so no locale setting can change it.
char lower_ascii(char c);

// ASCII-only lowercase: A-Z become a-z and every other byte is left alone, so
// UTF-8 text passes through intact. Chart hashes, file names and ini keys all
// go through here.
std::string to_lower_ascii(std::string_view s);

// Whether a byte is ASCII whitespace: space, tab, CR, LF, vertical tab or form
// feed. The set trim strips. A byte of 0x80 or above is never whitespace.
bool is_ascii_space(char c);

// Whether two texts are equal ignoring ASCII case ("Expert" equals "EXPERT").
// Texts of different lengths are never equal.
bool equals_ci(std::string_view a, std::string_view b);

// Strips ASCII whitespace (is_ascii_space) from both ends. Inner whitespace is
// kept.
std::string trim(std::string_view s);

// trim() without the copy: the same whitespace set, returned as a view into s
// (empty when s is all whitespace). Valid only while s's storage lives.
std::string_view trim_view(std::string_view s);

// Whether s starts with prefix, byte for byte.
bool starts_with(std::string_view s, std::string_view prefix);

// Whether s starts with prefix, ignoring ASCII case ("ARTIST:x" starts with
// "artist:").
bool starts_with_ci(std::string_view s, std::string_view prefix);

// Whether s ends with suffix, byte for byte.
bool ends_with(std::string_view s, std::string_view suffix);

// Whether s ends with suffix, ignoring ASCII case (".MID" matches ".mid").
bool ends_with_ci(std::string_view s, std::string_view suffix);

// A scanned path as scan_snapshot.json keys it: when path starts with root
// and is longer, the root and the one byte after it are dropped, then every
// backslash becomes a forward slash. "C:\in\a\notes.mid" under "C:\in" is
// "a/notes.mid". The dropped byte is whatever follows the root, separator or
// not ("C:/input/x" under "C:/in" is "ut/x", audit finding 274); the snapshot's
// keys were written that way, so the rule stays. An empty root strips nothing.
std::string relative_slash_path(std::string_view path, std::string_view root);

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
