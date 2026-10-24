// What the row scan (engine.cpp) and the clone scan (clones.cpp) share: the
// list of lines a scan accepts by their exact text, the one rule for using
// them up, and how both read a line of source. How a scan's test case ends
// is check_scan.h's.
#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "core/strutil.h"

namespace single_owner {

// A line the scan accepts by its exact text: an owner's own line or a
// baseline copy. Both kinds go in one list and follow one rule.
struct ListedLine {
    std::string question;
    std::string file;       // repo-relative, forward slashes
    std::string line_text;  // trimmed
    std::string stale;      // the message when no source line uses it
    std::string rules_file; // the row's rules file (named by select_rule_files in engine.cpp), or empty for a baseline entry
};

// The end of every failure the row scan gives about one row, so the fixer
// opens the right file.
inline std::string rule_suffix(const std::string& rules_file) {
    return rules_file.empty() ? std::string() : " (rule: " + rules_file + ")";
}

// The failure for a listed line no source line took.
inline std::string stale_message(const ListedLine& l) {
    return l.stale + ": " + l.file + ": " + l.line_text + rule_suffix(l.rules_file);
}

enum class Take { taken, used_up, unlisted };

// Each listed entry covers exactly one line of source: the first entry with
// this question, file and text that no earlier line used takes it. A
// word-for-word copy finds every such entry used (used_up). When the line is
// taken and `taken_index` is set, it receives the entry's index in `listed`.
inline Take take_listed_line(const std::vector<ListedLine>& listed, std::vector<bool>& used,
                             const std::string& question, const std::string& file,
                             const std::string& text, size_t* taken_index = nullptr) {
    bool any = false;
    for (size_t i = 0; i < listed.size(); ++i) {
        const ListedLine& l = listed[i];
        if (l.question != question || l.file != file || l.line_text != text) continue;
        any = true;
        if (!used[i]) {
            used[i] = true;
            if (taken_index) *taken_index = i;
            return Take::taken;
        }
    }
    return any ? Take::used_up : Take::unlisted;
}

// Whether a file the walk finds is C++ source, the files the C++ scans read.
inline bool is_cpp_source(const std::filesystem::path& path) {
    const std::filesystem::path ext = path.extension();
    return ext == ".cpp" || ext == ".h";
}

// Whether a trimmed source line is a comment line, which the scans skip.
inline bool is_line_comment(std::string_view trimmed) {
    return hydra::starts_with(trimmed, "//");
}

// Defined with the clone scan in clones.cpp. The owner-gone guard in scan.cpp
// uses is_word_byte for its whole-word check.
std::string clone_line_text(std::string_view line, bool& in_comment);
bool is_word_byte(char c);

}  // namespace single_owner
