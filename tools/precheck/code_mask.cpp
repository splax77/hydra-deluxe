#include "code_mask.h"

#include "code_lexer.h"

namespace precheck {

namespace {

// Appends the spaces that stand for text[b, e): a line break stays, every
// other character becomes one space per UTF-16 unit.
void blank(const std::string& text, size_t b, size_t e, std::string& out) {
    for (size_t i = b; i < e; ++i) {
        const unsigned char c = static_cast<unsigned char>(text[i]);
        if (c == '\n') out += '\n';
        else if (c >= 0xF0) out += "  ";  // starts a four-byte character: two UTF-16 units
        else if (c < 0x80 || c >= 0xC0) out += ' ';
        // a UTF-8 continuation byte: its character already has its space
    }
}

std::string extension_of(const std::string& file) {
    const size_t slash = file.find_last_of("/\\");
    const size_t dot = file.rfind('.');
    if (dot == std::string::npos || (slash != std::string::npos && dot < slash)) return "";
    return file.substr(dot);
}

}  // namespace

std::optional<std::string> code_mask(const std::string& text, const std::string& file) {
    const std::optional<codelex::Lang> lang = codelex::language_of_extension(extension_of(file));
    if (!lang || (*lang != codelex::Lang::Cpp && *lang != codelex::Lang::Python)) return std::nullopt;
    std::string out;
    out.reserve(text.size());
    for (const codelex::Piece& p : codelex::split(text, *lang)) {
        if (p.kind == codelex::Kind::Code) {
            out.append(text, p.begin, p.end - p.begin);
        } else if (p.kind == codelex::Kind::Comment) {
            blank(text, p.begin, p.end, out);
        } else {
            // Keep the prefix and the opening quote; keep the last character
            // when it closes the literal (an unclosed one ends at its line).
            const size_t open = text.find_first_of("\"'", p.begin);
            if (open == std::string::npos || open + 1 >= p.end) {
                out.append(text, p.begin, p.end - p.begin);
                continue;
            }
            const bool closed = text[p.end - 1] == text[open];
            const size_t inner_end = closed ? p.end - 1 : p.end;
            out.append(text, p.begin, open + 1 - p.begin);
            blank(text, open + 1, inner_end, out);
            if (closed) out += text[p.end - 1];
        }
    }
    return out;
}

}  // namespace precheck
