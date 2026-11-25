// The standing scan for totals over the corpus: docs/agents/test-rubric.md,
// "A standing scan for totals over the corpus", says what it fails and why.
// A row in tests/single_owner/rules/ reads one line, and this question needs
// the loop around the line, so it is its own case, the way special.cpp is.
// The walk and the repo root come from tests/source_tree.h, and the split of
// a file into code, comments and strings from tests/code_lexer.h.
#include "doctest.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <initializer_list>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "../code_lexer.h"
#include "../source_tree.h"
#include "core/strutil.h"
#include "core/winstr.h"
#include "listed.h"

namespace fs = std::filesystem;
using namespace single_owner;

namespace {

// One comparison the scan flags.
struct CorpusTotal {
    int line;             // 1-based
    std::string counter;  // what the loop adds to, with its spaces removed
    std::string text;     // the comparison's line, trimmed
};

// `text` with each comment and string literal blanked to spaces. Line breaks
// stay, so a position names the same line in both.
std::string code_only(const std::string& text) {
    std::string out = text;
    for (const codelex::Piece& p : codelex::split(text, codelex::Lang::Cpp)) {
        if (p.kind == codelex::Kind::Code) continue;
        for (size_t i = p.begin; i < p.end; ++i)
            if (out[i] != '\n') out[i] = ' ';
    }
    return out;
}

std::string without_space(const std::string& s) {
    std::string out;
    for (const char c : s)
        if (!hydra::is_ascii_space(c)) out += c;
    return out;
}

// One past the bracket that closes the '(' or '{' at `open`, or npos.
size_t closing(const std::string& code, size_t open) {
    const char o = code[open];
    const char c = o == '(' ? ')' : '}';
    int depth = 0;
    for (size_t i = open; i < code.size(); ++i) {
        if (code[i] == o) {
            ++depth;
        } else if (code[i] == c) {
            if (--depth == 0) return i + 1;
        }
    }
    return std::string::npos;
}

// One past the end of the statement that starts at `from`: a block's closing
// brace, or the ';' that ends a statement with no block.
size_t statement_end(const std::string& code, size_t from) {
    int depth = 0;
    for (size_t i = from; i < code.size(); ++i) {
        const char c = code[i];
        if (c == '{' && depth == 0) {
            const size_t e = closing(code, i);
            return e == std::string::npos ? code.size() : e;
        }
        if (c == '(' || c == '[') {
            ++depth;
        } else if (c == ')' || c == ']') {
            --depth;
        } else if (c == ';' && depth == 0) {
            return i + 1;
        }
    }
    return code.size();
}

// The '}' that closes the block holding `from`, or the end of the text.
size_t enclosing_block_end(const std::string& code, size_t from) {
    int depth = 0;
    for (size_t i = from; i < code.size(); ++i) {
        if (code[i] == '{') {
            ++depth;
        } else if (code[i] == '}') {
            if (depth == 0) return i;
            --depth;
        }
    }
    return code.size();
}

// Calls visit(at, line) for each line of code[begin, end), where `at` is the
// line's first position in `code`.
template <typename Visit>
void for_each_line(const std::string& code, size_t begin, size_t end, Visit&& visit) {
    for (size_t at = begin; at < end;) {
        size_t stop = code.find('\n', at);
        if (stop == std::string::npos || stop > end) stop = end;
        visit(at, code.substr(at, stop - at));
        at = stop + 1;
    }
}

// What a range-based for header loops over: the text after its one ':' that
// is not part of a '::', or empty for a header with none.
std::string range_of(const std::string& header) {
    for (size_t i = 0; i < header.size(); ++i) {
        if (header[i] != ':') continue;
        const bool before = i > 0 && header[i - 1] == ':';
        const bool after = i + 1 < header.size() && header[i + 1] == ':';
        if (before || after) continue;
        return header.substr(i + 1);
    }
    return {};
}

// A variable, or a member of one, as the code names it.
std::string name_pattern() { return R"re([A-Za-z_]\w*(?:\s*(?:\.|->)\s*[A-Za-z_]\w*)*)re"; }

// A number typed into the code: decimal, hex or floating, with its sign,
// digit separators and suffix.
std::string literal_pattern() {
    return R"re(-?\s*(?:0[xX][0-9a-fA-F']+|[0-9][0-9']*(?:\.[0-9]*)?(?:[eE][+-]?[0-9]+)?|\.[0-9]+(?:[eE][+-]?[0-9]+)?)[uUlLfFzZ]*)re";
}

std::string compare_pattern() { return "(?:==|!=|<=|>=|<|>)"; }

// Whether a literal literal_pattern matched is zero.
bool zero_literal(const std::string& lit) {
    size_t i = lit.find_first_not_of("- \t\r\n");
    if (i == std::string::npos) return false;
    const bool hex = lit.compare(i, 2, "0x") == 0 || lit.compare(i, 2, "0X") == 0;
    if (hex) i += 2;
    for (; i < lit.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(lit[i]);
        if (c == '0' || c == '\'' || (!hex && c == '.')) continue;
        if (std::isdigit(c)) return false;
        if (hex && std::isxdigit(c)) return false;
        break;  // an exponent or a suffix
    }
    return true;
}

// Whether a range-based for's range, spaces removed, is the corpus: a call
// of corpus::chart_paths(), or a variable the file set from one.
bool over_corpus(const std::string& range, const std::set<std::string>& aliases) {
    static const std::regex direct(R"re((?:::)?(?:corpus::)?chart_paths\(\))re");
    return std::regex_match(range, direct) || aliases.count(range) != 0;
}

// What code[begin, end) adds to with ++ or +=, each with its spaces removed.
std::set<std::string> counters_in(const std::string& code, size_t begin, size_t end) {
    static const std::regex prefix(R"re(\+\+\s*()re" + name_pattern() + ")");
    static const std::regex postfix("(" + name_pattern() + R"re()\s*(?:\+\+|\+=))re");
    std::set<std::string> out;
    for_each_line(code, begin, end, [&](size_t, const std::string& line) {
        if (line.find('+') == std::string::npos) return;
        for (const std::regex* r : {&prefix, &postfix})
            for (std::sregex_iterator m(line.begin(), line.end(), *r), stop; m != stop; ++m)
                out.insert(without_space((*m)[1].str()));
    });
    return out;
}

// The position of each nonzero literal in code[begin, end) that `counter` is
// compared with: counter op literal, literal op counter, or a doctest
// CHECK_EQ-style macro with the two as its arguments.
std::vector<size_t> typed_comparisons(const std::string& code, size_t begin, size_t end,
                                      const std::string& counter) {
    std::string e;
    for (const char c : counter) {
        if (c == '.') e += '\\';
        e += c;
    }
    const std::string lit = literal_pattern();
    const std::string op = compare_pattern();
    const std::regex forward(R"re((?:^|[^A-Za-z0-9_.:>]))re" + e + R"re(\s*)re" + op + R"re(\s*()re" + lit +
                             R"re()(?=\s*[),;&|?}]))re");
    const std::regex reverse(R"re((?:^|[(,&|])\s*()re" + lit + R"re()\s*)re" + op + R"re(\s*)re" + e +
                             R"re((?![A-Za-z0-9_.(]|->))re");
    const std::regex macro(R"re((?:CHECK|REQUIRE|WARN)_(?:EQ|NE|GT|GE|LT|LE)\s*\(\s*(?:)re" + e +
                           R"re(\s*,\s*()re" + lit + R"re()|()re" + lit + R"re()\s*,\s*)re" + e +
                           R"re()\s*\))re");
    const std::string head = counter.substr(0, counter.find_first_of(".-"));
    std::vector<size_t> out;
    for_each_line(code, begin, end, [&](size_t at, const std::string& line) {
        if (line.find(head) == std::string::npos) return;
        for (const std::regex* r : {&forward, &reverse, &macro})
            for (std::sregex_iterator m(line.begin(), line.end(), *r), stop; m != stop; ++m) {
                const size_t g = (*m)[1].matched ? 1 : 2;
                if (!zero_literal((*m)[g].str())) out.push_back(at + static_cast<size_t>(m->position(g)));
            }
    });
    return out;
}

// The flagged comparison at position `pos` of `text`.
CorpusTotal total_at(const std::string& text, size_t pos, const std::string& counter) {
    const size_t newline = text.rfind('\n', pos);
    const size_t from = newline == std::string::npos ? 0 : newline + 1;
    size_t to = text.find('\n', pos);
    if (to == std::string::npos) to = text.size();
    const int line =
        1 + static_cast<int>(std::count(text.begin(), text.begin() + static_cast<std::ptrdiff_t>(from), '\n'));
    return {line, counter, hydra::trim(text.substr(from, to - from))};
}

// The scan of one C++ file's text: each place after a range-based for over
// the corpus where the code compares what the loop added to with a typed
// number other than 0. The loop's header must open on the line of its "for",
// and the comparison must sit on one line, in the block that holds the loop.
std::vector<CorpusTotal> corpus_totals(const std::string& text) {
    static const std::regex alias(
        R"re(\b([A-Za-z_]\w*)\s*=\s*(?:::\s*)?(?:corpus\s*::\s*)?chart_paths\s*\(\s*\))re");
    static const std::regex loop(R"re(\bfor\s*\()re");
    const std::string code = code_only(text);
    std::set<std::string> aliases;
    for_each_line(code, 0, code.size(), [&](size_t, const std::string& line) {
        if (line.find("chart_paths") == std::string::npos) return;
        for (std::sregex_iterator m(line.begin(), line.end(), alias), stop; m != stop; ++m)
            aliases.insert((*m)[1].str());
    });
    std::vector<CorpusTotal> out;
    std::set<std::pair<int, std::string>> seen;
    for_each_line(code, 0, code.size(), [&](size_t at, const std::string& line) {
        if (line.find("for") == std::string::npos) return;
        for (std::sregex_iterator m(line.begin(), line.end(), loop), stop; m != stop; ++m) {
            const size_t open = at + static_cast<size_t>(m->position(0) + m->length(0)) - 1;
            const size_t close = closing(code, open);
            if (close == std::string::npos) continue;
            const std::string header = code.substr(open + 1, close - open - 2);
            if (!over_corpus(without_space(range_of(header)), aliases)) continue;
            const size_t body_end = statement_end(code, close);
            const size_t after_end = enclosing_block_end(code, body_end);
            for (const std::string& counter : counters_in(code, close, body_end))
                for (const size_t pos : typed_comparisons(code, body_end, after_end, counter)) {
                    CorpusTotal t = total_at(text, pos, counter);
                    if (seen.insert({t.line, counter}).second) out.push_back(std::move(t));
                }
        }
    });
    return out;
}

}  // namespace

