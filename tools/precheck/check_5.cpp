// Check 5: second homes in src/ (kind F). A type, a free function or a long
// string literal the range writes in one src/ file when another src/ file
// already has it, and a record whose field names mostly repeat another's.
#include <algorithm>
#include <cctype>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "braces.h"
#include "checks.h"
#include "code_lexer.h"
#include "definitions.h"
#include "records.h"
#include "views.h"

namespace precheck {

namespace {

// How big a repeat must be to count. These are the user's decision D113
// item 8 in docs/audit/2026-10-03-fix-decisions.md, starting values the
// first real runs may tune. This is the one place they live.
constexpr size_t kMinRepeatedString = 12;  // characters in a string literal, as written between its quotes
constexpr size_t kMinRecordFields = 3;     // fields a struct or class needs before it is compared
constexpr size_t kMinSharedFields = 4;     // field names two records share before they count as one idea

const char* const kWhy =
    "a second home for an idea that already has one; name the concept it extends, or reuse it (derive-once "
    "review, step 8)";

std::string fold(const std::string& s) {
    std::string out = s;
    for (char& c : out) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return out;
}

struct Home {
    std::string file;
    int line = 0;
};

struct Record {
    std::string file;
    int line = 0;
    std::string kind, name;
    std::vector<std::string> fields;     // in declaration order
    std::map<std::string, int> line_of;  // folded name -> line
};

// Every other home of one thing: "file:line, file:line", the first line in
// each file, files in the order the script's Group-Object gave them.
std::string other_homes(const std::vector<Home>& homes, const std::string& file) {
    std::vector<std::string> files;
    std::map<std::string, int> first;
    for (const Home& h : homes) {
        if (h.file == file || first.count(h.file)) continue;
        first[h.file] = h.line;
        files.push_back(h.file);
    }
    std::stable_sort(files.begin(), files.end(), text_before);
    std::string out;
    for (const std::string& f : files) {
        if (!out.empty()) out += ", ";
        out += f + ":" + std::to_string(first[f]);
    }
    return out;
}

size_t file_count(const std::vector<Home>& homes) {
    std::vector<std::string> seen;
    for (const Home& h : homes)
        if (std::find(seen.begin(), seen.end(), h.file) == seen.end()) seen.push_back(h.file);
    return seen.size();
}

bool include_line(const std::string& code_line) {
    size_t i = 0;
    auto skip = [&] {
        while (i < code_line.size() && std::isspace(static_cast<unsigned char>(code_line[i]))) ++i;
    };
    skip();
    if (i >= code_line.size() || code_line[i] != '#') return false;
    ++i;
    skip();
    static const std::string kWord = "include";
    if (code_line.size() - i < kWord.size()) return false;
    for (size_t k = 0; k < kWord.size(); ++k)
        if (std::tolower(static_cast<unsigned char>(code_line[i + k])) != kWord[k]) return false;
    const size_t after = i + kWord.size();
    return after == code_line.size() ||
           !(std::isalnum(static_cast<unsigned char>(code_line[after])) || code_line[after] == '_');
}

// The plain string literals ("...", with any u8/L/u/U prefix outside the
// piece) in one codelex String piece: raw strings and character literals
// are not among them. Gives [open quote, one past the close quote) each.
std::vector<std::pair<size_t, size_t>> quoted_literals(const std::string& t, size_t b, size_t e) {
    std::vector<std::pair<size_t, size_t>> out;
    size_t pos = b;
    while (pos < e && t[pos] == '"') {
        size_t i = pos + 1;
        bool closed = false;
        while (i < e) {
            if (t[i] == '\\' && i + 1 < e) {
                i += 2;
                continue;
            }
            if (t[i] == '"') {
                closed = true;
                ++i;
                break;
            }
            if (t[i] == '\n') break;
            ++i;
        }
        if (!closed) break;
        out.emplace_back(pos, i);
        pos = i;
    }
    return out;
}

}  // namespace

void run_check_5(Context& ctx) {
    // The audit that asked for this check covers src/ only (D113 item 1), so
    // both sides of every comparison are src/ C++ files at the range's tip.
    std::vector<std::string> src_files;
    for (const std::string& f : ctx.all_files)
        if (top_folder(f) == "src" && is_cpp(f)) src_files.push_back(f);
    if (std::none_of(src_files.begin(), src_files.end(), [&](const std::string& f) { return ctx.added.has_file(f); }))
        return;

    ViewCache views(ctx.repo);
    struct Type {
        Home home;
        std::string kind, name;
    };
    std::vector<Type> types;
    std::vector<Record> records;
    std::vector<std::pair<std::string, Home>> funcs;  // name, home
    std::vector<std::pair<std::string, Home>> texts;  // text, home
    for (const std::string& f : src_files) {
        const View& v = views.get(ctx.range.tip, f);
        const Braces braces(v.code);
        for (const TypeDefinition& d : type_definitions(v.code)) {
            const Home home{f, v.line_of(d.name_at)};
            types.push_back({home, d.kind, d.name});
            if (d.kind != "struct" && d.kind != "class") continue;
            const std::optional<size_t> close = braces.close(d.open, v.code.size());
            if (!close) continue;
            const std::vector<RecordField> fields = record_fields(v.code, braces, d.open, *close);
            if (fields.size() < kMinRecordFields) continue;
            Record r{f, home.line, d.kind, d.name, {}, {}};
            for (const RecordField& fl : fields) {
                r.fields.push_back(fl.name);
                r.line_of[fold(fl.name)] = v.line_of(fl.at);
            }
            records.push_back(std::move(r));
        }
        if (is_cpp_implementation(f)) {
            for (const Definition& d : definitions(v, braces, f, true))
                if (!d.lambda) funcs.push_back({d.name, {f, d.line}});
        }
        // String literals as codelex reads them, so a quote inside a comment
        // is never one; raw strings and #include paths are left out.
        for (const codelex::Piece& p : codelex::split(v.raw, codelex::Lang::Cpp)) {
            if (p.kind != codelex::Kind::String) continue;
            for (const auto& [open, end] : quoted_literals(v.raw, p.begin, p.end)) {
                const std::string s = v.raw.substr(open + 1, end - open - 2);
                if (s.size() < kMinRepeatedString) continue;
                const int line = v.line_of(open);
                if (include_line(v.line(v.code, line))) continue;
                texts.push_back({s, {f, line}});
            }
        }
    }

    {
        std::map<std::string, std::vector<const Type*>> by_name;
        for (const Type& t : types) by_name[t.name].push_back(&t);
        for (const auto& [name, group] : by_name) {
            std::vector<Home> homes;
            for (const Type* t : group) homes.push_back(t->home);
            if (file_count(homes) < 2) continue;
            for (const Type* t : group) {
                if (!ctx.added.added(t->home.file, t->home.line)) continue;
                const std::string others = other_homes(homes, t->home.file);
                if (!others.empty())
                    ctx.items.add("F", t->home.file, t->home.line,
                                  t->kind + " " + t->name + " is also defined at " + others, kWhy);
            }
        }
    }
    for (const Record& r : records) {
        for (const Record& o : records) {
            if (o.file == r.file) continue;
            std::vector<std::string> shared;
            for (const std::string& n : r.fields)
                if (o.line_of.count(fold(n))) shared.push_back(n);
            std::stable_sort(shared.begin(), shared.end(), text_before);
            if (shared.size() < kMinSharedFields) continue;
            // The record counts when the range added its name or one of the shared fields.
            std::vector<int> at = {r.line};
            for (const std::string& n : shared) at.push_back(r.line_of.at(fold(n)));
            const auto hit = std::find_if(at.begin(), at.end(), [&](int l) { return ctx.added.added(r.file, l); });
            if (hit == at.end()) continue;
            std::string list;
            for (const std::string& n : shared) list += (list.empty() ? "" : ", ") + n;
            ctx.items.add("F", r.file, *hit,
                          r.kind + " " + r.name + " shares " + std::to_string(shared.size()) + " field names with " +
                              o.kind + " " + o.name + " at " + o.file + ":" + std::to_string(o.line) + ": " + list,
                          kWhy);
        }
    }
    {
        std::map<std::string, std::vector<Home>> by_name;
        for (const auto& [name, home] : funcs) by_name[name].push_back(home);
        for (const auto& [name, homes] : by_name) {
            if (file_count(homes) < 2) continue;
            for (const Home& h : homes) {
                if (!ctx.added.added(h.file, h.line)) continue;
                const std::string others = other_homes(homes, h.file);
                if (!others.empty())
                    ctx.items.add("F", h.file, h.line, "free function " + name + " is also defined at " + others, kWhy);
            }
        }
    }
    {
        std::map<std::string, std::vector<Home>> by_text;
        for (const auto& [text, home] : texts) by_text[text].push_back(home);
        for (const auto& [text, homes] : by_text) {
            if (file_count(homes) < 2) continue;
            // One line per file: the first place this range wrote the text there.
            std::vector<std::string> done;
            for (const Home& h : homes) {
                if (!ctx.added.added(h.file, h.line)) continue;
                if (std::find(done.begin(), done.end(), h.file) != done.end()) continue;
                done.push_back(h.file);
                ctx.items.add("F", h.file, h.line, "\"" + text + "\" is also written at " + other_homes(homes, h.file),
                              kWhy);
            }
        }
    }
}

}  // namespace precheck
