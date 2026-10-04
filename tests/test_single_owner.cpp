// One owner per rule, checked where a grep can check it. Each row names a
// question, the one place that answers it, and a pattern for the text that
// answers it. A matching line anywhere else fails here, unless it calls the
// owner on that same line, or the baseline lists it with the fix that will
// remove it. A baseline entry that no longer matches also fails, so the list
// only shrinks.
//
// This is the one scan of src/ and tools/. The long-path rules (ADR 0020) are
// rows here too, so a new one-place rule is a new row, not a new walker.
#include "doctest.h"

#include <filesystem>
#include <fstream>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "core/strutil.h"

#ifndef HYDRA_SOURCE_DIR
#error "HYDRA_SOURCE_DIR must be defined (see CMakeLists.txt)"
#endif

namespace fs = std::filesystem;

namespace {

// A file whose matches answer a different question than the rule's.
struct Exempt {
    std::string file;  // repo-relative, forward slashes
    std::string why;
};

// One line inside the owner that answers the question. A rule that lists its
// owner's lines instead of naming the whole file guards the owner too: a
// second answer added there fails like one anywhere else.
struct OwnerLine {
    std::string file;       // repo-relative, forward slashes
    std::string line_text;  // the line with leading and trailing space trimmed
};

struct OwnerRule {
    std::string question;                  // plain English, shown on failure
    std::string owner;                     // the one place that answers it
    std::string pattern;                   // ECMAScript regex, matched per line
    std::string calls_owner;               // a line also matching this calls the owner: it passes
    std::vector<std::string> owner_files;  // repo-relative files that may match freely
    std::vector<Exempt> exempt;
    std::string decided_by;                // ADR, CONTEXT.md, the user's words, or the audit id
    std::vector<std::string> must_match;   // lines the rule flags
    std::vector<std::string> must_not_match;
    std::vector<OwnerLine> owner_lines;    // the owner's own lines; each must still match
};

struct KnownCopy {
    std::string question;   // equals a rule's question
    std::string file;       // repo-relative, forward slashes
    std::string line_text;  // the line with leading and trailing space trimmed
    std::string removed_by; // the fix that deletes it
};

const std::vector<OwnerRule>& rules() {
    static const std::vector<OwnerRule> r = {
        // Only length comparisons count. Sizing a buffer against MAX_PATH
        // (GetTempPathW in copy_to_short_temp) asks whether a result fitted,
        // which audit R7.12 calls a different question. The limit may be
        // spelled 260, and either side may carry a little arithmetic
        // (size() - 4 < MAX_PATH).
        {"Does the Windows shell take a path this long?",
         "shell_path in src/core/winstr.cpp",
         R"((\.(size|length)\(\)|\b(wcs|str)len\s*\([^()]*\))(\s*[-+]\s*[\w.]+)*\s*[<>]=?\s*(MAX_PATH|260)\b|\b(MAX_PATH|260)(\s*[-+]\s*[\w.]+)*\s*[<>]=?\s*[\w.:>()-]*(\.(size|length)\(\)|\b(wcs|str)len\s*\())",
         "",
         {"src/core/winstr.cpp"},
         {},
         "ADR 0020 (the shell takes nothing of 260 or more); one owner put on the fix "
         "list by the user 2026-10-03 (audit R7.12 addendum)",
         {"if (path.size() < MAX_PATH) return path;",
          "if (copy.native().size() >= MAX_PATH) return {};",
          "return s.size() < MAX_PATH ? s : L\"\";",
          "if (wcslen(p) >= MAX_PATH) return false;",
          "if (MAX_PATH > name.length()) ok = true;",
          "if (path.size() - 4 < MAX_PATH) return path;",
          "if (name.length() >= 260) return false;",
          "if (260 <= s.size()) ok = false;",
          "if (MAX_PATH - 1 > wcslen(p)) ok = true;"},
         {"wchar_t tmp[MAX_PATH + 1];", "GetTempPathW(MAX_PATH + 1, tmp);",
          "if (n == 0 || n > MAX_PATH) return {};",
          "constexpr size_t kPlainPathLimit = MAX_PATH - 12;", "const int kWidth = 260;",
          "if (n > 260) return;", "if (rows.size() > 2600) return;"}},
        // Every way this codebase works out a file's size: the stdio seek and
        // tell (32- and 64-bit), stream seekg/tellg, the Win32 size calls and
        // the size fields of their find and attribute data, the CRT's
        // filelength and fstat, std::filesystem's file_size, and a seek to the
        // end. A file already open is sized by open_file_size_bytes (a
        // stdio stream) or open_handle_size_bytes (a Win32 handle);
        // file_size_bytes and read_file_bytes both go through them. winstr.cpp
        // is not exempt as a file: only its two answering lines are listed.
        {"How many bytes does a file hold?",
         "file_size_bytes, open_file_size_bytes and open_handle_size_bytes in "
         "src/core/winstr.cpp",
         R"((^|[^\w])(std::)?(_?f(tell|seek)(i64|o)?|GetFileSize(Ex)?|GetCompressedFileSize[AW]?|GetFileInformationByHandle(Ex)?|_?filelength(i64)?|_?fstat(64|i64)?|(tell|seek)g|file_size)\s*\(|\b(nFileSize(High|Low)|st_size|SEEK_END|FILE_END)\b)",
         "",
         {},
         {},
         "no ADR records it. Audit R7.11 names read_file_bytes the owner (fix "
         "read-file-bytes-owner); its 64-bit size came from the user's \"make all of "
         "those fixes\" (2026-10-03, preview-loading-fixes plan); the open-file "
         "helpers are the user's decision D39 (2026-10-04, audit R7.25)",
         {"long n = std::ftell(f);", "std::fseek(f, 0, SEEK_END);",
          "const long long size = _ftelli64(f);", "if (!GetFileSizeEx(file, &size)) return;",
          "e.size = (static_cast<uint64_t>(fd.nFileSizeHigh) << 32) | fd.nFileSizeLow;",
          "const auto n = fs::file_size(hydra::os_path(p));", "in.seekg(0, std::ios::end);",
          "return st.st_size;"},
         {"const uint64_t n = hydra::file_size_bytes(path);",
          "const std::vector<uint8_t> bytes = hydra::read_file_bytes(path);",
          "MA_DR_MP3_SEEK_END,", "const size_t n = bytes.size();",
          "return hydra::open_file_size_bytes(f).value_or(static_cast<ImU64>(-1));",
          "const std::optional<uint64_t> size = hydra::open_handle_size_bytes(file);"},
         // open_handle_size_bytes, which every other size helper calls, and
         // list_dir's size straight from the find data it already has (the
         // rescan cache's fingerprint; sizing each entry by handle would open
         // every file in the library).
         {{"src/core/winstr.cpp",
           "if (!GetFileSizeEx(h, &size) || size.QuadPart < 0) return std::nullopt;"},
          {"src/core/winstr.cpp",
           "e.size = (static_cast<uint64_t>(fd.nFileSizeHigh) << 32) | fd.nFileSizeLow;"}}},
        {"How is a long path prefixed for Win32?",
         "win32_path in src/core/winstr.cpp",
         R"(\\\\\\\\\?\\\\)",
         "",
         {"src/core/winstr.cpp"},
         {},
         "ADR 0020",
         {"return L\"\\\\\\\\?\\\\\" + full;"},
         {"return L\"\\\\\\\\\" + s.substr(8);"}},
        // utf8_to_wide is for text; a path takes win32_path.
        {"How does a path become a wide string for Windows?",
         "win32_path and os_path in src/core/winstr.cpp",
         R"(utf8_to_wide\()",
         "",
         {"src/core/winstr.cpp", "src/core/winstr.h"},
         {{"src/net/dmbot_client.cpp", "it converts a URL, not a path"}},
         "ADR 0020",
         {"std::wstring w = hydra::utf8_to_wide(path);",
          "const std::wstring p = win32_path(utf8_to_wide(s));"},
         {"std::wstring w = hydra::win32_path(path);"}},
        {"How does a raw Windows file call get a path it accepts at any length?",
         "win32_path in src/core/winstr.cpp",
         R"(FindFirstFile|CreateFileW|CreateFileA|fopen\(|_wfreopen|_wopen|_wstat|GetFileAttributes|DeleteFile|MoveFile|CopyFile|CreateDirectory|RemoveDirectory|sqlite3_open)",
         R"(win32_path\()",
         {"src/core/winstr.cpp", "src/core/winstr.h"},
         {},
         "ADR 0020",
         {"HANDLE h = CreateFileW(w.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);",
          "FILE* f = _wfopen(w.c_str(), L\"rb\");"},
         {"HANDLE h = CreateFileW(win32_path(p).c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);",
          "std::FILE* f = hydra::fopen_utf8(path, L\"rb\");"}},
        // The shell takes no long path, so only the two report buttons launch
        // it, each with shell_path's short form.
        {"Which code launches the Windows shell?",
         "open_in_browser (src/app/report_files.cpp) and show_in_folder "
         "(src/ui/win32_dialogs.cpp)",
         R"(ShellExecute|CreateProcess)",
         "",
         {"src/app/report_files.cpp", "src/ui/win32_dialogs.cpp"},
         {},
         "ADR 0020 (The shell: Open report and Show in folder)",
         {"ShellExecuteW(nullptr, L\"open\", p.c_str(), nullptr, nullptr, SW_SHOWNORMAL);",
          "CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);"},
         {"hydra::app::open_in_browser(page);"}},
        // A std::filesystem call (a line naming filesystem:: or fs:: that
        // calls one of these) or a file stream. No file is exempt, winstr
        // included.
        {"How does a std::filesystem call or file stream get a path it accepts at any length?",
         "os_path in src/core/winstr.cpp",
         R"(^(?=.*(filesystem::|fs::)).*::(create_directories|create_directory|is_directory|is_regular_file|exists|remove|remove_all|rename|copy|copy_file|file_size|last_write_time|weakly_canonical|canonical|directory_iterator)\(|^(?!.*#include).*fstream )",
         R"(os_path\()",
         {},
         {},
         "ADR 0020",
         {"fs::create_directories(dir);", "std::ifstream in(path);",
          "for (const auto& e : std::filesystem::directory_iterator(dir)) {"},
         {"fs::create_directories(hydra::os_path(dir));", "#include <fstream>",
          "std::ifstream in(hydra::os_path(path));", "const fs::path p = dir / name;"}},
    };
    return r;
}

const std::vector<KnownCopy>& known_copies() {
    static const std::vector<KnownCopy> k = {
        {"Does the Windows shell take a path this long?", "src/app/report_files.cpp",
         "if (copy.native().size() >= MAX_PATH) return {};",
         "fix shell-path-owner (audit R7.12)"},
        {"Does the Windows shell take a path this long?", "src/app/report_files.cpp",
         "if (path.size() < MAX_PATH) return shell_open(path);",
         "fix shell-path-owner (audit R7.12)"},
        {"How many bytes does a file hold?", "src/parse/midi.cpp",
         "std::fseek(f, 0, SEEK_END);", "fix read-file-bytes-owner (audit R7.11)"},
        {"How many bytes does a file hold?", "src/parse/midi.cpp",
         "long n = std::ftell(f);", "fix read-file-bytes-owner (audit R7.11)"},
        {"How many bytes does a file hold?", "src/parse/midi.cpp",
         "std::fseek(f, 0, SEEK_SET);", "fix read-file-bytes-owner (audit R7.11)"},
    };
    return k;
}

struct CompiledRule {
    const OwnerRule* rule;
    std::regex pattern;
    std::regex calls_owner;
};

std::vector<CompiledRule> compile_rules() {
    const auto flags = std::regex::ECMAScript | std::regex::optimize;
    std::vector<CompiledRule> out;
    for (const OwnerRule& r : rules())
        out.push_back({&r, std::regex(r.pattern, flags),
                       std::regex(r.calls_owner.empty() ? "$^" : r.calls_owner, flags)});
    return out;
}

// The one verdict: does this line answer the rule's question without calling
// its owner? The self-test and the scan both ask it.
bool flags_line(const CompiledRule& c, const std::string& line) {
    if (!std::regex_search(line, c.pattern)) return false;
    return c.rule->calls_owner.empty() || !std::regex_search(line, c.calls_owner);
}

}  // namespace

