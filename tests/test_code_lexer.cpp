// The code lexer in tests/code_lexer.h: how it splits a source file into code,
// comments and string literals.
#include "doctest.h"

#include <string>
#include <vector>

#include "code_lexer.h"

namespace {

// The pieces of `text` as "C:", "#:" or "S:" (code, comment, string) plus the
// piece's text.
std::vector<std::string> lexed(const std::string& text, codelex::Lang lang) {
    std::vector<std::string> out;
    for (const codelex::Piece& p : codelex::split(text, lang)) {
        const char* tag = p.kind == codelex::Kind::Code      ? "C:"
                          : p.kind == codelex::Kind::Comment ? "#:"
                                                             : "S:";
        out.push_back(tag + text.substr(p.begin, p.end - p.begin));
    }
    return out;
}

using V = std::vector<std::string>;

}  // namespace

TEST_CASE("code lexer: C++ and Python comments and string literals") {
    const auto cpp = codelex::Lang::Cpp;
    const auto py = codelex::Lang::Python;
    // C++ line and block comments.
    CHECK(lexed("a // c\nb", cpp) == V{"C:a ", "#:// c", "C:\nb"});
    CHECK(lexed("a /* c\nd */ b", cpp) == V{"C:a ", "#:/* c\nd */", "C: b"});
    // A string with an escaped quote, and a comment marker inside a string.
    CHECK(lexed(R"(x = "q\"r" + y)", cpp) == V{"C:x = ", R"(S:"q\"r")", "C: + y"});
    CHECK(lexed(R"("// no" z)", cpp) == V{R"(S:"// no")", "C: z"});
    // A raw string holds a quote and a parenthesis without ending.
    CHECK(lexed(R"--(s = R"d(a")b)d"; t)--", cpp) ==
          V{"C:s = ", R"--(S:R"d(a")b)d")--", "C:; t"});
    // A character literal, and a digit separator that is not one.
    CHECK(lexed(R"(c = '\''; n = 1'000;)", cpp) == V{"C:c = ", R"(S:'\'')", "C:; n = 1'000;"});
    // Python line comments.
    CHECK(lexed("a # c\nb", py) == V{"C:a ", "#:# c", "C:\nb"});
    // Single and double quotes, with an escape; a prefix belongs to its string.
    CHECK(lexed(R"(x = 'q\'r' + "s")", py) == V{"C:x = ", R"(S:'q\'r')", "C: + ", R"(S:"s")"});
    CHECK(lexed("rb'x' y", py) == V{"S:rb'x'", "C: y"});
    // Triple-quoted strings span lines and hold a lone quote and a #.
    CHECK(lexed("'''a\n#b''' c", py) == V{"S:'''a\n#b'''", "C: c"});
    CHECK(lexed(R"("""a"b""" c)", py) == V{R"(S:"""a"b""")", "C: c"});
    // PowerShell: # and <# #> comments, a string that ends in a backslash, a
    // backtick-escaped quote, a doubled quote, and a here-string.
    const auto ps = codelex::Lang::PowerShell;
    CHECK(lexed("a # c\nb", ps) == V{"C:a ", "#:# c", "C:\nb"});
    CHECK(lexed("a <# c\n#d #> b", ps) == V{"C:a ", "#:<# c\n#d #>", "C: b"});
    CHECK(lexed(R"(x = 'C:\' + y # c)", ps) == V{"C:x = ", R"(S:'C:\')", "C: + y ", "#:# c"});
    CHECK(lexed("\"a`\"# b\" c", ps) == V{"S:\"a`\"# b\"", "C: c"});
    CHECK(lexed("'it''s # not' z", ps) == V{"S:'it''s # not'", "C: z"});
    CHECK(lexed("@'\na # b\n'@ c", ps) == V{"S:@'\na # b\n'@", "C: c"});
    // YAML: a # after a space is a comment and one inside a word is not; a
    // quote opens a string where a value starts, and is an apostrophe inside
    // a plain value.
    const auto yml = codelex::Lang::Yaml;
    CHECK(lexed("a: 1 # c\nb", yml) == V{"C:a: 1 ", "#:# c", "C:\nb"});
    CHECK(lexed("# c\n  run: x#y", yml) == V{"#:# c", "C:\n  run: x#y"});
    CHECK(lexed("a: 'it''s # no' z", yml) == V{"C:a: ", "S:'it''s # no'", "C: z"});
    CHECK(lexed(R"(a: "q\"# r")", yml) == V{"C:a: ", R"(S:"q\"# r")"});
    CHECK(lexed("run: echo it's # c", yml) == V{"C:run: echo it's ", "#:# c"});
    CHECK(lexed("- [x, 'y']", yml) == V{"C:- [x, ", "S:'y'", "C:]"});
}
