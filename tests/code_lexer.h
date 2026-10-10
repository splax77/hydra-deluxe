// Splits a source file's text into code, comments and string literals, so a
// test that reads the repo's own sources can tell a name the code uses from a
// name a comment or a string only mentions. It knows C++, Python, PowerShell
// and YAML (they differ in comment markers and string escapes). It is a
// lexer for this repo's tests, not a compiler: it does not expand macros, and
// it ends a broken single-line string at the end of its line instead of
// running on through the file.
//
// The docs check in tests/test_docs_match_code.cpp and the single-owner scan
// in tests/single_owner/scan.cpp use it. Its own case is "code lexer: ..." in
// tests/test_docs_match_code.cpp.
#pragma once

#include <cctype>
#include <optional>
#include <string>
#include <vector>

namespace codelex {

enum class Lang { Cpp, Python, PowerShell, Yaml };
enum class Kind { Code, Comment, String };

// One run of text of one kind: text.substr(begin, end - begin). A string
// piece holds the whole literal, its quotes and any prefix (R, u8, f, rb, ...)
// included. A character literal counts as a string.
struct Piece {
    Kind kind;
    size_t begin;
    size_t end;
};

namespace detail {

inline bool ident_char(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

// Adds [begin, end) as a piece of `kind`, joining it to the last piece when
// that one has the same kind.
inline void add(std::vector<Piece>& out, Kind kind, size_t begin, size_t end) {
    if (begin >= end) return;
    if (!out.empty() && out.back().kind == kind && out.back().end == begin) {
        out.back().end = end;
        return;
    }
    out.push_back({kind, begin, end});
}

// The end of a quoted literal that opened at `open` (the quote's index) with
// quote character q and backslash escapes: one past the closing quote, or the
// end of the line when the line ends first.
inline size_t quoted_end(const std::string& t, size_t open, char q) {
    size_t i = open + 1;
    while (i < t.size()) {
        const char c = t[i];
        if (c == '\\' && i + 1 < t.size()) {
            i += 2;
            continue;
        }
        if (c == q) return i + 1;
        if (c == '\n') return i;
        ++i;
    }
    return i;
}

inline bool is_raw_prefix(const std::string& id) {
    return id == "R" || id == "u8R" || id == "LR" || id == "uR" || id == "UR";
}

inline std::vector<Piece> split_cpp(const std::string& t) {
    std::vector<Piece> out;
    size_t i = 0;
    size_t code_from = 0;  // start of the code run not yet added
    auto flush_code = [&](size_t upto) {
        add(out, Kind::Code, code_from, upto);
    };
    while (i < t.size()) {
        const char c = t[i];
        const char n = i + 1 < t.size() ? t[i + 1] : '\0';
        if (c == '/' && n == '/') {
            flush_code(i);
            size_t e = t.find('\n', i);
            if (e == std::string::npos) e = t.size();
            add(out, Kind::Comment, i, e);
            i = code_from = e;
            continue;
        }
        if (c == '/' && n == '*') {
            flush_code(i);
            size_t e = t.find("*/", i + 2);
            e = e == std::string::npos ? t.size() : e + 2;
            add(out, Kind::Comment, i, e);
            i = code_from = e;
            continue;
        }
        if (ident_char(c)) {
            const size_t start = i;
            if (std::isdigit(static_cast<unsigned char>(c))) {
                // A number, with its digit separators (1'000) kept inside it.
                while (i < t.size() &&
                       (ident_char(t[i]) || t[i] == '.' ||
                        (t[i] == '\'' && i + 1 < t.size() && ident_char(t[i + 1]))))
                    ++i;
                continue;
            }
            while (i < t.size() && ident_char(t[i])) ++i;
            if (i < t.size() && t[i] == '"' && is_raw_prefix(t.substr(start, i - start))) {
                // R"delim( ... )delim"
                const size_t paren = t.find('(', i + 1);
                size_t e = t.size();
                if (paren != std::string::npos) {
                    const std::string close = ")" + t.substr(i + 1, paren - i - 1) + "\"";
                    const size_t at = t.find(close, paren + 1);
                    if (at != std::string::npos) e = at + close.size();
                }
                flush_code(start);
                add(out, Kind::String, start, e);
                i = code_from = e;
            }
            continue;
        }
        if (c == '"' || c == '\'') {
            flush_code(i);
            const size_t e = quoted_end(t, i, c);
            add(out, Kind::String, i, e);
            i = code_from = e;
            continue;
        }
        ++i;
    }
    flush_code(t.size());
    return out;
}

// A Python string prefix: any mix of r, b, f, u in either case, at most two
// letters (rb, Rb, fr, ...).
inline bool is_py_prefix(const std::string& id) {
    if (id.empty() || id.size() > 2) return false;
    for (const char ch : id) {
        const char l = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        if (l != 'r' && l != 'b' && l != 'f' && l != 'u') return false;
    }
    return true;
}

// The end of a Python string whose quote run starts at `q`: one past the
// closing quote(s). A single-quoted string stops at the end of its line.
inline size_t py_string_end(const std::string& t, size_t q) {
    const char ch = t[q];
    if (t.compare(q, 3, std::string(3, ch)) == 0) {
        const std::string close(3, ch);
        size_t i = q + 3;
        while (i < t.size()) {
            if (t[i] == '\\' && i + 1 < t.size()) {
                i += 2;
                continue;
            }
            if (t.compare(i, 3, close) == 0) return i + 3;
            ++i;
        }
        return t.size();
    }
    return quoted_end(t, q, ch);
}

inline std::vector<Piece> split_python(const std::string& t) {
    std::vector<Piece> out;
    size_t i = 0;
    size_t code_from = 0;
    auto flush_code = [&](size_t upto) {
        add(out, Kind::Code, code_from, upto);
    };
    while (i < t.size()) {
        const char c = t[i];
        if (c == '#') {
            flush_code(i);
            size_t e = t.find('\n', i);
            if (e == std::string::npos) e = t.size();
            add(out, Kind::Comment, i, e);
            i = code_from = e;
            continue;
        }
        if (ident_char(c)) {
            const size_t start = i;
            while (i < t.size() && ident_char(t[i])) ++i;
            if (i < t.size() && (t[i] == '"' || t[i] == '\'') &&
                is_py_prefix(t.substr(start, i - start))) {
                flush_code(start);
                const size_t e = py_string_end(t, i);
                add(out, Kind::String, start, e);
                i = code_from = e;
            }
            continue;
        }
        if (c == '"' || c == '\'') {
            flush_code(i);
            const size_t e = py_string_end(t, i);
            add(out, Kind::String, i, e);
            i = code_from = e;
            continue;
        }
        ++i;
    }
    flush_code(t.size());
    return out;
}

// The end of a PowerShell quoted string that opened at `open` with quote q: one
// past the closing quote, or the end of the text when it never closes. A
// doubled quote stays inside the string, and a double-quoted string also
// treats a backtick as an escape. PowerShell strings may span lines.
inline size_t ps_quoted_end(const std::string& t, size_t open, char q) {
    size_t i = open + 1;
    while (i < t.size()) {
        const char c = t[i];
        if (c == '`' && q == '"' && i + 1 < t.size()) {
            i += 2;
            continue;
        }
        if (c == q) {
            if (i + 1 < t.size() && t[i + 1] == q) {
                i += 2;
                continue;
            }
            return i + 1;
        }
        ++i;
    }
    return t.size();
}

// The end of a PowerShell here-string whose "@" is at `at` (@' or @" at the
// end of its line, closed by '@ or "@ at the start of a later line): one past
// the closing "@". Returns `at` when there is no here-string there.
inline size_t ps_here_string_end(const std::string& t, size_t at) {
    if (at + 1 >= t.size() || (t[at + 1] != '\'' && t[at + 1] != '"')) return at;
    size_t j = at + 2;
    while (j < t.size() && (t[j] == ' ' || t[j] == '\t' || t[j] == '\r')) ++j;
    if (j >= t.size() || t[j] != '\n') return at;
    const size_t close = t.find(std::string("\n") + t[at + 1] + "@", j);
    return close == std::string::npos ? t.size() : close + 3;
}

// Splits `t` with piece_at(t, i, kind), which returns the end of the comment
// or string piece that starts at i and sets its kind, or returns i when code
// goes on there. Everything between those pieces is code.
template <typename PieceAt>
std::vector<Piece> split_by_piece(const std::string& t, PieceAt piece_at) {
    std::vector<Piece> out;
    size_t i = 0;
    size_t code_from = 0;
    while (i < t.size()) {
        Kind kind = Kind::String;
        const size_t e = piece_at(t, i, kind);
        if (e == i) {
            ++i;
            continue;
        }
        add(out, Kind::Code, code_from, i);
        add(out, kind, i, e);
        i = code_from = e;
    }
    add(out, Kind::Code, code_from, t.size());
    return out;
}

// The end of the line that holds `i`: the index of its newline, or the end of
// the text.
inline size_t line_end(const std::string& t, size_t i) {
    const size_t e = t.find('\n', i);
    return e == std::string::npos ? t.size() : e;
}

inline std::vector<Piece> split_powershell(const std::string& t) {
    return split_by_piece(t, [](const std::string& s, size_t i, Kind& kind) -> size_t {
        const char c = s[i];
        if (c == '<' && i + 1 < s.size() && s[i + 1] == '#') {
            kind = Kind::Comment;
            const size_t e = s.find("#>", i + 2);
            return e == std::string::npos ? s.size() : e + 2;
        }
        if (c == '#') {
            kind = Kind::Comment;
            return line_end(s, i);
        }
        if (c == '@') return ps_here_string_end(s, i);
        if (c == '"' || c == '\'') return ps_quoted_end(s, i, c);
        return i;
    });
}

// The end of a YAML quoted value that opened at `open` with quote q: one past
// the closing quote, or the end of the text when it never closes. A
// single-quoted value doubles its quote to hold one; a double-quoted one
// escapes with a backslash. Both may span lines.
inline size_t yaml_quoted_end(const std::string& t, size_t open, char q) {
    size_t i = open + 1;
    while (i < t.size()) {
        if (q == '"' && t[i] == '\\' && i + 1 < t.size()) {
            i += 2;
            continue;
        }
        if (t[i] == q) {
            if (q == '\'' && i + 1 < t.size() && t[i + 1] == '\'') {
                i += 2;
                continue;
            }
            return i + 1;
        }
        ++i;
    }
    return t.size();
}

// Whether a quote at `at` opens a YAML quoted value: it is the first thing on
// its line, or follows "[", "{" or ",", or follows ":", "-" or "?" and a
// space. Anywhere else it is a plain value's apostrophe, as in "it's".
inline bool yaml_value_starts(const std::string& t, size_t at) {
    size_t j = at;
    while (j > 0 && (t[j - 1] == ' ' || t[j - 1] == '\t')) --j;
    if (j == 0 || t[j - 1] == '\n') return true;
    const char p = t[j - 1];
    if (p == '[' || p == '{' || p == ',') return true;
    return (p == ':' || p == '-' || p == '?') && j < at;
}

// YAML, as the workflows under .github/ use it: a # opens a comment at the
// start of a line or after a space or tab. A block value's lines (after "|"
// or ">") are read the same way, so a shell comment in a run step counts as a
// comment.
inline std::vector<Piece> split_yaml(const std::string& t) {
    return split_by_piece(t, [](const std::string& s, size_t i, Kind& kind) -> size_t {
        const char c = s[i];
        if (c == '#' && (i == 0 || s[i - 1] == '\n' || s[i - 1] == ' ' || s[i - 1] == '\t')) {
            kind = Kind::Comment;
            return line_end(s, i);
        }
        if ((c == '"' || c == '\'') && yaml_value_starts(s, i)) return yaml_quoted_end(s, i, c);
        return i;
    });
}

}  // namespace detail

// The language a file is read in, by its extension (with the dot, as
// std::filesystem::path::extension gives it), or nothing when the lexer does
// not read such files. The one place that maps an extension to a Lang.
inline std::optional<Lang> language_of_extension(const std::string& ext) {
    if (ext == ".cpp" || ext == ".h" || ext == ".c" || ext == ".hlsl") return Lang::Cpp;
    if (ext == ".py") return Lang::Python;
    if (ext == ".ps1" || ext == ".psm1") return Lang::PowerShell;
    if (ext == ".yml" || ext == ".yaml") return Lang::Yaml;
    return std::nullopt;
}

// The pieces of `text`, in order, covering all of it.
inline std::vector<Piece> split(const std::string& text, Lang lang) {
    switch (lang) {
        case Lang::Cpp: return detail::split_cpp(text);
        case Lang::Python: return detail::split_python(text);
        case Lang::PowerShell: return detail::split_powershell(text);
        case Lang::Yaml: return detail::split_yaml(text);
    }
    return {};
}

}  // namespace codelex