TEST_CASE("single-owner rules match their own examples") {
    for (const CompiledRule& c : compile_rules()) {
        for (const std::string& line : c.rule->must_match) {
            INFO(c.rule->question << " should flag: " << line);
            CHECK(flags_line(c, line));
        }
        for (const std::string& line : c.rule->must_not_match) {
            INFO(c.rule->question << " should not flag: " << line);
            CHECK_FALSE(flags_line(c, line));
        }
        for (const OwnerLine& o : c.rule->owner_lines) {
            INFO(c.rule->question << " owner line the rule does not flag: " << o.line_text);
            CHECK(flags_line(c, o.line_text));
        }
    }
    // Every baseline entry names a real rule, and every question is unique.
    std::set<std::string> questions;
    for (const OwnerRule& r : rules()) {
        INFO("question listed twice: " << r.question);
        CHECK(questions.insert(r.question).second);
    }
    for (const KnownCopy& c : known_copies()) {
        INFO("baseline entry names no rule: " << c.question);
        CHECK(questions.count(c.question) == 1);
    }
}

TEST_CASE("single-owner rules hold across src/ and tools/") {
    const fs::path root = fs::u8path(HYDRA_SOURCE_DIR);
    const std::vector<CompiledRule> compiled = compile_rules();

    std::set<size_t> seen;  // indexes into known_copies() that matched a line
    std::set<const OwnerLine*> owner_seen;
    std::vector<std::string> problems;
    int files = 0;
    for (const char* sub : {"src", "tools"}) {
        for (const fs::directory_entry& e : fs::recursive_directory_iterator(root / sub)) {
            const fs::path ext = e.path().extension();
            if (ext != ".cpp" && ext != ".h") continue;
            ++files;
            const std::string rel = fs::relative(e.path(), root).generic_u8string();
            std::ifstream in(e.path());
            std::string line;
            int lineno = 0;
            while (std::getline(in, line)) {
                ++lineno;
                const std::string t = hydra::trim(line);
                if (t.empty() || t.compare(0, 2, "//") == 0) continue;
                for (const CompiledRule& c : compiled) {
                    const OwnerRule& rule = *c.rule;
                    bool skip = false;
                    for (const std::string& o : rule.owner_files) skip = skip || rel == o;
                    for (const Exempt& x : rule.exempt) skip = skip || rel == x.file;
                    if (skip || !flags_line(c, line)) continue;
                    bool owned = false;
                    for (const OwnerLine& o : rule.owner_lines) {
                        if (o.file == rel && o.line_text == t) {
                            owner_seen.insert(&o);
                            owned = true;
                        }
                    }
                    if (owned) continue;
                    bool known = false;
                    for (size_t i = 0; i < known_copies().size(); ++i) {
                        const KnownCopy& k = known_copies()[i];
                        if (k.question == rule.question && k.file == rel && k.line_text == t) {
                            seen.insert(i);
                            known = true;
                        }
                    }
                    if (!known)
                        problems.push_back(rel + ":" + std::to_string(lineno) + ": answers \"" +
                                           rule.question + "\", which belongs to " +
                                           rule.owner + ": " + t);
                }
            }
        }
    }
    for (size_t i = 0; i < known_copies().size(); ++i) {
        if (seen.count(i)) continue;
        const KnownCopy& c = known_copies()[i];
        problems.push_back("baseline entry no longer matches (remove it): " + c.file + ": " +
                           c.line_text);
    }
    for (const OwnerRule& r : rules()) {
        for (const OwnerLine& o : r.owner_lines) {
            if (owner_seen.count(&o)) continue;
            problems.push_back("owner line no longer matches (update it): " + o.file + ": " +
                               o.line_text);
        }
    }
    CHECK(files > 100);  // the scan found the sources
    std::ostringstream report;
    for (const std::string& p : problems) report << p << "\n";
    INFO(report.str());
    CHECK(problems.empty());
}
