// hydra_precheck --code-mask: a file's text with its comments and the inside
// of its string and char literals turned into spaces, every line break kept,
// so offsets and line numbers match the source. codelex (tests/code_lexer.h)
// decides what is a comment and what is a string.
#pragma once

#include <optional>
#include <string>

namespace precheck {

// The masked text of a C++ or Python file named `file` (its extension picks
// the language), or nothing when codelex reads neither language for it.
//
// A comment becomes spaces; a string keeps its prefix, its opening quote and
// its closing quote, and its inside becomes spaces. One space stands for each
// UTF-16 unit of the text it covers (one for most characters, two for one
// outside the Basic Multilingual Plane), so a reader that counts characters
// the way .NET does finds every code character at its source offset.
std::optional<std::string> code_mask(const std::string& text, const std::string& file);

}  // namespace precheck
