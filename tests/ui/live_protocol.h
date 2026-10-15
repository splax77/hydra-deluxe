// The live channel's wire lines (uitest_live.cpp). Each reply is one JSON
// object on one line, ending in '\n', so the reader never has to guess where
// an answer ends. The C++ side only writes JSON, never parses it, so a string
// escaper is all it needs. No engine dependency: hydra_tests includes this
// file to pin the lines (tests/test_live_protocol.cpp).

#ifndef HYDRA_TESTS_UI_LIVE_PROTOCOL_H
#define HYDRA_TESTS_UI_LIVE_PROTOCOL_H

#include <cstdio>
#include <string>
#include <string_view>

namespace live {

// `s` as a quoted JSON string. The quote, the backslash and every control
// character are escaped; other bytes, UTF-8 included, pass through as they are.
inline std::string json_string(std::string_view s) {
    std::string out;
    out.reserve(s.size() + 2);
    out += '"';
    for (const char ch : s) {
        const unsigned char c = static_cast<unsigned char>(ch);
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += ch;
                }
        }
    }
    out += '"';
    return out;
}

// The reply to a command that worked: what it produced, maybe empty.
inline std::string ok_line(std::string_view text) {
    return "{\"ok\":true,\"text\":" + json_string(text) + "}\n";
}

// The reply to a command that failed: a one-line reason, and the engine log
// written while the command ran.
inline std::string fail_line(std::string_view error, std::string_view log) {
    return "{\"ok\":false,\"error\":" + json_string(error) + ",\"log\":" + json_string(log) +
           "}\n";
}

// The first line on a new connection. mode is "headless" or "window";
// scratch is the engine's scratch folder; timeout is the session's starting
// cap on a wait, in seconds (its owner is uitest::CommandSession).
inline std::string hello_line(std::string_view mode, std::string_view scratch, unsigned long pid,
                              double timeout) {
    char nums[64];
    std::snprintf(nums, sizeof(nums), ",\"pid\":%lu,\"timeout\":%g}}\n", pid, timeout);
    return "{\"ok\":true,\"hello\":{\"mode\":" + json_string(mode) +
           ",\"scratch\":" + json_string(scratch) + nums;
}

}  // namespace live

#endif  // HYDRA_TESTS_UI_LIVE_PROTOCOL_H
