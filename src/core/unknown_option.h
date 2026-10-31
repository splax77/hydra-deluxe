// The line a command-line tool prints for an option it does not know. Every
// tool calls unknown_option_line, so they all say it the same way (D271 item
// 6; the wording is D276 item 5). It lives in core because hydra_precheck
// links only hydra_core.

#ifndef HYDRA_CORE_UNKNOWN_OPTION_H
#define HYDRA_CORE_UNKNOWN_OPTION_H

#include <string>

namespace hydra {

// The line for `option`, as the user typed it, with no line break after it.
inline std::string unknown_option_line(const std::string& option) {
    return "Unknown option: " + option;
}

}  // namespace hydra

#endif  // HYDRA_CORE_UNKNOWN_OPTION_H
