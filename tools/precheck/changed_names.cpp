#include "changed_names.h"

#include <algorithm>
#include <regex>
#include <unordered_map>

#include "core/strutil.h"
#include "definitions.h"

namespace precheck {

const char* kind_word(NameKind kind) {
    switch (kind) {
    case NameKind::Body: return "body";
    case NameKind::Declared: return "declared";
    case NameKind::Value: return "value";
    case NameKind::Comment: return "comment";
    }
    return "body";
}

namespace {

bool space_char(char c) { return hydra::is_ascii_space(c); }

using hydra::trim;

// A head or declaration's tokens: words, numbers, the two-character
// operators a declaration can hold, and any other one character.
std::vector<std::string> tokens_of(const std::string& t) {
    static const char* const kPairs[] = {"::", "->", "&&", "==", "!=", "<=", ">="};
    std::vector<std::string> out;
    size_t i = 0;
    while (i < t.size()) {
        if (space_char(t[i])) {
            ++i;
            continue;
        }
        size_t j = i + 1;
        if (word_char(t[i])) {
            while (j < t.size() && word_char(t[j])) ++j;
        } else {
            for (const char* p : kPairs)
                if (t.compare(i, 2, p) == 0) j = i + 2;
        }
        out.push_back(t.substr(i, j - i));
        i = j;
    }
    return out;
}

bool is_ident(const std::string& tok) { return !tok.empty() && ident_start(tok[0]) && !is_cpp_keyword(tok); }

// The text before a "{" or ";" with what never names it removed:
// preprocessor lines, access labels, attributes and any leading
// template<...> heads.
std::string strip_head(const std::string& raw) {
    std::string t;
    size_t i = 0;
    while (i <= raw.size()) {
        size_t nl = raw.find('\n', i);
        if (nl == std::string::npos) nl = raw.size();
        const std::string line = raw.substr(i, nl - i);
        if (trim(line).rfind('#', 0) != 0) t += line + ' ';
        i = nl + 1;
    }
    static const std::regex labels(R"(\b(?:public|private|protected)\s*:(?!:))");
    t = std::regex_replace(t, labels, " ");
    for (size_t a = t.find("[["); a != std::string::npos; a = t.find("[[")) {
        const size_t z = t.find("]]", a + 2);
        if (z == std::string::npos) break;
        t.erase(a, z + 2 - a);
    }
    t = trim(t);
    while (t.rfind("template", 0) == 0 && (t.size() == 8 || !word_char(t[8]))) {
        const size_t lt = t.find('<');
        if (lt == std::string::npos) break;
        int depth = 0;
        size_t k = lt;
        for (; k < t.size(); ++k) {
            if (t[k] == '<') ++depth;
            if (t[k] == '>' && --depth == 0) break;
        }
        if (k >= t.size()) break;
        t = trim(t.substr(k + 1));
    }
    return t;
}

enum class HeadKind { Namespace, Type, Enum, Function, Constant, Other };

struct Head {
    HeadKind kind = HeadKind::Other;
    std::string name;  // empty for an anonymous type and for Namespace or Other
};

// The index of the ")" that closes the "(" at `open`, or npos.
size_t close_paren(const std::vector<std::string>& toks, size_t open) {
    int depth = 0;
    for (size_t k = open; k < toks.size(); ++k) {
        if (toks[k] == "(") ++depth;
        if (toks[k] == ")" && --depth == 0) return k;
    }
    return std::string::npos;
}

// class/struct/union/enum, then the type's name: the last word before a
// base list's ":" (or the end), skipping template arguments, alignas(...)
// and "final". A "(" outside those means a function returning the type.
bool read_type(const std::vector<std::string>& toks, Head& h) {
    size_t k = 0;
    while (k < toks.size() && (toks[k] == "typedef" || toks[k] == "friend")) ++k;
    if (k >= toks.size()) return false;
    const std::string& kw = toks[k];
    if (kw != "class" && kw != "struct" && kw != "union" && kw != "enum") return false;
    h.kind = kw == "enum" ? HeadKind::Enum : HeadKind::Type;
    ++k;
    if (h.kind == HeadKind::Enum && k < toks.size() && (toks[k] == "class" || toks[k] == "struct")) ++k;
    std::string name;
    for (; k < toks.size(); ++k) {
        const std::string& t = toks[k];
        if (t == ":") break;
        if (t == "alignas" && k + 1 < toks.size() && toks[k + 1] == "(") {
            k = close_paren(toks, k + 1);
            if (k == std::string::npos) return false;
            continue;
        }
        if (t == "<") {
            int depth = 0;
            for (; k < toks.size(); ++k) {
                if (toks[k] == "<") ++depth;
                if (toks[k] == ">" && --depth == 0) break;
            }
            continue;
        }
        if (t == "(") return false;
        if (t != "final" && is_ident(t)) name = t;
    }
    h.name = name;
    return true;
}

// A constant: const, constexpr or static before a name followed by "=" (a
// declaration) or by the head's "{". Nothing before the name holds a "(".
bool read_constant(const std::vector<std::string>& toks, bool head, Head& h) {
    size_t end = toks.size();
    int depth = 0;
    for (size_t k = 0; k < toks.size(); ++k) {
        if (toks[k] == "(" || toks[k] == "[") ++depth;
        if (toks[k] == ")" || toks[k] == "]") --depth;
        if (depth == 0 && toks[k] == "=") {
            end = k;
            break;
        }
    }
    if (end == toks.size() && !head) return false;
    if (end != toks.size() && head && end + 1 != toks.size()) return false;
    bool marked = false;
    for (size_t k = 0; k < end; ++k) {
        if (toks[k] == "(") return false;
        if (toks[k] == "const" || toks[k] == "constexpr" || toks[k] == "static") marked = true;
    }
    if (!marked) return false;
    // A trailing array bound ("kTable[]", "kTable[3]") belongs to the name.
    size_t last = end;
    while (last > 0 && toks[last - 1] == "]") {
        while (last > 0 && toks[last - 1] != "[") --last;
        if (last > 0) --last;
    }
    if (last == 0 || !is_ident(toks[last - 1])) return false;
    h.kind = HeadKind::Constant;
    h.name = toks[last - 1];
    return true;
}

// A function: a name, its "(...)", then only what may follow a signature
// (cv and ref qualifiers, noexcept, override, final, a trailing return
// type, a constructor's initialiser list, or a "= ..." specifier). The name
// is the word before the first "(", so any Class:: before it is dropped;
// a keyword there (if, for, while, switch, catch) is a statement's block.
bool read_function(const std::vector<std::string>& toks, Head& h) {
    size_t open = 0;
    for (; open < toks.size(); ++open) {
        const std::string& t = toks[open];
        if (t == "(") break;
        if (t == "=" || t == "." || t == "->") return false;
    }
    if (open == 0 || open >= toks.size() || !is_ident(toks[open - 1])) return false;
    const size_t close = close_paren(toks, open);
    if (close == std::string::npos) return false;
    for (size_t k = close + 1; k < toks.size(); ++k) {
        const std::string& t = toks[k];
        // A trailing return type, an initialiser list, or a pure, defaulted
        // or deleted function's "= ...": the rest is not read.
        if (t == "->" || t == ":" || t == "=") break;
        if (t == "noexcept" && k + 1 < toks.size() && toks[k + 1] == "(") {
            k = close_paren(toks, k + 1);
            if (k == std::string::npos) return false;
            continue;
        }
        if (t != "const" && t != "volatile" && t != "noexcept" && t != "override" && t != "final" && t != "&" &&
            t != "&&" && t != "mutable")
            return false;
    }
    h.kind = HeadKind::Function;
    h.name = toks[open - 1];
    return true;
}

// What the text before a "{" (head) or a ";" declares.
Head classify(const std::string& raw, bool head) {
    const std::string t = strip_head(raw);
    Head h;
    if (is_namespace_head(t)) {
        h.kind = HeadKind::Namespace;
        return h;
    }
    const std::vector<std::string> toks = tokens_of(t);
    if (read_type(toks, h) || read_constant(toks, head, h) || read_function(toks, h)) return h;
    return Head{};
}

class Finder {
public:
    Finder(const View& view, const Braces& braces) : view_(view), code_(view.code), braces_(braces) {}

