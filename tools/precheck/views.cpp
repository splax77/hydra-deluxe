#include "views.h"

#include <algorithm>

#include "range.h"

namespace precheck {

namespace {

// Every byte of [b, e) but a line break becomes a space.
void blank(std::string& s, size_t b, size_t e) {
    for (size_t i = b; i < e; ++i)
        if (s[i] != '\n') s[i] = ' ';
}

}  // namespace

View make_view(const std::string& text, codelex::Lang lang) {
    View v;
    v.raw = text;
    v.no_comments = text;
    v.code = text;
    for (const codelex::Piece& p : codelex::split(text, lang)) {
        if (p.kind == codelex::Kind::Comment) {
            blank(v.no_comments, p.begin, p.end);
            blank(v.code, p.begin, p.end);
        } else if (p.kind == codelex::Kind::String) {
            // The script's Blank-Inner: keep any prefix and the opening
            // quote, and the last character when it closes the literal (an
            // unclosed one ends at its line); blank what is between.
            const size_t open = text.find_first_of("\"'", p.begin);
            if (open == std::string::npos || open + 1 >= p.end) continue;
            const bool closed = text[p.end - 1] == text[open];
            blank(v.code, open + 1, closed ? p.end - 1 : p.end);
        }
    }
    v.line_starts.push_back(0);
    for (size_t i = 0; i < text.size(); ++i)
        if (text[i] == '\n') v.line_starts.push_back(i + 1);
    return v;
}

int View::line_of(size_t offset) const {
    // The last line start at or before the offset.
    return static_cast<int>(std::upper_bound(line_starts.begin(), line_starts.end(), offset) - line_starts.begin());
}

std::string View::line(const std::string& text, int n) const {
    if (n < 1 || n > line_count()) return {};
    const size_t b = line_starts[n - 1];
    const size_t e = n < line_count() ? line_starts[n] - 1 : text.size();
    return text.substr(b, e - b);
}

codelex::Lang view_lang(const std::string& path) {
    return is_py(path) ? codelex::Lang::Python : codelex::Lang::Cpp;
}

const View& ViewCache::get(const std::string& rev, const std::string& path) {
    auto key = std::make_pair(rev, path);
    auto it = views_.find(key);
    if (it == views_.end())
        it = views_.emplace(key, make_view(repo_.files().text(rev, path), view_lang(path))).first;
    return it->second;
}

}  // namespace precheck