TEST_CASE("single-owner: the corpus-totals scan flags a typed total, not a 0 or a check in the loop") {
    // A counter added to over the corpus and compared with a typed number.
    const std::string typed = R"src(int notes = 0;
for (const std::string& path : corpus::chart_paths()) {
    notes += count_notes(path);
}
CHECK(notes == 18734);
)src";
    const std::vector<CorpusTotal> flagged = corpus_totals(typed);
    REQUIRE(flagged.size() == 1);
    CHECK(flagged[0].line == 5);
    CHECK(flagged[0].counter == "notes");
    CHECK(flagged[0].text == "CHECK(notes == 18734);");

    // The same kind of counter compared with 0.
    const std::string zero = R"src(int missing = 0;
for (const std::string& path : corpus::chart_paths())
    if (count_notes(path) == 0) ++missing;
CHECK(missing == 0);
)src";
    CHECK(corpus_totals(zero).empty());

    // A property every chart must meet, checked inside the loop, including a
    // per-chart count compared with a typed number there.
    const std::string property = R"src(for (const std::string& path : corpus::chart_paths()) {
    int open = 0;
    for (const Note& n : notes_of(path))
        if (n.open) ++open;
    CHECK(open <= 1);
    CHECK(count_notes(path) > 0);
}
)src";
    CHECK(corpus_totals(property).empty());
}

