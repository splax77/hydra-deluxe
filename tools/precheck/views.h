// The three views of a file every check reads: its text, the text with
// comments blanked, and the text with comments and string contents blanked.
// Every view keeps each byte where it was and every line break, so one list
// of line starts serves all three. This is the one place the precheck lexes a
// file; the lexing itself is codelex's (tests/code_lexer.h).
#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "code_lexer.h"
#include "git_text.h"

namespace precheck {

struct View {
    std::string raw;          // the file's text (tip_text_form)
    std::string no_comments;  // comments blanked, strings kept
    std::string code;         // comments blanked, and the text inside each string or char literal
    std::vector<size_t> line_starts;  // the offset of each line's first character; line_starts[0] == 0

    // Which line (1-based) a byte offset in any of the three texts is on.
    // The one place the precheck turns an offset into a line.
    int line_of(size_t offset) const;

    // How many lines the file has (the script's Split("`n").Count).
    int line_count() const { return static_cast<int>(line_starts.size()); }
    // Line n (1-based) of one of the three texts, without its line break.
    std::string line(const std::string& text, int n) const;
};

// The views of `text`, lexed as `lang`.
View make_view(const std::string& text, codelex::Lang lang);

// Which lexer a file's views use: Python for is_py files, C++ for every
// other file, as the script's Get-Views chose.
codelex::Lang view_lang(const std::string& path);

// The views of files at revisions, each built once. Files come through the
// repo's one cat-file process.
class ViewCache {
public:
    explicit ViewCache(Repo& repo) : repo_(repo) {}
    // Throws GitError when `rev` has no such file.
    const View& get(const std::string& rev, const std::string& path);

private:
    Repo& repo_;
    std::map<std::pair<std::string, std::string>, View> views_;
};

}  // namespace precheck
