// The one splitter of a line of screen text in the tests.
//
// A test that reads a sentence the app joins with a separator splits it back
// into its pieces here, so each piece can be checked against its own source.

#ifndef HYDRA_TESTS_TEXT_SPLIT_H
#define HYDRA_TESTS_TEXT_SPLIT_H

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

// The pieces of `text` between each `sep`. A text with no `sep` is one piece,
// and a `sep` at either end leaves an empty piece there. `sep` must not be
// empty.
inline std::vector<std::string> split_on(const std::string& text, std::string_view sep) {
    std::vector<std::string> out;
    std::size_t from = 0;
    for (std::size_t at; (at = text.find(sep, from)) != std::string::npos; from = at + sep.size())
        out.push_back(text.substr(from, at - from));
    out.push_back(text.substr(from));
    return out;
}

#endif  // HYDRA_TESTS_TEXT_SPLIT_H