TEST_CASE("single-owner: no test compares a total over the corpus with a typed number") {
    const fs::path root = sourcetree::root();
    std::vector<std::string> problems;
    std::set<std::string> scanned;
    sourcetree::for_each_source_file([&](const fs::path& path, const std::string& rel) {
        if (!hydra::starts_with(rel, "tests/") || !is_cpp_source(path)) return;
        scanned.insert(rel);
        for (const CorpusTotal& t : corpus_totals(hydra::read_file_text(path.u8string())))
            problems.push_back(rel + ":" + std::to_string(t.line) + ": compares the corpus total \"" + t.counter +
                               "\" with a typed number; check a property every chart meets instead "
                               "(docs/agents/test-rubric.md, snapshot): " +
                               t.text);
    });
    // The scan read every C++ file git lists under tests/ that is on disk.
    for (const std::string& rel : sourcetree::git_listed(root, "tests")) {
        if (!is_cpp_source(fs::u8path(rel)) || !fs::is_regular_file(root / fs::u8path(rel))) continue;
        INFO("the corpus-totals scan never read " << rel);
        CHECK(scanned.count(rel) == 1);
    }
    std::ostringstream report;
    for (const std::string& p : problems) report << p << "\n";
    INFO(report.str());
    CHECK(problems.empty());
}