    std::vector<ChangedName> run(const std::set<int>& lines) {
        for (int n : lines) {
            if (n < 1 || n > view_.line_count()) continue;
            const size_t p = first_code(n);
            if (p == std::string::npos) {
                // A comment or blank line: the declaration below it.
                const size_t at = view_.line_starts[n - 1];
                for (int m = n + 1; m <= view_.line_count(); ++m) {
                    const size_t pm = first_code(m);
                    if (pm == std::string::npos) continue;
                    declared(pm, m, true);
                    break;
                }
                enclosing(at);
            } else {
                declared(p, n, false);
                enclosing(p);
            }
        }
        return out_;
    }

private:
    const View& view_;
    const std::string& code_;
    const Braces& braces_;
    std::vector<ChangedName> out_;
    std::unordered_map<std::string, size_t> index_;
    std::unordered_map<size_t, Head> heads_;

    void add(const std::string& name, NameKind kind) {
        if (name.empty()) return;
        const auto it = index_.find(name);
        if (it != index_.end()) {
            out_[it->second].kind = std::min(out_[it->second].kind, kind);
            return;
        }
        index_[name] = out_.size();
        out_.push_back({name, kind});
    }

    // The offset of line n's first non-blank code character, or npos when
    // the line masks to blank.
    size_t first_code(int n) const {
        const size_t b = view_.line_starts[n - 1];
        const size_t e = n < view_.line_count() ? view_.line_starts[n] - 1 : code_.size();
        for (size_t i = b; i < e; ++i)
            if (!space_char(code_[i])) return i;
        return std::string::npos;
    }

