#include "code_mask.h"

#include <filesystem>

#include "code_lexer.h"
#include "core/winstr.h"
#include "views.h"

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
            // The same inside make_view blanks; what is around it is kept.
            const auto inside = string_inside(text, p);
            if (!inside) {
                out.append(text, p.begin, p.end - p.begin);
                continue;
            }
            out.append(text, p.begin, inside->first - p.begin);
            blank(text, inside->first, inside->second, out);
            out.append(text, inside->second, p.end - inside->second);
        }
    }
    return out;
}

}  // namespace precheck
