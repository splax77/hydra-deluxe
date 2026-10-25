#include "code_mask.h"

#include <filesystem>

#include "code_lexer.h"
#include "core/winstr.h"

namespace precheck {

namespace {

// Appends the spaces that stand for text[b, e): a line break stays, every
// other character becomes one space per UTF-16 unit, so
// a character's column in the masked text matches a .NET string index.
void blank(const std::string& text, size_t b, size_t e, std::string& out) {
    while (b < e) {
        size_t nl = text.find('\n', b);
        if (nl == std::string::npos || nl > e) nl = e;
        out.append(hydra::utf8_to_wide(text.substr(b, nl - b)).size(), ' ');
        if (nl < e) out += '\n';
        b = nl + 1;
    }
}

}  // namespace

std::optional<std::string> code_mask(const std::string& text, const std::string& file) {
    const std::optional<codelex::Lang> lang = codelex::language_of_extension(std::filesystem::u8path(file).extension().string());
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