    // The text between the last ";", "{" or "}" before `at` and `at`. A
    // braced group the declaration itself holds (a "b_{2}" initialiser, a
    // "= {}" default argument) is read through: a "}" followed by nothing,
    // a "," or a ")" before `at` does not end the text.
    std::string text_before(size_t at) const {
        size_t end = at;
        for (;;) {
            const size_t prev = end == 0 ? std::string::npos : code_.find_last_of(";{}", end - 1);
            if (prev == std::string::npos) return code_.substr(0, at);
            if (code_[prev] == '}') {
                const std::string after = trim(code_.substr(prev + 1, end - prev - 1));
                const size_t open = innermost(prev);
                if (open != Braces::kTop && (after.empty() || after[0] == ',' || after[0] == ')')) {
                    end = open;
                    continue;
                }
            }
            return code_.substr(prev + 1, at - prev - 1);
        }
    }

    const Head& head_of(size_t open) {
        auto it = heads_.find(open);
        if (it == heads_.end()) it = heads_.emplace(open, classify(text_before(open), true)).first;
        return it->second;
    }

    // The innermost "{" whose block holds offset p (its own "}" included),
    // or kTop.
    size_t innermost(size_t p) const {
        size_t o = Braces::kTop;
        for (;;) {
            size_t inner = Braces::kTop;
            for (size_t c : braces_.children(o)) {
                if (c >= p) break;
                inner = c;
            }
            if (inner == Braces::kTop) return o;
            const std::optional<size_t> close = braces_.close(inner, std::string::npos);
            if (close && *close < p) return o;
            o = inner;
        }
    }

    // Rule 1: the name of every enclosing block's head, up to the first
    // namespace.
    void enclosing(size_t p) {
        for (size_t o = innermost(p); o != Braces::kTop; o = braces_.parent(o)) {
            const Head& h = head_of(o);
            if (h.kind == HeadKind::Namespace) return;
            add(h.name, h.kind == HeadKind::Constant ? NameKind::Value : NameKind::Body);
        }
    }

    // Rule 2 for an enum: the word that starts each comma-separated entry of
    // `text` gives Enum::Value.
    void add_enumerators(const std::string& text, const std::string& enum_name, bool comment) {
        int depth = 0;
        bool at_start = true;
        for (const std::string& t : tokens_of(text)) {
            if (t == "(" || t == "{" || t == "<") ++depth;
            if (t == ")" || t == "}" || t == ">") --depth;
            if (depth == 0 && t == ",") {
                at_start = true;
                continue;
            }
            if (at_start && depth == 0 && is_ident(t))
                add(enum_name.empty() ? t : enum_name + "::" + t, comment ? NameKind::Comment : NameKind::Value);
            at_start = false;
        }
    }

    // Rule 2: what line n (first code at p) itself declares. `comment` marks
    // the names as reached through a comment line above it (rule 3).
    void declared(size_t p, int n, bool comment) {
        const size_t o = innermost(p);
        const HeadKind ctx = o == Braces::kTop ? HeadKind::Namespace : head_of(o).kind;
        if (ctx == HeadKind::Enum) {
            add_enumerators(view_.line(code_, n), head_of(o).name, comment);
            return;
        }
        const size_t q = code_.find_first_of(";{}", p);
        if (q == std::string::npos || code_[q] == '}') return;
        const bool head = code_[q] == '{';
        const bool body = ctx != HeadKind::Namespace && ctx != HeadKind::Type;
        const Head h = classify(text_before(q), head);
        if (body && h.kind != HeadKind::Type && h.kind != HeadKind::Enum) return;
        if (h.kind == HeadKind::Namespace || h.kind == HeadKind::Other) return;
        add(h.name, comment ? NameKind::Comment : h.kind == HeadKind::Constant ? NameKind::Value : NameKind::Declared);
        if (h.kind == HeadKind::Enum && head) {
            // Enumerators after the "{" on this same line (a one-line enum).
            const size_t line_end = n < view_.line_count() ? view_.line_starts[n] : code_.size();
            const std::optional<size_t> close = braces_.close(q, code_.size());
            const size_t stop = std::min(close.value_or(line_end), line_end);
            if (stop > q + 1) add_enumerators(code_.substr(q + 1, stop - q - 1), h.name, comment);
        }
    }
};

}  // namespace

std::vector<ChangedName> changed_names(const View& view, const Braces& braces, const std::set<int>& lines) {
    return Finder(view, braces).run(lines);
}

}  // namespace precheck
