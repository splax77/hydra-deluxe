// Small text steps the precheck's readers share.
#pragma once

#include <string>

#include "core/strutil.h"

namespace precheck {

// Where the run of whitespace that starts at `i` in `s` ends: the first byte
// from `i` on that is not hydra::is_ascii_space, or the end of `s`.
inline size_t skip_space(const std::string& s, size_t i) {
    while (i < s.size() && hydra::is_ascii_space(s[i])) ++i;
    return i;
}

}  // namespace precheck
