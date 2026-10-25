#include "braces.h"

namespace precheck {

Braces::Braces(const std::string& code) {
    std::vector<size_t> stack;
    for (size_t i = 0; i < code.size(); ++i) {
        if (code[i] == '{') {
            const size_t p = stack.empty() ? kTop : stack.back();
            parent_[i] = p;
            children_[p].push_back(i);
            stack.push_back(i);
        } else if (code[i] == '}' && !stack.empty()) {
            close_[stack.back()] = i;
            stack.pop_back();
        }
    }
}

std::optional<size_t> Braces::close(size_t open, size_t limit) const {
    const auto it = close_.find(open);
    if (it == close_.end() || it->second >= limit) return std::nullopt;
    return it->second;
}

size_t Braces::parent(size_t open) const {
    const auto it = parent_.find(open);
    return it == parent_.end() ? kTop : it->second;
}

size_t Braces::innermost(size_t p) const {
    size_t o = kTop;
    for (;;) {
        size_t inner = kTop;
        for (size_t c : children(o)) {
            if (c >= p) break;
            inner = c;
        }
        if (inner == kTop) return o;
        const std::optional<size_t> end = close(inner, std::string::npos);
        if (end && *end < p) return o;
        o = inner;
    }
}

const std::vector<size_t>& Braces::children(size_t open) const {
    static const std::vector<size_t> none;
    const auto it = children_.find(open);
    return it == children_.end() ? none : it->second;
}

}  // namespace precheck
