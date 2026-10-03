// One owner per rule, checked where a grep can check it. Each row names a
// question, the only file allowed to answer it, and a pattern for the text
// that answers it. A matching line anywhere else fails here, unless the
// baseline lists it with the fix that will remove it. A baseline entry that
// no longer matches also fails, so the list only shrinks.
#include "doctest.h"

#include <filesystem>
#include <fstream>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace {

struct OwnerRule {
    std::string question;               // plain English, shown on failure
    std::string pattern;                // ECMAScript regex, matched per line
    std::vector<std::string> owners;    // repo-relative files allowed to match
    std::string decided_by;             // ADR, CONTEXT.md or the user's words
    std::vector<std::string> must_match;
    std::vector<std::string> must_not_match;
};

struct KnownCopy {
    std::string question;   // equals a rule's question
    std::string file;       // repo-relative, forward slashes
    std::string line_text;  // the line with leading and trailing space trimmed
    std::string removed_by; // the fix that deletes it
};

const std::vector<OwnerRule>& rules() {
    static const std::vector<OwnerRule> r = {
        {"Does the Windows shell take a path this long?",
         R"([<>]=?\s*MAX_PATH\b)",
         {"src/core/winstr.cpp"},
         "ADR 0020; one-owner fix on the 2026-10-03 fix list",
         {"if (path.size() < MAX_PATH) return path;",
          "if (copy.native().size() >= MAX_PATH) return {};",
          "return s.size() < MAX_PATH ? s : L\"\";"},
         {"wchar_t tmp[MAX_PATH + 1];", "GetTempPathW(MAX_PATH + 1, tmp);"}},
        {"How many bytes does a file hold?",
         R"((^|[^\w])(std::)?f(tell|seek)\s*\()",
         {"src/core/winstr.cpp"},
         "ADR 0020 (read_file_bytes reads files over 2 GB)",
         {"long n = std::ftell(f);", "std::fseek(f, 0, SEEK_END);", "fseek(f, 0, SEEK_SET);"},
         {"_fseeki64(f, 0, SEEK_END);", "const long long n = _ftelli64(f);"}},
        {"How is a long path prefixed for Win32?",
         R"(\\\\\\\\\?\\\\)",
         {"src/core/winstr.cpp"},
         "ADR 0020",
         {"return L\"\\\\\\\\?\\\\\" + full;"},
         {"return L\"\\\\\\\\\" + s.substr(8);"}},
    };
    return r;
}

const std::vector<KnownCopy>& known_copies() {
    static const std::vector<KnownCopy> k = {
        {"Does the Windows shell take a path this long?", "src/app/report_files.cpp",
         "if (n == 0 || n > MAX_PATH) return {};",
         "fix list: one owner for the shell path limit"},
        {"Does the Windows shell take a path this long?", "src/app/report_files.cpp",
         "if (copy.native().size() >= MAX_PATH) return {};",
         "fix list: one owner for the shell path limit"},
        {"Does the Windows shell take a path this long?", "src/app/report_files.cpp",
         "if (path.size() < MAX_PATH) return shell_open(path);",
         "fix list: one owner for the shell path limit"},
        {"How many bytes does a file hold?", "src/parse/midi.cpp",
         "std::fseek(f, 0, SEEK_END);", "round 7 candidate N2-5 (MidiFile::from_file)"},
        {"How many bytes does a file hold?", "src/parse/midi.cpp",
         "long n = std::ftell(f);", "round 7 candidate N2-5 (MidiFile::from_file)"},
        {"How many bytes does a file hold?", "src/parse/midi.cpp",
         "std::fseek(f, 0, SEEK_SET);", "round 7 candidate N2-5 (MidiFile::from_file)"},
    };
    return k;
}

std::string trim(const std::string& s) {
    const size_t a = s.find_first_not_of(" \t\r");
    if (a == std::string::npos) return {};
    const size_t b = s.find_last_not_of(" \t\r");
    return s.substr(a, b - a + 1);
}

}  // namespace

TEST_CASE("single-owner rules match their own examples") {
    for (const OwnerRule& r : rules()) {
        const std::regex re(r.pattern);
        for (const std::string& line : r.must_match) {
            INFO(r.question << " should match: " << line);
            CHECK(std::regex_search(line, re));
        }
        for (const std::string& line : r.must_not_match) {
            INFO(r.question << " should not match: " << line);
            CHECK_FALSE(std::regex_search(line, re));
        }
    }
    // Every baseline entry names a real rule.
    std::set<std::string> questions;
    for (const OwnerRule& r : rules()) questions.insert(r.question);
    for (const KnownCopy& c : known_copies()) {
        INFO("baseline entry names no rule: " << c.question);
        CHECK(questions.count(c.question) == 1);
    }
}

TEST_CASE("single-owner rules hold across src/ and tools/") {
    const fs::path root = fs::u8path(HYDRA_SOURCE_DIR);
    std::vector<std::pair<OwnerRule, std::regex>> compiled;
    for (const OwnerRule& r : rules()) compiled.emplace_back(r, std::regex(r.pattern));

    std::set<size_t> seen;  // indexes into known_copies() that matched a line
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
                const std::string t = trim(line);
                if (t.empty() || t.compare(0, 2, "//") == 0) continue;
                for (const auto& [rule, re] : compiled) {
                    bool owner = false;
                    for (const std::string& o : rule.owners) owner = owner || rel == o;
                    if (owner || !std::regex_search(line, re)) continue;
                    bool known = false;
                    for (size_t i = 0; i < known_copies().size(); ++i) {
                        const KnownCopy& c = known_copies()[i];
                        if (c.question == rule.question && c.file == rel && c.line_text == t) {
                            seen.insert(i);
                            known = true;
                        }
                    }
                    if (!known)
                        problems.push_back(rel + ":" + std::to_string(lineno) + ": answers \"" +
                                           rule.question + "\", which belongs to " +
                                           rule.owners.front() + ": " + t);
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
    CHECK(files > 100);  // the scan found the sources
    std::ostringstream report;
    for (const std::string& p : problems) report << p << "\n";
    INFO(report.str());
    CHECK(problems.empty());
}
