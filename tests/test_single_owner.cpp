// One owner per rule, checked where a grep can check it. Each row names a
// question, the one place that answers it, and a pattern for the text that
// answers it. A matching line anywhere else fails here, unless it calls the
// owner on that same line, or the baseline lists it with the fix that will
// remove it. A baseline entry that no longer matches also fails, so the list
// only shrinks. Each listed line (baseline or owner) covers exactly one line
// of source, so a word-for-word copy of it fails too.
//
// This is the one scan of src/, tools/ and tests/. A row covers src/ and
// tools/ unless it names its own scope; the rows about tests (a test that
// recomputes an engine fact, or walks the source tree itself) cover tests/.
// The long-path rules (ADR 0020) are rows here too, so a new one-place rule
// is a new row, not a new walker. The walk itself, and the repo root, come
// from tests/source_tree.h. The clone scan at the end of this file (ADR 0025)
// reads the same walk for blocks of code pasted into two places.
#include "doctest.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <istream>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "core/strutil.h"
#include "source_tree.h"

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
    std::string why;        // the owner, or the recorded decision that allows it
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
    // Top folders scanned, or single repo-relative files; empty means src and
    // tools.
    std::vector<std::string> scope;
    // When set, the rule checks only one function's body in one file: from
    // the line in function_file that contains unction to the next line
    // that is a lone "}". The scan fails if it never finds that line.
    std::string function_file;
    std::string function;
    // Whole-line comments are skipped unless this is set (a rule about a
    // name that comments must not use).
    bool scan_comments = false;
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
        // which audit R7.12 calls a different question. A length is .size(),
        // .length() (through . or ->), strlen or wcslen. The limit may be
        // spelled MAX_PATH, 260 or 259, on either side of any comparison
        // (<, <=, >, >=, ==, !=), and either side may carry a little
        // arithmetic (size() - 4 < MAX_PATH). A length held in a plain
        // variable is not caught: `n > MAX_PATH` is also how a buffer-fit
        // check reads. winstr.cpp is not exempt as a file: only fits_shell's
        // one line is listed.
        {"Does the Windows shell take a path this long?",
         "fits_shell in src/core/winstr.cpp",
         R"(((\.|->)(size|length)\(\)|\b(wcs|str)len\s*\(([^()]|\([^()]*\))*\))(\s*[-+]\s*[\w.]+)*\s*(==|!=|[<>]=?)\s*(MAX_PATH|259|260)\b|\b(MAX_PATH|259|260)(\s*[-+]\s*[\w.]+)*\s*(==|!=|[<>]=?)\s*[\w.:>()-]*((\.|->)(size|length)\(\)|\b(wcs|str)len\s*\())",
         "",
         {},
         {},
         "ADR 0020 (the shell takes nothing of 260 or more, and no prefixed path); one owner put on the fix "
         "list by the user 2026-10-03 (audit R7.12 addendum)",
         {"if (path.size() < MAX_PATH) return path;",
          "if (copy.native().size() >= MAX_PATH) return {};",
          "return s.size() < MAX_PATH ? s : L\"\";",
          "if (wcslen(p) >= MAX_PATH) return false;",
          "if (MAX_PATH > name.length()) ok = true;",
          "if (path.size() - 4 < MAX_PATH) return path;",
          "if (name.length() >= 260) return false;",
          "if (260 <= s.size()) ok = false;",
          "if (MAX_PATH - 1 > wcslen(p)) ok = true;",
          "if (path.size() <= 259) return path;", "if (path.size() > 259) return {};",
          "if (path.size() == 260) return {};", "if (path.size() != 260) ok = true;",
          "if (259 >= s.length()) ok = true;", "if (260 == p->size()) return {};",
          "if (p->size() < MAX_PATH) return p;",
          "if (wcslen(path.c_str()) >= MAX_PATH) return false;",
          "if (MAX_PATH <= strlen(s.c_str())) ok = false;"},
         {"wchar_t tmp[MAX_PATH + 1];", "GetTempPathW(MAX_PATH + 1, tmp);",
          "if (n == 0 || n > MAX_PATH) return {};",
          "constexpr size_t kPlainPathLimit = MAX_PATH - 12;", "const int kWidth = 260;",
          "if (n > 260) return;", "if (rows.size() > 2600) return;",
          "if (rows.size() == 2590) return;", "if (12600 < rows.size()) return;",
          "std::wstring buf(MAX_PATH, L'\\0');",
          "if (fits_shell(path)) return path;", "if (!fits_shell(copy.native())) return {};"},
         {{"src/core/winstr.cpp",
           R"(return path.size() < MAX_PATH && !has_namespace_prefix(path);)",
           "fits_shell, the owner: shell_path, open_in_browser and copy_to_short_temp "
           "call it"}}},
        // Every way this codebase works out a file's size: the stdio seek and
        // tell (32- and 64-bit), stream seekg/tellg, the Win32 size calls and
        // the size fields of their find and attribute data, the CRT's
        // filelength and fstat, std::filesystem's file_size, and a seek to the
        // end. open_handle_size_bytes owns sizing a file, open or by path:
        // open_file_size_bytes (a stdio stream), file_size_bytes and
        // read_file_bytes call it. list_dir uses the listing's size by D39.
        // winstr.cpp is not exempt as a file: only those two lines are listed.
        {"How many bytes does a file hold?",
         "open_handle_size_bytes in src/core/winstr.cpp (list_dir uses the folder "
         "listing's size by decision D39)",
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
         {{"src/core/winstr.cpp",
           "if (!GetFileSizeEx(h, &size) || size.QuadPart < 0) return std::nullopt;",
           "open_handle_size_bytes, the owner: the other size helpers call it"},
          {"src/core/winstr.cpp",
           "e.size = (static_cast<uint64_t>(fd.nFileSizeHigh) << 32) | fd.nFileSizeLow;",
           "list_dir keeps the folder listing's size for the rescan cache, because "
           "opening every library file would slow scans: the user's decision D39 "
           "(2026-10-04)"}}},
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
         {{"src/net/dmbot_client.cpp", "it converts a URL, not a path"},
          {"src/ui/main.cpp", "it converts the startup message box's text, not a path"}},
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
        // The shell takes no long or prefixed path (fits_shell), so only the
        // two report buttons launch it, each with shell_path's short form.
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
        // calls one of these) or a file stream. The list is every
        // std::filesystem free function that takes a path, plus the two
        // directory iterators and directory_entry, which read the disk when
        // built from a path: as a temporary, a named variable or with
        // braces. An empty argument list is no path (fs::current_path(), an
        // end iterator), so it passes. No file is exempt, winstr included.
        {"How does a std::filesystem call or file stream get a path it accepts at any length?",
         "os_path in src/core/winstr.cpp",
         R"(^(?=.*(filesystem::|fs::)).*::((absolute|canonical|weakly_canonical|relative|proximate|copy|copy_file|copy_symlink|create_directory|create_directories|create_directory_symlink|create_hard_link|create_symlink|current_path|equivalent|exists|file_size|hard_link_count|is_block_file|is_character_file|is_directory|is_empty|is_fifo|is_other|is_regular_file|is_socket|is_symlink|last_write_time|permissions|read_symlink|remove|remove_all|rename|resize_file|space|status|symlink_status)\s*\(|((recursive_)?directory_iterator|directory_entry)\s*(\w+\s*)?[({])(?!\s*[)}])|^(?!.*#include).*fstream(\s|[({](?!\s*[)}])))",
         R"(os_path\()",
         {},
         {},
         "ADR 0020",
         {"fs::create_directories(dir);", "std::ifstream in(path);",
          "for (const auto& e : std::filesystem::directory_iterator(dir)) {",
          "for (const auto& e : fs::recursive_directory_iterator(root)) {",
          "std::filesystem::path outpath = std::filesystem::absolute(std::filesystem::u8path(out));",
          "const fs::file_status st = fs::status(p);", "if (fs::equivalent(a, b)) return;",
          "const fs::space_info s = fs::space(dir);", "fs::current_path(dir);",
          "std::filesystem::directory_iterator it(dir);",
          "std::filesystem::recursive_directory_iterator it(root, ec);",
          "for (auto& e : std::filesystem::directory_iterator{dir}) {",
          "for (auto& e : fs::recursive_directory_iterator{root}) {",
          "const fs::directory_entry entry(p);", "if (fs::is_empty(dir)) return;",
          "const fs::path rel = fs::relative(p, base);",
          "const fs::path rel = fs::proximate(p, base);", "fs::resize_file(p, 0);",
          "fs::permissions(p, fs::perms::owner_write);",
          "const auto st = fs::symlink_status(p);", "fs::create_symlink(target, link);",
          "fs::create_directory_symlink(target, link);", "fs::create_hard_link(target, link);",
          "fs::copy_symlink(from, to);", "const fs::path t = fs::read_symlink(p);",
          "const auto n = fs::hard_link_count(p);", "if (fs::is_symlink(p)) return;",
          "if (fs::is_block_file(p) || fs::is_character_file(p)) return;",
          "if (fs::is_fifo(p) || fs::is_socket(p) || fs::is_other(p)) return;",
          "if (fs::exists (p)) return;", "const std::string s = slurp(std::ifstream(path));",
          "std::ofstream{path} << text;"},
         {"fs::create_directories(hydra::os_path(dir));", "#include <fstream>",
          "#include <filesystem>",
          "std::ifstream in(hydra::os_path(path));", "const fs::path p = dir / name;",
          "for (const auto& e : fs::recursive_directory_iterator(hydra::os_path(root))) {",
          "std::filesystem::path outpath = std::filesystem::absolute(hydra::os_path(std::filesystem::u8path(out)));",
          "std::filesystem::directory_iterator it(hydra::os_path(dir), ec);",
          "for (; it != fs::directory_iterator(); ++it) {",
          "const fs::path cwd = fs::current_path();",
          "for (const fs::directory_entry& e : list) {",
          "const fs::file_status st = e.status();", "if (e.is_directory()) continue;",
          "const fs::path tmp = fs::temp_directory_path(ec);",
          "void write(std::ofstream& out);",
          "const RecordStatusView& PathsTabCache::status(const store::RecordLookup& lookup,"}},

        // ---- engine facts (step 1 of the 2026-10-03 derivation audit) ----

        // A chord exactly on the SP end is not after it.
        {"Is this phrase chord after the SP end?",
         "after_sp_end in src/core/backend_value.h",
         R"(\.ticks\(\)\s*>\s*end\.ticks\(\)|\btick\s*>\s*w\.deact_tick\b)",
         R"(after_sp_end\()",
         {},
         {},
         "audit findings 1 and 32; step-1 derive-once review finding 1 (2026-10-04)",
         {"choice.late = c->timecode.ticks() > end.ticks();",
          "const bool past_deact = row.tick > w.deact_tick;"},
         {"if (s.tick > var.fold_tick)",
          "choice.late = core::after_sp_end(c->timecode.ticks(), end.ticks());"}},
        {"Is this row the squeezed-out chord, or past it?",
         "sqout_position in src/core/backend_value.h",
         R"(ticks\(\)\s*(==|>|<)\s*\*?sqout_tick\b)",
         "",
         {"src/core/backend_value.h"},
         {},
         "audit finding 146; step-1 derive-once review finding 5 (2026-10-04)",
         {"return sqout_tick.has_value() && bsq.timecode.ticks() == *sqout_tick;",
          "if (b.timecode.ticks() == *sqout_tick) return &b;"},
         {"if (row_tick < *sqout_tick) return SqOutPosition::Before;",
          "return core::sqout_position(bsq.timecode.ticks(), sqout_tick) == core::SqOutPosition::Exact;"},
         {},
         // Tests too (J3-2): a test asks sqout_position.
         {"src", "tools", "tests"}},
        {"Is this SP-end step a squeeze-in?",
         "is_sqin_kind in src/core/model.h",
         R"(\bkind\s*==\s*SpEndKind::SqIn\b)",
         "",
         {},
         {},
         "ADR 0014 (D34: the window holds an SqIn step on that phrase); step-1 derive-once "
         "review findings 2 and 3 (2026-10-04)",
         {"if (s.tick == tick && s.kind == SpEndKind::SqIn) return true;",
          "if (s.kind == SpEndKind::SqIn) out.push_back(s.tick);"},
         {"if (s.kind != SpEndKind::SqIn) continue;", "if (is_sqin_kind(s.kind)) out.push_back(s.tick);"},
         {{"src/core/model.h",
           "inline bool is_sqin_kind(SpEndKind kind) { return kind == SpEndKind::SqIn; }",
           "is_sqin_kind, the owner"}},
         // Tests too (step-1 derive-once sweep after the review of cc2e1d9):
         // a test asks is_sqin_kind, is_sqin_step_on or sqin_phrase_ticks.
         {"src", "tools", "tests"}},
        {"Does an E0 need nothing passed over?",
         "is_e0 in src/core/model.h",
         R"(skips\s*==\s*0\b)",
         "",
         {},
         {},
         "CONTEXT.md (E0); step-1 derive-once review finding 4 (2026-10-04)",
         {"const bool can_be_e0 = has_ms_filter_ && p.currentskips == 0;",
          "return e_offset < kEarlyFillWindowMs && skips == 0;"},
         {"CHECK(act.skips() == 0);", "if (is_e0(e_offset, p.currentskips)) ++over;"},
         {{"src/core/model.h", "return e_offset < kEarlyFillWindowMs && skips == 0;",
           "is_e0, the owner"}}},
        {"Which SP-end kinds are valid?",
         "kLastSpEndKind in src/core/model.h",
         R"([<>]=?\s*static_cast<uint8_t>\(SpEndKind::)",
         "",
         {"src/core/model.h"},
         {},
         "ADR 0021 (the SP-end history); step-1 derive-once review finding 7 (2026-10-04)",
         {"if (kind > static_cast<uint8_t>(SpEndKind::SqIn))",
          "if (kind >= static_cast<uint8_t>(SpEndKind::Clamped))"},
         {"w.u8(static_cast<uint8_t>(s.kind));",
          "if (kind > static_cast<uint8_t>(kLastSpEndKind))"}},
        // Was its own walker in test_model.cpp (review finding 12).
        {"Who reads the early-fill window?",
         "fill_refuses and is_e0 in src/core/model.h",
         R"(\bkEarlyFillWindowMs\b)",
         "",
         {},
         {},
         "review of D35, finding A; step-1 derive-once review finding 12 (2026-10-04)",
         {"if (e_offset < kEarlyFillWindowMs) ok = true;",
          "return d - hydra::kEarlyFillWindowMs;"},
         {"if (is_e0(e_offset, 0)) ok = true;"},
         {{"src/core/model.h", "constexpr double kEarlyFillWindowMs = 60.0;",
           "the constant itself"},
          {"src/core/model.h",
           "inline bool fill_refuses(double e_offset) { return e_offset < -kEarlyFillWindowMs; }",
           "fill_refuses, the owner of the refusal"},
          {"src/core/model.h", "return e_offset < kEarlyFillWindowMs && skips == 0;",
           "is_e0, the owner of the E0"}}},
        // Was its own walker in test_model.cpp (review finding 12). The replay
        // keeps its own windows (ReplayWindow, and replay.cpp's Window) with a
        // field of the same name; those are always named w or win, and a
        // write to w.sqout_tick or win.sqout_tick is not flagged.
        {"Who writes an activation's sqout_tick?",
         "Activation::set_sqout in src/core/model.cpp",
         R"((^|[^\w.>])sqout_tick\s*(=(?!=)|\.\s*(reset|emplace)\s*\()|\b\w+\s*(\.|->)\s*sqout_tick\s*(=(?!=)|\.\s*(reset|emplace)\s*\())",
         "",
         {},
         {},
         "ADR 0014 (the squeeze-out is stored once); step-1 derive-once review finding 12 "
         "(2026-10-04)",
         {"act.sqout_tick = 200;", "a->sqout_tick.reset();", "sqout_tick.emplace(5);",
          "w.sqout_tick = 5;"},
         {"if (act.sqout_tick == t)", "w.opt_i64(act.sqout_tick);",
          "if (a.sqout_tick != b.sqout_tick) return false;"},
         {{"src/core/model.cpp", "sqout_tick = tick;", "set_sqout, the one writer"},
          {"src/core/model.cpp", "sqout_tick.reset();",
           "set_sqout, undoing its write when no row is on the tick"},
          {"src/core/replay.cpp", "win.sqout_tick = w.sqout_tick;",
           "the replay's own window (ReplayWindow), not an Activation"},
          {"src/core/replay.cpp", "w.sqout_tick = row->timecode.ticks();",
           "the replay's own window (replay.cpp's Window), not an Activation"},
          {"tools/replay.cpp", "w.sqout_tick = n.tick;",
           "a window typed by hand for hydra_replay (ReplayWindow), not an Activation"},
          {"tools/replay_json.cpp", "w.sqout_tick = act[\"sqout_tick\"].get<int64_t>();",
           "a window read from hydra_replay's JSON (ReplayWindow), not an Activation"}}},
        {"Is this SP-end step a clamp?",
         "is_clamp_kind in src/core/model.h (last_clamp_tick asks it)",
         R"(\bkind\s*==\s*SpEndKind::Clamped\b)",
         "",
         {},
         {},
         "ADR 0013 (the clamp note is stored); step-1 derive-once review of fb1189b, finding 2 "
         "(2026-10-04); T10+T11 derive-once review finding 6 (2026-10-04)",
         {"if (it->kind == SpEndKind::Clamped) return it->tick;",
          "if (sp_end_steps[s].kind == SpEndKind::Clamped) return sp_end_steps[s].tick;"},
         {"mit->second.clamped ? SpEndKind::Clamped"},
         {{"src/core/model.h",
           "inline bool is_clamp_kind(SpEndKind kind) { return kind == SpEndKind::Clamped; }",
           "is_clamp_kind, the owner"}}},
        // D36: an SP end offers a window its newest phrase only when that
        // phrase's step moved the end from there. Comparing a step's moved-
        // from end to an SP end anywhere else restates the early side.
        {"Does this SP end offer the window its newest phrase?",
         "core::offered_phrase in src/core/sqout_chord.h",
         R"(\b(sqout_at|newest_moved_from)\s*[!=]=(?!\s*NO_TIME\b))",
         "",
         {},
         {},
         "D36 (extreme-tempo gaps, 2026-10-04)",
         {"if (newest.sqout_at == d)", "if (newest_moved_from == sp_end)",
          "return s.sqout_at != node_tick;"},
         {"const int64_t at = ends_[(size_t)p.end_tail].sqout_at;",
          "newest.sqout_at == NO_TIME ? std::nullopt : std::optional<int64_t>(newest.sqout_at),"},
         {{"src/core/sqout_chord.h", "if (newest_moved_from == sp_end && !squeezed_in(newest_tick)) {",
           "offered_phrase, the owner"}}},
        // Was part of a walker in test_squeeze_rating.cpp (review finding 12).
        // An optional namespace prefix (hydra::kSqueezeWindowMs) must not
        // hide a comparison.
        {"Is a value inside the squeeze window?",
         "within_squeeze_window in src/core/model.h",
         R"([<>]=?\s*(\w+::)*kSqueezeWindowMs|(\w+::)*kSqueezeWindowMs\s*[<>])",
         "",
         {},
         {},
         "ADR 0014; step-1 plan (one squeeze window); step-1 derive-once review finding 12 "
         "(2026-10-04)",
         {"x < kSqueezeWindowMs", "x < hydra::kSqueezeWindowMs", "hydra::kSqueezeWindowMs > x"},
         {"clamp(v, 0, hydra::kSqueezeWindowMs)", "static_cast<int>(kSqueezeWindowMs));"},
         {{"src/core/model.h", "return std::fabs(offset_from_sp_end_ms) < kSqueezeWindowMs;",
           "within_squeeze_window, the owner"}}},
        // The rest of that walker. The Path limit's ceiling is the squeeze
        // window too (decision D41), so no clamp is exempt.
        {"Is the squeeze window typed as 500?",
         "kSqueezeWindowMs in src/core/model.h",
         R"(clamp\([^;]*[-, ]500\b|within 500 ms)",
         "",
         {},
         {},
         "step-1 plan (one squeeze window); the user's decision D41 (2026-10-04: the Path "
         "limit's ±500 ms ceiling is the squeeze window)",
         {"app.settings.mslimit_value = std::clamp(app.settings.mslimit_value, -500, 500);",
          "app.settings.backendlimit_value = std::clamp(app.settings.backendlimit_value, 0, 500);",
          "label = \"within 500 ms of the SP end\";"},
         {"app.settings.mslimit_value = std::clamp(app.settings.mslimit_value, -window, window);",
          "x = std::clamp(x, 0, 5000);"}},

        // ---- rows that also cover tests/ ----

        {"Where does SP end, counting bars as measures?",
         "add_act_edge and add_deact_edge in src/search/graph.cpp",
         R"(plusmeasure\([^;]*sp_bars_to_measures\()",
         "",
         {"src/search/graph.cpp"},
         {{"tests/test_timing.cpp",
           "it tests plusmeasure itself and pins its result as a literal tick"}},
         "audit finding 156; step-1 derive-once review findings 8 and 10 (2026-10-04)",
         {"song.timing().plusmeasure(act.timecode, sp_bars_to_measures(act.sp_meter())).ticks());",
          "song.timing().plusmeasure(end, sp_bars_to_measures(1)).ticks();"},
         {"CHECK(st.plusmeasure(act.timecode, 16).ticks() == 21840);"},
         {},
         {"src", "tools", "tests"}},
        {"Is a note inside the squeeze window, typed as 500 in a test?",
         "within_squeeze_window in src/core/model.h",
         R"(fabs\([^)]*\)\s*<\s*500(\.0)?\b)",
         "",
         {},
         {},
         "step-1 derive-once review finding 9 (2026-10-04)",
         {"CHECK((std::fabs(*b.offset_ms) < 500.0 || act.is_sqout_backend(b)));",
          "if (std::fabs(*copy.offset_ms) < 500.0) want.push_back(copy);"},
         {"if (n.ms < time_ms - 500.0 || n.ms > time_ms + 1500.0) continue;"},
         {},
         {"tests"}},
        {"Which deactivation edge has this SP end, in a test?",
         "deact_edge_at in tests/record_fixtures.h",
         R"(const ScoreGraphEdge\*\s+deact_edge_at\()",
         "",
         {"tests/record_fixtures.h"},
         {},
         "T10+T11 derive-once review finding 2 (2026-10-04)",
         {"const ScoreGraphEdge* deact_edge_at(const ScoreGraph& graph, int64_t tick) {",
          "const ScoreGraphEdge* deact_edge_at(const ScoreGraph& graph, int64_t end_tick) {"},
         {"const ScoreGraphEdge* e = deact_edge_at(graph, 13440);"},
         {},
         {"tests"}},
        {"Which search options keep every branch, in a test?",
         "wide_search in tests/record_fixtures.h",
         R"(^\s*((inline|static)\s+)?EngineOptions\s+\w+\(\)\s*\{)",
         "",
         {"tests/record_fixtures.h"},
         {},
         "T10+T11 derive-once review finding 3 (2026-10-04)",
         {"EngineOptions keep_losers() {", "static EngineOptions keep_all() {"},
         {"EngineOptions o;"},
         {},
         {"tests"}},
        {"Which paths did a search return, tied variants included, in a test?",
         "flatten_paths in src/core/model.h (HydraRecord::all_paths for a record)",
         R"(for \(const Path&\s*\w+\s*:\s*\w+\.variants\)\s*\w+\(\w+\);)",
         "",
         {},
         {},
         "T10+T11 derive-once review finding 4 (2026-10-04)",
         {"for (const Path& v : p.variants) scan(v);",
          "for (const Path& w : root.variants) check(w);"},
         {"for (const Path& v : path.variants) collect_payloads(v, out);"},
         {},
         {"tests"}},
        // all_paths already walks every root's tied variants (flatten_paths),
        // so adding all_tied's list to it checks each variant twice.
        {"Every stored path of a record, tied variants included, in a test?",
         "HydraRecord::all_paths in src/core/model.cpp",
         R"(for \(const (hydra::)?Path\* \w+ : all_tied\(\w+\.paths\)\) \w+\.push_back\()",
         "",
         {},
         {},
         "D36 derive-once review finding 1 (2026-10-04)",
         {"for (const Path* p : all_tied(rec.paths)) all.push_back(p);",
          "for (const hydra::Path* v : all_tied(r.paths)) out.push_back(v);"},
         {"for (const Path* p : rec.all_allzero_paths()) all.push_back(p);"},
         {},
         {"tests"}},
        {"Where does the graph's SP track start, in a test?",
         "sp_track_start in tests/record_fixtures.h",
         R"(branch_edge\)\s*\w+\s*=\s*\w+->branch_edge->dest;)",
         "",
         {},
         {},
         "D36 derive-once review finding 3 (2026-10-04)",
         {"if (b->branch_edge) sp = b->branch_edge->dest;",
          "if (!sp_start && n->branch_edge) sp_start = n->branch_edge->dest;"},
         {"if (sp->branch_edge && sp->branch_edge->dest->timecode.ticks() == end_tick)"},
         {},
         {"tests"}},
        {"Does every stored path of one record bank in order, in a test?",
         "bank_check::check_record_banks in tests/bank_check.h",
         R"(=\s*bank_check::phrase_ends\(\w+\);)",
         "",
         {},
         {},
         "D36 derive-once review finding 2 (2026-10-04)",
         {"const std::set<int64_t> phrase_ends = bank_check::phrase_ends(song);",
          "std::set<int64_t> ends = bank_check::phrase_ends(s);"},
         {"inline std::set<int64_t> phrase_ends(const hydra::Song& song) {"},
         {},
         {"tests"}},
        // D36 deleted core::sqout_chord; a comment that still names it points
        // at nothing. Comments are scanned too, since that is where it lived.
        {"Which chords can this SP end squeeze out (the deleted sqout_chord's name)?",
         "core::sqout_chords in src/core/sqout_chord.h",
         R"(\bsqout_chord\b(?![s.]))",
         "",
         {},
         {},
         "D36 derive-once review finding 10 (2026-10-04)",
         {"// the one core::sqout_chord names. A typed offset is matched",
          "// The engine squeezes out only the chord core::sqout_chord names for this"},
         {"core::sqout_chords(song, deact_tc, w.act_tick, w.sqin_ticks, false);",
          "#include \"core/sqout_chord.h\""},
         {},
         {"src", "tools", "tests"},
         "",
         "",
         true},
        {"Which ticks does a path activate on, in a test?",
         "act_ticks in tests/record_fixtures.h",
         R"(for \(const Activation&\s*\w+\s*:.*\)\s*\w+\.push_back\(\w+\.timecode\.ticks\(\)\))",
         "",
         {"tests/record_fixtures.h"},
         {},
         "T10+T11 derive-once review finding 5 (2026-10-04)",
         {"for (const Activation& a : acts) ticks.push_back(a.timecode.ticks());",
          "for (const Activation& a : p.all_activations()) t.push_back(a.timecode.ticks());"},
         {"for (const SongTimestamp& ts : song.sequence) ticks.push_back(ts.timecode.ticks());"},
         {},
         {"tests"}},
        {"How far is a note from an SP end, in a test?",
         "offset_from_sp_end in src/core/model.h",
         R"(\.ms\(\)\s*-\s*(end\.ms\(\)|end_ms\b)|(sqout_ms|offset_ms|own_ms)\s*==?\s*\w+\.timecode\(|-\s*\w+\.timecode\([^;]*\b(\w*end|deact_tick\(\))\)*\.ms\(\)|\w*_ms\([^;]*\)\s*-\s*\w*end\w*_ms\b)",
         "",
         {},
         {},
         "step-1 derive-once review findings 8, 9 and R7.30 (2026-10-04); T10+T11 derive-once "
         "review finding 1 and its round-2 follow-up (2026-10-04)",
         {"CHECK(got.timing == c->timecode.ms() - end.ms());",
          "copy.offset_ms = b.timecode.ms() - end_ms;",
          "CHECK(s.sqout_ms == song.timecode(3456).ms() - song.timecode(5376).ms());",
          "song.timecode(*a.sqout_tick).ms() - song.timecode(*a.deact_tick()).ms());",
          "doctest::Approx(tick_ms(song, 5280) - end_ms).epsilon(1e-9));",
          "const double off = note_ms(song, t) - sp_end_ms;"},
         {"double gap = st.timecode(69120).ms() - st.timecode(68880).ms();",
          "double gap = tick_ms(song, b) - tick_ms(song, a);"},
         {},
         {"tests"}},
        {"What points band keeps every path?",
         "kKeepEveryPathBand in src/search/pather.h",
         R"(depth_value\s*=\s*1'?000'?000('?000)?\s*;)",
         "",
         {},
         {},
         "audit R7.29; step-1 derive-once review finding 13 (2026-10-04)",
         {"o.depth_value = 1'000'000'000;", "o.depth_value = 1000000;"},
         {"cfg.depth_value = 40;", "o.depth_value = kKeepEveryPathBand;"},
         {},
         {"src", "tools", "tests"}},
        {"Which test walks the source tree?",
         "sourcetree::for_each_source_file in tests/source_tree.h",
         R"(recursive_directory_iterator\(root\s*/\s*sub\))",
         "",
         {"tests/source_tree.h"},
         {},
         "this file's own rule (one scan, new rules are rows); step-1 derive-once review "
         "finding 12 (2026-10-04); one walker for the docs test too (phase 4 join, "
         "2026-10-04)",
         {"for (const fs::directory_entry& e : fs::recursive_directory_iterator(root / sub)) {",
          "for (const auto& e : fs::recursive_directory_iterator(root/sub))"},
         {"for (const auto& e : std::filesystem::directory_iterator(dir)) {"},
         {},
         {"tests"}},
        {"Which test reads the source tree?",
         "sourcetree::root in tests/source_tree.h",
         R"(\bHYDRA_SOURCE_DIR\b)",
         "",
         {"tests/source_tree.h"},
         {},
         "this file's own rule (one scan, new rules are rows); step-1 derive-once review of "
         "fb1189b, finding 5 (2026-10-04); one reader for the docs test too (phase 4 join, "
         "2026-10-04)",
         {"std::ifstream in(std::string(HYDRA_SOURCE_DIR) + \"/src/app/preview_view.cpp\");",
          "const fs::path root = fs::u8path(HYDRA_SOURCE_DIR);"},
         {"return load_songpath(std::string(HYDRA_INPUT_DIR) + \"/test_fast_tempo/\" + name, "
          "true, true);"},
         {},
         {"tests"}},
        // A hand write into the meter or signature map is `tpm_changes[...] =`
        // or `timesig_changes[...] =` followed by anything but a second `=`.
        {"Who writes a meter or a signature into Song's maps?",
         "apply_timesig in src/parse/song.cpp",
         R"((tpm_changes|timesig_changes)\[[^\]]*\]\s*=[^=])",
         "",
         {},
         {},
         "decision D27 (audit findings 258 and 319: the meter is written once, through "
         "apply_timesig); widened to signatures and tools by the step-2 derive-once review "
         "of 11b9d44",
         {"song.tpm_changes[0] = 768;", "fixture.tpm_changes[2880] = 1440;",
          "s.tpm_changes[t]=960;", "song.timesig_changes[0] = {4, 4};"},
         {"CHECK(song.tpm_changes[0] == 768);", "CHECK(fixture.tpm_changes.at(2880) == 1440);",
          "apply_timesig(fixture, 2880, 3, 4);",
          "CHECK(song.timesig_changes.at(0) == std::make_pair(4, 4));"},
         {{"src/parse/song.cpp",
           "song.tpm_changes[tick] = song.tick_resolution() * static_cast<int64_t>(numerator) * 4 /",
           "apply_timesig, the owner"},
          {"src/parse/song.cpp", "song.timesig_changes[tick] = {numerator, denominator};",
           "apply_timesig, the owner"}},
         {"src", "tools", "tests"}},
        // Step-2 derive-once review of 11b9d44, finding 3 (audit finding 286).
        {"Which test helper writes MThd/MTrk chunks?",
         "smf_tracks in tests/midi_util.h",
         R"re('M', 'T', '(h', 'd|r', 'k)'|"MTrk")re",
         "",
         {"tests/midi_util.h"},
         {},
         "audit finding 286; step-2 derive-once review of 11b9d44, finding 3",
         {"d.insert(d.end(), {'M', 'T', 'r', 'k'});", "const char* tag = \"MTrk\";"},
         {"smf(concat({track_name(\"PART DRUMS\"), end_of_track()}))"},
         {{"tests/test_midi.cpp",
           "'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 0, 0, 1, 0xE8, 0x00,  // div < 0",
           "a deliberately broken header (a negative division), which smf_tracks cannot "
           "write"}},
         {"tests"}},
        // Step-2 derive-once review of 11b9d44, finding 4 (audit finding 117).
        {"Which test helper deflates a container stream?",
         "deflate_raw in tests/srb_util.h",
         R"(tdefl_compress_mem_to_heap)",
         "",
         {"tests/srb_util.h"},
         {},
         "audit finding 117; step-2 derive-once review of 11b9d44, finding 4",
         {"void* p = tdefl_compress_mem_to_heap(src.data(), src.size(), &out_len,",
          "p = tdefl_compress_mem_to_heap(a, n, &len, 0);"},
         {"deflate_raw(meta)"},
         {},
         {"tests"}},
        // Step-2 derive-once review of 11b9d44, finding 6, widened by the review
        // of dbeb3a3, finding 1: any string literal that opens a [Song] or
        // [SyncTrack] section, so a header typed across several source lines,
        // or one that takes its lines as a parameter, is caught on its first
        // line. The test_song.cpp lines build deliberately malformed charts from
        // raw pieces; the test_app_state.cpp lines write a placeholder file.
        {"Which test helper writes a .chart [Song] or [SyncTrack] header?",
         "chart_text in tests/chart_text.h",
         R"("\[(Song|SyncTrack)\]\\n)",
         "",
         {"tests/chart_text.h"},
         {},
         "step-2 derive-once review of 11b9d44, finding 6; review of dbeb3a3, finding 1",
         {R"("[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n")",
          R"("[SyncTrack]\n{\n  0 = TS 3 3\n}\n")",
          R"("[SyncTrack]\n{\n" + sync + "}\n")", R"("[SyncTrack]\n")", R"("[Song]\n")",
          R"(std::string s = "[Song]\n{\n  Resolution = " + resolution + "\n}\n")"},
         {R"(testchart::section("Song", "  Resolution = 192\n"))",
          R"(testchart::section("SyncTrack", testchart::kSync44At120))",
          R"(write_bytes(ini, bytes_of("[Song]\r\nDelay = -250\r\n"));)"},
         {{"tests/test_song.cpp", R"("[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n";)",
           "\"section headers are found as the regex found them\" puts an arbitrary first "
           "line in front of this header, which chart_text cannot write"},
          {"tests/test_song.cpp",
           R"(const std::string song = "[Song]\n{\n  Resolution = 192\n}\n";)",
           "\"malformed lines keep their handling\" joins the sections with stray text between "
           "them, which chart_text cannot write"},
          {"tests/test_song.cpp",
           R"(const std::string sync = "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n";)",
           "\"malformed lines keep their handling\" joins the sections with stray text between "
           "them, which chart_text cannot write"},
          {"tests/test_song.cpp",
           R"(Song s = parse(song + "[Song]\n{\n  Resolution = 480\n}\n" + sync +)",
           "\"malformed lines keep their handling\" repeats the [Song] section to pin that the "
           "later one replaces it, which chart_text cannot write"},
          {"tests/test_app_state.cpp", R"({ std::ofstream f(chart); f << "[Song]\n"; })",
           "a placeholder .chart that only has to exist on disk; no case parses it"},
          {"tests/test_app_state.cpp", R"({ std::ofstream f(chart); f << "[Song]\n"; })",
           "a placeholder .chart that only has to exist on disk; no case parses it"}},
         {"tests"}},
        // Step-2 derive-once review of dbeb3a3, finding 2. A test's own
        // per-difficulty table pairs a difficulty with a number; a plain list
        // of difficulties to walk is not flagged. The owner's rows carry the
        // hydra:: prefix, so they do not match. Loose by design: a table
        // spelled another way needs a review reading.
        {"Which test types a difficulty's kick pitch or other per-difficulty value?",
         "kLiterals in tests/difficulty_literals.h",
         R"(\{Difficulty::(Hard|Medium|Easy), \d)",
         "",
         {"tests/difficulty_literals.h"},
         {},
         "step-2 derive-once review of dbeb3a3, finding 2",
         {"{Difficulty::Hard, 480},", "{Difficulty::Easy, 60, 59, '0'}"},
         {R"({hydra::Difficulty::Hard, 84, 83, 85, '2', "Hard", "HardDrums"},)",
          "for (Difficulty d : {Difficulty::Hard, Difficulty::Medium, Difficulty::Easy}) {"},
         {},
         {"tests"}},
        // Was part of the same walker: the Preview's time box reads Song's
        // default meter and keeps no 4/4 of its own. Scoped to its two files.
        {"Does the Preview type its own 4/4 meter?",
         "kDefaultTimeSigNumerator and kDefaultTimeSigDenominator in src/parse/song.h",
         R"((numerator|denominator|ts_num|ts_den)\s*=\s*4\b)",
         "",
         {},
         {},
         "decision D27 (audit findings 258 and 319: one default meter)",
         {"int numerator = 4;", "sig.denominator = 4;", "int ts_den = 4;"},
         {"int numerator = kDefaultTimeSigNumerator;", "if (sig.denominator == 4) ok = true;",
          "const int numerator = 48;"},
         {},
         {"src/app/preview_view.h", "src/app/preview_view.cpp"}},
        // A test that sets EngineOptions' target ticks runs the engine's
        // targeted search itself. The lines listed test the engine's target
        // mode on hand-built songs (each pins its own answer); a test that
        // wants search_target's answer calls search_target.
        {"Does a test run its own targeted search?",
         "search_target in src/search/pather.cpp",
         R"(\.target_act_ticks\s*=)",
         "",
         {},
         {},
         "step-1 derive-once review of fb1189b, finding 1 (2026-10-04)",
         {"o.target_act_ticks = want;", "options.target_act_ticks = ticks;"},
         {"const std::vector<Path> kept = search_target(song, cfg, want);"},
         {{"src/search/pather.cpp", "options.target_act_ticks = ticks;", "search_target, the owner"},
          {"tests/test_search.cpp", "pinned.target_act_ticks = ticks;",
           "tests EngineOptions' plumbing, not search_target's filter"},
          {"tests/test_search.cpp", "opts.target_act_ticks = std::vector<int64_t>{5760, 17280};",
           "drives the engine's target mode directly on a hand-built song, not search_target's filter"},
          {"tests/test_search.cpp", "opts.target_act_ticks = std::vector<int64_t>{5760, 17280};",
           "drives the engine's target mode directly on a hand-built song, not search_target's filter"},
          {"tests/test_search.cpp", "opts.target_act_ticks = std::vector<int64_t>{5760, 17280};",
           "drives the engine's target mode directly on a hand-built song, not search_target's filter"},
          {"tests/test_search.cpp", "target.target_act_ticks = std::vector<int64_t>{7680};",
           "drives the engine's target mode directly on a hand-built song, not search_target's filter"},
          {"tests/test_search.cpp", "a_then_late.target_act_ticks = std::vector<int64_t>{2304, 7680};",
           "drives the engine's target mode directly on a hand-built song, not search_target's filter"},
          {"tests/test_search.cpp", "opts.target_act_ticks = std::vector<int64_t>{28800};",
           "drives the engine's target mode directly on a hand-built song, not search_target's filter"},
          {"tests/test_search.cpp", "opts.target_act_ticks = std::vector<int64_t>{19200};",
           "drives the engine's target mode directly on a hand-built song, not search_target's filter"},
          {"tests/test_preview_view.cpp", "opts.target_act_ticks = std::vector<int64_t>{28800};",
           "drives the engine's target mode directly on a hand-built song, not search_target's filter"},
          {"tests/test_preview_view.cpp",
           "opts.target_act_ticks = std::vector<int64_t>{5760, 17280};",
           "drives the engine's target mode directly on a hand-built song, not search_target's filter"},
          {"tests/test_replay.cpp", "opts.target_act_ticks = std::vector<int64_t>{28800};",
           "drives the engine's target mode directly on a hand-built song, not search_target's filter"}},
         {"src", "tools", "tests"}},
        // A SqIn's transfer scale sits at its SqIn rank. Indexing the list
        // with a hand-kept counter restates that rank; index it with
        // sqin_rank, or compare whole lists (stored_transfer_scales pairs
        // them). Literal-index pins in tests do not match.
        {"Which transfer scale belongs to this SqIn?",
         "sqin_rank in src/core/model.h",
         R"(\bsqins\[\s*[a-z_]\w*(\+\+)?\s*\])",
         "",
         {},
         {},
         "step-1 derive-once review of de4e23a, finding 2 (2026-10-04)",
         {"rate_note(sq.offset_ms, sq.is_free(), out.scales->sqins[j++], hit_window_ms);",
          "if (j >= scales->sqins.size() || differs(scales->sqins[j], *sq.transfer)) {"},
         {"sq->transfer = scales->sqins[sqin_rank(first, sq, is_sqin_squeeze)];",
          "CHECK(scales->sqins[0].late == doctest::Approx(1.0));"},
         {},
         {"src", "tools", "tests"}},
        {"Which test walks a path's tied variants?",
         "collect_tied in tests/record_fixtures.h",
         R"(^\s*(inline\s+)?void\s+collect_(variants|tied)\s*\()",
         "",
         {},
         {},
         "step-1 derive-once review of de4e23a, finding 4 (2026-10-04)",
         {"void collect_variants(const Path& p, std::vector<const Path*>& out) {",
          "inline void collect_tied(const Path& p, std::vector<const Path*>& out) {"},
         {"void collect_paths(const Path& p, std::vector<const Path*>& out) {"},
         {{"tests/record_fixtures.h",
           "inline void collect_tied(const Path& p, std::vector<const Path*>& out) {",
           "collect_tied, the owner"}},
         {"tests"}},
        // A test that gathers a window's SqIn steps to count or compare them
        // restates D34's check. The check reads the window's SqIn step ticks
        // from sqin_phrase_ticks (src/core/replay.cpp), as check_spent_phrases
        // does, so no line in tests/ matches.
        {"Does each SqIn have its own step, never repeated or squeezed out?",
         "check_one_step_per_sqin in tests/bank_check.h (its ticks from sqin_phrase_ticks)",
         R"(if \(s\.kind == (hydra::)?SpEndKind::SqIn\) (\+\+\w+|\w+\.push_back\(s\.tick\));)",
         "",
         {},
         {},
         "D34; step-1 derive-once review of cc2e1d9, finding 3 (2026-10-04)",
         {"if (s.kind == SpEndKind::SqIn) ticks.push_back(s.tick);",
          "if (s.kind == SpEndKind::SqIn) ++sqin_steps;"},
         {"for (const int64_t t : hydra::sqin_phrase_ticks(act)) sqin_phrases.insert(t);"},
         {},
         {"tests"}},
        {"Which paths are tied under any root of this list?",
         "test::all_tied in tests/record_fixtures.h",
         R"(for \(const (hydra::)?Path& \w+ : [\w.>-]+\) collect_tied\()",
         "",
         {},
         {},
         "step-1 derive-once review of 9a3373b, finding 1 (2026-10-04)",
         {"for (const Path& root : rec.paths) collect_tied(root, vs);",
          "for (const Path& r : rec.paths) collect_tied(r, all);"},
         {"collect_tied(p, tied);"},
         {{"tests/record_fixtures.h", "for (const Path& r : roots) collect_tied(r, out);",
           "test::all_tied, the owner"}},
         {"tests"}},
        {"Is this transfer scale x1.00?",
         "is_scaled in src/core/squeeze_rating.h",
         R"(\.(early|late)\s*[!=]=\s*1\.0\b)",
         "",
         {},
         {},
         "step-1 derive-once review of cc2e1d9, finding 4 (2026-10-04)",
         {"return s.early == 1.0 && s.late == 1.0;",
          "if (act.transfer_pre.early != 1.0 || act.transfer_pre.late != 1.0)"},
         {"CHECK(scales->sqins[0].late == doctest::Approx(1.0));"},
         {},
         {"src", "tools", "tests"}},
        // Was its own test in test_preview_view.cpp (findings 147 and 159):
        // the path gauge's body never touches the chart's phrases, the
        // collected list, the bank count, the cap rule or the squeeze-in rule
        // (that one lives in refill_tick). Scoped to that one function.
        {"Does the path gauge work out an SP fact itself?",
         "the stored facts build_sp_meter_curve reads (bank_rise_ticks, sp_end_steps, "
         "refill_tick)",
         R"(sp_phrases|collected_phrase_ticks|sp_meter\(\)|sp_bars_to_measures|std::min\(|SqIn)",
         "",
         {},
         {},
         "audit findings 147 and 159; step-1 derive-once review of fb1189b, finding 5 "
         "(2026-10-04)",
         {"for (int64_t t : song.sp_phrases()) {", "const int bars = act.sp_meter();",
          "end = std::min(end, cap_end);", "if (s.kind == SpEndKind::SqIn) continue;"},
         {"const int64_t at = act.refill_tick(k);"},
         {},
         {"src"},
         "src/app/preview_view.cpp",
         "SpMeterCurve build_sp_meter_curve("},
        // ---- named UI colours (phase 3 task C4b) ----
        // A colour typed as numbers, in ImVec4 (n / 255.0f) or IM_COL32 form.
        // theme.h names each one once; everything else reads the name.
        {"Which grey is dimmed or disabled text?",
         "kDimTextColor in src/ui/theme.h",
         R"(IM_COL32\(\s*160\s*,\s*160\s*,\s*160\b|\b160\s*/\s*255\.0f?\s*,\s*160\s*/\s*255\.0f?\s*,\s*160\s*/\s*255\b)",
         "",
         {},
         {},
         "audit finding 218, the code half (phase 3 task C4b)",
         {"inline const ImVec4 kDisabledInputTextColor{160 / 255.0f, 160 / 255.0f, 160 / 255.0f, 1.0f};",
          "dl->AddText(pos, IM_COL32(160, 160, 160, 255), label);"},
         {"inline const ImVec4& kDisabledInputTextColor = kDimTextColor;",
          "inline const ImVec4 kNewSongColor{145 / 255.0f, 145 / 255.0f, 145 / 255.0f, 1.0f};"},
         {{"src/ui/theme.h",
           "inline const ImVec4 kDimTextColor{160 / 255.0f, 160 / 255.0f, 160 / 255.0f, 1.0f};",
           "kDimTextColor, the owner"}}},
        // The pattern also catches the old frame/header teal, the second hover
        // teal D74 folded into the button one, so it can't come back.
        {"Which teal is the hover colour for buttons, frames and headers?",
         "kButtonHoveredColor in src/ui/theme.h",
         R"(IM_COL32\(\s*0\s*,\s*10[04]\s*,\s*10[04]\b|\b0(\s*/\s*255\.0f?)?\s*,\s*10[04]\s*/\s*255\.0f?\s*,\s*10[04]\s*/\s*255\b)",
         "",
         {},
         {},
         "audit finding 218, the code half (phase 3 task C4b); D74 item 1, one hover teal",
         {"colors[ImGuiCol_FrameBgHovered] = ImVec4(0, 100 / 255.0f, 100 / 255.0f, 1.0f);",
          "colors[ImGuiCol_HeaderHovered] = ImVec4(0, 104 / 255.0f, 104 / 255.0f, 1.0f);",
          "inline const ImVec4 kFrameHoveredColor{0 / 255.0f, 100 / 255.0f, 100 / 255.0f, 1.0f};",
          "ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(0, 104, 104, 255));"},
         {"colors[ImGuiCol_FrameBgHovered] = kFrameHoveredColor;",
          "inline const ImVec4& kFrameHoveredColor = kButtonHoveredColor;",
          "inline const ImVec4 kButtonActiveColor{0 / 255.0f, 88 / 255.0f, 88 / 255.0f, 1.0f};",
          "IM_COL32(200, 100, 100, 255)"},
         {{"src/ui/theme.h",
           "inline const ImVec4 kButtonHoveredColor{0 / 255.0f, 104 / 255.0f, 104 / 255.0f, 1.0f};",
           "kButtonHoveredColor, the owner"}}},
        {"Which gold is Star Power?",
         "kStarPowerColor in src/ui/theme.h",
         R"(IM_COL32\(\s*255\s*,\s*204\s*,\s*51\b|\b255\s*/\s*255\.0f?\s*,\s*204\s*/\s*255\.0f?\s*,\s*51\s*/\s*255\b)",
         "",
         {},
         {},
         "audit finding 136, the naming half; D48, Q26 (phase 3 task C4b)",
         {"IM_COL32(255, 204, 51, 255), \"SP\");",
          "const ImU32 accent = drain.active ? IM_COL32(255, 204, 51, 255)  // SP gold"},
         {"ImGui::GetColorU32(kStarPowerColor));",
          "inline const ImVec4 kBestPathColor{250 / 255.0f, 210 / 255.0f, 0 / 255.0f, 1.0f};"},
         {{"src/ui/theme.h",
           "inline const ImVec4 kStarPowerColor{255 / 255.0f, 204 / 255.0f, 51 / 255.0f, 1.0f};",
           "kStarPowerColor, the owner"}}},
        // ---- named UI timings (phase 3 task C4b) ----
        // A time-since (now - x, or ImGui::GetTime() - x) compared against a
        // bare number of seconds. AppState names each one beside
        // kFileCheckSeconds; everything else reads the name.
        {"How long does a UI confirmation stay, and how often does the UI re-check?",
         "AppState in src/ui/app_state.h",
         R"(\b(now|GetTime\(\))\s*-\s*[\w.:>-]+\s*[<>]=?\s*\d)",
         "",
         {"src/ui/app_state.h"},
         {},
         "audit finding R7.33; D48, Q33 (phase 3 task C4b)",
         {"if (!shown || now - d.done_at > 0.5) analyze_job.reset();",
          "if (copied_at >= 0.0 && ImGui::GetTime() - copied_at < 2.0) {",
          "(snap.finished || now - batch_refreshed_at_ >= 1.0)) {"},
         {"now - details_ui.file_checked_at >= kFileCheckSeconds) {",
          "if (copied_at >= 0.0 && ImGui::GetTime() - copied_at < AppState::kCopiedSeconds) {"}},
        // ---- how a SqOut refusal names a chord (REPLAYTIE review) ----
        // resolve_sqout_note's refusals list chords through one local
        // lambda; a second hand-written format would drift from it.
        {"How does a typed SqOut refusal name a chord in a list?",
         "the chord_text lambda in resolve_sqout_note, src/core/replay.cpp",
         R"(tick %lld \(%\.2f ms\))",
         "",
         {},
         {},
         "derive-once review of 0997980, finding 1 (D83)",
         {"std::snprintf(one, sizeof(one), \"tick %lld (%.2f ms)\",",
          "std::snprintf(one, sizeof(one), \"%stick %lld (%.2f ms)\", can.empty() ? \"\" : \" or \","},
         {"named += chord_text(tied[i]);",
          "can += chord_text(c);",
          "\"tick %lld (%.2f ms from the SP end)\\n\","},
         {{"src/core/replay.cpp",
           "std::snprintf(one, sizeof(one), \"tick %lld (%.2f ms)\", (long long)ts->timecode.ticks(),",
           "chord_text, the owner"}}},
        // ---- counts and whole-ms timings (phase 3 task O1) ----
        // A test on exactly 1 that picks a word, or a number with " bar" or
        // " bars" typed after it. counted owns the rule and has_have the verb
        // a sentence puts after such a count.
        {"How is a count written next to its noun?",
         "counted (and has_have, for the verb after it) in src/core/model.cpp",
         R"(==\s*1\s*\?\s*("|one\b)|%\w+ bars?\b|" bars?"|group_thousands\([^;]*\)\s*\+\s*" (charts?|records?|songs?|paths?|scores?(?! higher)|rows?|notes?)\b|%[sd] (charts|records|songs|paths|scores|rows|notes)\b)",
         R"(\b(counted|has_have)\()",
         {},
         {},
         "D48, Q12 (audit finding 14; phase 3 task O1; M_D review finding 4)",
         {"return group_thousands(n) + \" \" + (n == 1 ? one : many);",
          "view.lines.push_back(\"SP cap:  \" + std::to_string(*record.sp_cap) + \" bars\");",
          "std::printf(\"SP cap     : %d bars\\n\", settings.sp_cap);",
          "group_thousands(out.stats.total) + \" charts in \" + chartmode + \": \" +",
          "\"Compared %s charts: %s same, %s 1.0 higher, %s 1.1 higher, \""},
         {"view.lines.push_back(\"Path limit:  off\");",
          "s.depth_mode = depth_mode == 1 ? DepthMode::Points : DepthMode::Scores;",
          "ImGui::TextUnformatted(\"bars\");",
          "hydra::counted(report.rows, \"path row\", \"path rows\").c_str());",
          // "score higher" is the verb after a count of charts, not a noun.
          "group_thousands(out.stats.ch11_higher) + \" score higher under 1.1, \" +"},
         {{"src/core/model.cpp", "return group_thousands(n) + \" \" + (n == 1 ? one : many);",
           "counted, the owner"},
          {"src/core/model.cpp", "return n == 1 ? \"has\" : \"have\";", "has_have, the owner"}}},
        // A timing printed as a whole number of ms: printf's %.0f with or
        // without a space before "ms", or a cast, round, lround or llround
        // with " ms" after it.
        {"How is a timing written in whole ms?",
         "format_ms_whole in src/core/model.cpp",
         R"(%\.0f ?ms|static_cast<(long long|long|int|int64_t)>\([^;]*\)\)\s*\+\s*" ms"|\bl{0,2}round\([^;]*\+\s*" ms")",
         R"(\bformat_ms_whole\()",
         {},
         {{"src/core/replay.cpp", "it prints the fixed 500 ms squeeze window, not a timing"},
          {"tools/replay.cpp", "it prints the fixed 500 ms squeeze window, not a timing"}},
         "D48, Q2 (audit findings 4 and 18; phase 3 task O1; M_D review round 2, library "
         "finding 6)",
         {"std::snprintf(buf, sizeof(buf), \"%s %.0f ms\", what, *hardest);",
          "std::to_string(static_cast<long long>(sq.difficulty())) + \" ms\");",
          "\"Effectively %.1fms on the normal %.0fms scale:\\n\"",
          "std::to_string(std::llround(ms)) + \" ms\";",
          "std::to_string(std::lround(ms)) + \" ms\";"},
         {"std::snprintf(buf, sizeof(buf), \"%.1f ms\", ms);",
          "std::snprintf(buf, sizeof(buf), \"%.1f\", ms);",
          "const long long total = std::llround(ms);"},
         {{"src/core/model.cpp", "return std::to_string(std::lround(ms)) + \" ms\";",
           "format_ms_whole, the owner"}}},
        // ---- one name per fill rule (phase 3 task O3a) ----
        // A quoted string (C++ or the reports' JavaScript and HTML) whose
        // first word is a game version's name is a label for a fill rule. A
        // sentence that only mentions the version later on is prose, and a
        // trailing // comment is not a label, so neither is flagged. The
        // "1.0 fills" checkbox keeps its own words (docs/adr/0010).
        {"Which rule names a fill deadline?",
         "fill_rule_name in src/search/graph.h",
         R"(^(?:(?!//).)*["'][^"'\w]*(Clone Hero|CH) 1\.[01]\b)",
         R"(\bfill_rule_(name|description)\()",
         {},
         {},
         "audit finding 55; D48, Q13 (phase 3 task O3a)",
         {"out.fills = s.legacy_fills ? \"Clone Hero 1.0\" : \"Clone Hero 1.1\";",
          "{k:'s10',     t:'CH 1.0',      num:true},",
          "<h1>Fill spawn <span class=\"accent\">CH 1.0 vs CH 1.1</span></h1>",
          "if (options.lens.legacy_fills) cap_label += \" — Clone Hero 1.0 fills\";"},
         {"const char* ch10 = hydra::engine_mode_stamp(hydra::FillDeadlineRule::Ch10);",
          "help_marker(\"Spawn drum fills by Clone Hero 1.0's rule instead of 1.1's. A fill only \"",
          "\"--legacy-fills would write Clone Hero 1.0 results into the \"",
          "std::string fills;        // \"Clone Hero 1.1\" or \"Clone Hero 1.0\"",
          "ImGui::Checkbox(\"1.0 fills\", &s.legacy_fills);",
          "out.fills = hydra::fill_rule_name(rule, hydra::FillRuleNameStyle::Long);"},
         {{"src/search/graph.h",
           "if (rule == FillDeadlineRule::Ch10) return is_short ? \"CH 1.0\" : \"Clone Hero 1.0\";",
           "fill_rule_name, the owner: Clone Hero 1.0's two names"},
          {"src/search/graph.h", "return is_short ? \"CH 1.1\" : \"Clone Hero 1.1\";",
           "fill_rule_name, the owner: Clone Hero 1.1's two names"}}},
        // A legacy_fills flag (a setting, a Lens or a switch) turned into a
        // rule by a ternary of its own.
        {"Which fill rule does a legacy_fills flag mean?",
         "fill_rule_for in src/search/graph.h",
         R"(legacy\w*\s*\?\s*(hydra::)?FillDeadlineRule::Ch1[01])",
         R"(\bfill_rule_for\()",
         {},
         {},
         "M_D review finding 3 and round 2 finding 5 (phase 3 tasks FX-R and FX2-R); "
         "widened to tests by audit finding 177 / phase 6 task J4-1 (D53, D54)",
         {"options.lens.legacy_fills ? FillDeadlineRule::Ch10 : FillDeadlineRule::Ch11;",
          "return legacy_fills ? FillDeadlineRule::Ch10 : FillDeadlineRule::Ch11;"},
         {"fill_rule_for(settings.legacy_fill_deadline), settings.rules);"},
         {{"src/search/graph.h",
           "return legacy_fills ? FillDeadlineRule::Ch10 : FillDeadlineRule::Ch11;",
           "fill_rule_for, the owner"}},
         {"src", "tools", "tests"}},
        // A command-line tool that reads the app's INI by plain load() keeps
        // the app's "1.0 fills" box, so its run (and a bench digest) changes
        // with whatever the GUI was left on.
        {"Which fill rule does a command-line run use?",
         "Settings::load_for_command_line in src/app/config.cpp",
         R"(\bSettings::load\(\))",
         "",
         {"src/app/config.cpp"},
         {{"src/ui/app_state.cpp", "the GUI is where the 1.0 fills setting lives"},
          {"src/cli/report.cpp",
           "hydra_report follows the app's setting, or a database's 1.0 stamp, by design "
           "(docs/adr/0010)"},
          {"src/cli/fillcompare.cpp",
           "hydra_fillcompare sets each side's fill rule itself (collect_fill_rows in "
           "src/app/fill_report.cpp)"}},
         "perf follow-up B (docs/handoffs/2026-10-06-perf-speedups-wave2-handoff.md)",
         {"hydra::app::Settings settings = hydra::app::Settings::load();",
          "app::Settings st = app::Settings::load();"},
         {"hydra::app::Settings settings = hydra::app::Settings::load_for_command_line(legacy_fills);",
          "app::Settings st = app::Settings::load_for_command_line(g_legacy_fills);"},
         {}},
        // A database's engine_mode stamp compared by hand: through the
        // store's accessor, or any line spelling a rule's stamp text, which
        // only engine_mode_stamp in search/graph.h may write.
        {"Which fill rule does a database's stamp name?",
         "fill_rule_from_stamp in src/search/graph.h",
         R"(engine_mode\(\)\s*==|"ch1[01]")",
         R"(\bfill_rule_from_stamp\()",
         {"src/search/graph.h"},
         {},
         "M_D review round 2 findings 2 and 5 (phase 3 task FX2-R); widened by the M7-2b "
         "review, finding 4",
         {"if (store->engine_mode() == std::string(",
          "const std::string legacy = mode && *mode == \"ch10\" ? \"1\" : \"0\";",
          "legacy_fills = stamp == \"ch10\" ? \"1\" : \"0\";",
          "if (m == \"ch11\") return;"},
         {"if (mode && hydra::fill_rule_from_stamp(*mode) != expected)",
          "legacy_fills = stamped_fill_rule() == FillDeadlineRule::Ch10 ? \"1\" : \"0\";"}},
        // The meta key the stamp is stored under, typed anywhere but its one
        // constant. RecordStore::engine_mode and set_engine_mode read and
        // write through it.
        {"Which meta key holds the fill-rule stamp?",
         "kEngineModeKey in src/store/record_store.cpp",
         R"("engine_mode")",
         "",
         {},
         {},
         "audit R7.26 (M7-2b review, finding 1; the key typed once by phase 6 task J4-2)",
         {"if (const std::optional<std::string> mode = meta_get(\"engine_mode\"))",
          "return meta_get(\"engine_mode\");", "meta_set(\"engine_mode\", mode);"},
         {"if (const std::optional<std::string> mode = engine_mode())",
          "return meta_get(kEngineModeKey);", "meta_set(kEngineModeKey, mode);"},
         {{"src/store/record_store.cpp", "constexpr const char* kEngineModeKey = \"engine_mode\";",
           "kEngineModeKey, the owner"}}},
        // The charts table grouped by md5 to pick a copy, outside the one
        // query that does it.
        {"Which copy names a chart the scan found twice?",
         "kNamingCopiesSql in src/store/record_store.h",
         R"(MIN\(rowid\)|GROUP BY md5)",
         "",
         {},
         {},
         "D51 call 10 and D63 (task ST2)",
         {"\" FROM (SELECT md5, name, artist, charter, MIN(rowid) FROM charts GROUP BY md5)\""},
         {"kNamingCopiesSql + \" AS c WHERE songmeta.hyhash = c.md5\")"},
         {{"src/store/record_store.h",
           "\"(SELECT md5, name, artist, charter, MIN(rowid) AS naming_rowid, COUNT(*) AS copies\"",
           "kNamingCopiesSql, the owner"},
          {"src/store/record_store.h", "\" GROUP BY md5)\";", "kNamingCopiesSql, the owner"}}},
        // A pass over the library that picks a chart's file from its own
        // listing, instead of the naming copy, can open another copy's file
        // than the batch did (storage-T3 review, finding 1).
        {"Which library copy of a chart does a pass analyze?",
         "kNamingCopiesSql in src/store/record_store.h (the naming copy), read through "
         "RecordStore::naming_copy_paths",
         R"(\.emplace\(normalize_chart_hash\(\w+\.md5\),\s*&\w+\))",
         "",
         {},
         {},
         "D51 call 10, D63 and D76 (storage-T3 review, finding 1)",
         {"for (const store::ChartLibraryEntry& e : library) files.emplace(normalize_chart_hash(e.md5), &e);",
          "by_hash.emplace(normalize_chart_hash(entry.md5), &entry);"},
         {"const auto naming = naming_copy_files.find(normalize_chart_hash(listing.hyhash));",
          "files.emplace(normalize_chart_hash(md5), std::move(path));"},
         {},
         {"src"}},
        // ---- one cleaned song title (phase 3 task O3a) ----
        // Clone Hero's rich-text tags spelled as text: a tag in angle
        // brackets at the start of a string, a tag name kept in a named
        // constant, or a row of a tag table. Config keys that happen to be
        // called "color" or "size" are not flagged.
        {"Which tags does Hydra strip from a song name?",
         "strip_rich_tags in src/parse/song.cpp, read through display_title "
         "(and display_artist, which applies display_title then the artist placeholder, D50 "
         "item 5 and D56 item 2)",
         R"re("</?(color|size|b|i|u|s|sub|sup)\b|=\s*"(color|size|sub|sup)"\s*;|\{\s*"(color|size|b|i|u|s|sub|sup)"\s*,\s*(true|false)\s*\})re",
         R"(\b(strip_rich_tags|display_title|display_artist)\()",
         {},
         {},
         "audit findings 8 and 111; D48, Q14 (phase 3 task O3a)",
         {"static const char* kWord = \"color\";",
          "{\"color\", true}, {\"size\", true}, {\"b\", false},   {\"i\", false},",
          "out = replace_all(out, \"</b>\", \"\");"},
         {"row.title = app::strip_rich_tags(entry.title);",
          "get_color(sub(track, \"color\"), \"normal\", c.track.color_normal);",
          "get_f(time_box, \"size\", c.text.time_box_size);",
          "<div class=\"sub\">__SUBTITLE__</div>"},
         {{"src/parse/song.cpp",
           "{\"color\", true}, {\"size\", true}, {\"b\", false},   {\"i\", false},",
           "strip_rich_tags' tag table, the owner"},
          {"src/parse/song.cpp",
           "{\"u\", false},    {\"s\", false},   {\"sub\", false}, {\"sup\", false},",
           "strip_rich_tags' tag table, the owner"}}},
        // A charter cleaned for showing by hand: the tags stripped (and maybe
        // trimmed) straight into a charter. The Library's search text keeps
        // the stored charter by decision, so folding it is not flagged.
        {"Which charter text does a screen show?",
         "display_charter in src/parse/song.cpp",
         R"(charter\s*=\s*(trim\()?(app::)?strip_rich_tags\()",
         R"(\bdisplay_charter\()",
         {},
         {},
         "M_D review finding 1 and round 2 finding 5 (phase 3 tasks FX-R and FX2-R)",
         {"row.charter = trim(strip_rich_tags(meta.ref_charter));",
          "const std::string charter = app::strip_rich_tags(song.charter);"},
         {"row.charter = display_charter(rec->ref_charter);",
          "row.charter = fold_for_search(strip_rich_tags(charter));"}},
        // ---- app and UI helper owners (phase 3 task O3b) ----
        // The out-of-date sentence, by any of its pieces: the cause it names
        // or the "Re-analyze to refresh" it ends on. stale_text names the
        // store's real cause; every screen shows its sentence.
        {"Why is a stored result out of date?",
         "stale_text in src/app/user_messages.cpp",
         R"(another Hydra version|different rules in hydra_rules|Re-analyze to refresh|run a batch to refresh)",
         "",
         {},
         {},
         "audit finding 13; D48, Q17 (phase 3 task O3b)",
         {"hint(\"Analyzed by another Hydra version, or under different rules in \"",
          "\"from different rules in hydra_rules.ini. Re-analyze to refresh it.\");",
          "\"hydra_rules.ini. Re-analyze to refresh.\");"},
         {"} else if (status == store::RecordStatus::Stale) {",
          "\"hydra_rules.ini has a line Hydra can't read. Fix or delete that line, then restart \""},
         {{"src/app/user_messages.cpp",
           "cause = \"another Hydra version or from different rules in hydra_rules.ini\";",
           "stale_text, the owner: both causes (today's sentence)"},
          {"src/app/user_messages.cpp", "cause = \"another Hydra version\";",
           "stale_text, the owner: another Hydra version"},
          {"src/app/user_messages.cpp", "cause = \"different rules in hydra_rules.ini\";",
           "stale_text, the owner: different rules"},
          {"src/app/user_messages.cpp",
           "\". Click the song or run a batch to refresh it.\";",
           "stale_text, the owner: the sentence's ending (D87 item 6)"}}},
        // Whether a chart file changed since its library row was made. The
        // Preview's load and the click's job both ask chart_changed_since, so
        // the two never disagree; the fingerprint and the hash have no other
        // caller in src or tools.
        {"Has a chart file changed since its library row was made?",
         "chart_changed_since in src/app/analysis.cpp",
         R"(\b(hash_chart_file|chart_files_unchanged|chart_files_sig)\()",
         "",
         {"src/app/analysis.h", "src/app/analysis.cpp"},
         {},
         "storage-T2 review finding 2 (D87 item 3)",
         {"if (app::chart_files_unchanged(entry_.notespath, entry_.sig)) return false;",
          "const std::string hash = app::hash_chart_file(entry_.notespath);",
          "out_.new_sig = app::chart_files_sig(song_.notespath);"},
         {"app::chart_changed_since(entry_.notespath, entry_.sig);"},
         {},
         // src only: tools/replay.cpp hashes a chart to look it up by path,
         // which is a different question.
         {"src"}},
        // The not-analyzed row's tooltip. The old copy was split over two
        // source lines, so a rule on the whole sentence missed it; this one
        // matches the sentence's first words, which sit on one line however
        // the rest is split. kNotAnalyzedText is the one place it is typed.
        {"What does a library row with no result say when hovered?",
         "kNotAnalyzedText in src/app/user_messages.h",
         R"(Not analyzed yet)",
         "",
         {"src/app/user_messages.h"},
         {},
         "D91 (2026-10-07)",
         {"ImGui::SetTooltip(\"Not analyzed yet. Open the song and press \\\"Analyze this \"",
          "\"Not analyzed yet. Click the song or run a batch to analyze it.\";"},
         {"ImGui::SetTooltip(\"%s\", app::kNotAnalyzedText);"},
         {}},
        // The Preview's line for a chart file that changed after it was
        // analyzed. It is typed once, in preview_tab.cpp; the User Guide
        // quotes it. The owner line pins D94's words.
        {"What does the Preview say when the chart changed since it was analyzed?",
         "the changed-chart line in src/ui/preview_tab.cpp",
         R"(This chart changed since|\bwas analyzed\.)",
         "",
         {},
         {},
         "D51 call 18; D94 (2026-10-07)",
         {"\"This chart changed since it was analyzed. Analyze it again to see its path.\");",
          "\"This chart changed since it \"",
          "\"was analyzed. Click the song again to see its path.\");"},
         {"// Has the chart changed since it was analyzed (finding 126)? First the",
          "const bool changed = app::chart_changed_since(entry_.notespath, entry_.sig);"},
         {{"src/ui/preview_tab.cpp",
           "\"This chart changed since it was analyzed. Click the song again to see its path.\");",
           "the owner (D94)"}}},
        // Cutting a label to end in "…": ImGui's own ellipsis renderer, its
        // ellipsis glyph, the "…" bytes typed out as escapes, or a "…" typed
        // straight into a string before any // comment. ellipsize is the one
        // rule (no trailing space, an exact fit allowed); text_ellipsized
        // cuts through it. The last alternative is built from the three
        // UTF-8 bytes of "…", so this file holds no raw one in a pattern.
        {"How is a long label cut to fit its space?",
         "ellipsize in src/render/overlay_layout.cpp",
         R"(RenderTextEllipsis|EllipsisChar|\\xE2\\x80\\xA6|^(?:(?!//).)*"(?:(?!//)[^"])*)"
         "\xE2\x80\xA6",
         "",
         {},
         {},
         "audit finding 70; D48, Q18 (phase 3 task O3b)",
         {"font->RenderChar(draw, size, ImVec2(IM_TRUNC(pos.x + kept_w), pos.y), col, font->EllipsisChar);",
          "ImGui::RenderTextEllipsis(window->DrawList, pos, ImVec2(max_x, pos.y + text_size.y),",
          R"(return text.substr(0, end) + "\xE2\x80\xA6";)",
          "return text.substr(0, end) + \"\xE2\x80\xA6\";",
          "const char* tail = \"\xE2\x80\xA6\";"},
         {"text_ellipsized(text.c_str());",
          "const std::string shown = ellipsize(label, w, ten_per_char);",
          "// ends in \"\xE2\x80\xA6\" rather than vanish",
          "label = \"a\";  // ends in \xE2\x80\xA6 when cut"},
         {{"src/render/overlay_layout.cpp",
           R"(static const std::string kEllipsis = "\xE2\x80\xA6";)",
           "ellipsize, the owner: the one ellipsis a cut ends in"}}},
        // ---- M_D review follow-ups (phase 3 task FX-L) ----
        // A switch over the Dynamics rows, the 2x test that picks a kick
        // row, or a test of a row against a named row (the old cymbal-row
        // hide was "r == app::DynamicsRow::YellowCymbal || ..."): a second
        // table of which note each row holds. The one table is
        // kDynamicsRows in dynamics_breakdown.cpp, read through
        // dynamics_row_info and dynamics_row_for.
        {"Which table says what a Dynamics row holds?",
         "kDynamicsRows in src/app/dynamics_breakdown.cpp",
         R"(case (app::)?DynamicsRow::\w+:|return note\.is2x \? DynamicsRow::|==\s*(app::)?DynamicsRow::\w+)",
         "",
         {},
         {},
         "M_D review, library finding 1, round 2 library finding 6 (phase 3 tasks FX-L, FX2-L)",
         {"case app::DynamicsRow::GreenTom:     return ImVec4(0.15f, 0.75f, 0.20f, 1.0f);",
          "case DynamicsRow::GreenCymbal:  return pad(NoteColor::Green, NoteCymbalType::Cymbal);",
          "return note.is2x ? DynamicsRow::Kick2x : DynamicsRow::Kick;",
          "if (!pro && (r == app::DynamicsRow::YellowCymbal ||"},
         {"ImVec4 dot = pad_color(r);",
          "const DynamicsRowInfo& info = dynamics_row_info(r);"},
         {{"src/app/dynamics_breakdown.cpp", "if (r == DynamicsRow::Count) return std::string();",
           "dynamics_row_label's guard: Count marks the end of the rows and names no row"}}},
        // "Is the typed search narrowing the library?" asked of the query,
        // from outside the model or inside it. LibraryModel::searching
        // answers it, from the model's own query_.
        {"Is the typed search narrowing the library?",
         "LibraryModel::searching in src/ui/library_model.h",
         R"(\bquery(\(\)|_)\.empty\(\))",
         "",
         {},
         {},
         "M_D review, library finding 5, round 2 library finding 5 (phase 3 tasks FX-L, FX2-L)",
         {"const bool searching = !app.library.query().empty();",
          "if (app.library.query().empty()) return;",
          "if (query_.empty()) return sorted_;"},
         {"app.library.set_query(search);",
          "const bool searching = app.library.searching();",
          "if (!searching()) return sorted_;"},
         {{"src/ui/library_model.h", "bool searching() const { return !query_.empty(); }",
           "searching, the owner"}}},
        // A time turned into a tick outside the timing code. display_tick_at_ms
        // is the one rule for which tick a screen shows at a time (D48, Q23).
        // The row flags every raw tick_at_ms call, so a rounding of any kind
        // (llround, round, floor, a cast) is caught, and so is one split over
        // two lines, whose second line still holds the call.
        {"Which tick does a screen show at a time?",
         "SongTiming::display_tick_at_ms in src/core/timing.cpp",
         R"(\btick_at_ms\()",
         "",
         {},
         {},
         "D48, Q23 (audit finding 183; M_D review, reports finding 7, round 2 library finding 6)",
         {"const int64_t end_tick = std::llround(timing->ms_index().tick_at_ms(*song_length_ms));",
          "const int64_t t = std::round(timing->ms_index().tick_at_ms(ms));",
          "const int64_t t = static_cast<int64_t>(ms_.tick_at_ms(ms));",
          // The second line of "std::llround(\n timing->ms_index().tick_at_ms(ms));".
          "timing->ms_index().tick_at_ms(ms));"},
         {"const int64_t end_tick = timing->display_tick_at_ms(*song_length_ms);"},
         {{"src/core/timing.cpp", "const int64_t tick = std::llround(ms_.tick_at_ms(ms));",
           "display_tick_at_ms, the owner"},
          {"src/core/timing.cpp", "double MsIndex::tick_at_ms(double ms) const {",
           "the ms-to-tick map itself"},
          {"src/core/timing.h", "double tick_at_ms(double ms) const;",
           "the ms-to-tick map's declaration"},
          {"src/core/timing.cpp", "double t = ms_.tick_at_ms(act_hit_ms);",
           "sp_end_ms keeps the fractional tick for its continuous map; no screen shows it"}}},
        // ---- phase 3 wave C owners (derive-once review of M_C) ----
        // A status word typed in quotes. The library chips, the Best path
        // cell and the uitest state dump all ask status_label.
        {"Which word names a record's status?",
         "status_label in src/ui/library_model.cpp",
         R"re("(Not analyzed|Analyzed|Stale)")re",
         "",
         {},
         {},
         "audit finding 87; derive-once review of M_C (6d86f1c), finding 2 (2026-10-04)",
         {"{StatusChip::Stale, \"Stale\", \"chipstale\"},",
          "case store::RecordStatus::Ready: return \"Analyzed\";"},
         {"return status_label(status);", "{StatusChip::All, \"chipall\"},"},
         {{"src/ui/library_model.cpp", "case store::RecordStatus::Ready: return \"Analyzed\";",
           "status_label, the owner"},
          {"src/ui/library_model.cpp", "case store::RecordStatus::Stale: return \"Stale\";",
           "status_label, the owner"},
          {"src/ui/library_model.cpp", "return \"Not analyzed\";", "status_label, the owner"}}},
        // A filter chip's button id typed as text. chip_label builds the whole
        // label (word, count, id), and render_chips and the GUI tests ask it.
        // src only: the GUI tests also pin whole labels as literals, which a
        // text search cannot tell from a label built by hand, so the test that
        // looks chips up by a computed label is reviewed, not scanned.
        {"What does a status chip's button label read?",
         "chip_label in src/ui/library_model.cpp",
         R"re("chip(all|new|stale|done)")re",
         "",
         {},
         {},
         "storage-T2 sign-off (library-layout chip refs)",
         {"{StatusChip::All, \"chipall\"},", "{StatusChip::Analyzed, \"chipdone\"},"},
         {"const std::string label = chip_label(chip, n);",
          "if (chip == StatusChip::Stale) hint(app::stale_text(true, true).c_str());"},
         {{"src/ui/library_model.cpp", "const char* id = \"chipall\";", "chip_label, the owner"},
          {"src/ui/library_model.cpp", "case StatusChip::NotAnalyzed: id = \"chipnew\"; break;",
           "chip_label, the owner"},
          {"src/ui/library_model.cpp", "case StatusChip::Stale: id = \"chipstale\"; break;",
           "chip_label, the owner"},
          {"src/ui/library_model.cpp", "case StatusChip::Analyzed: id = \"chipdone\"; break;",
           "chip_label, the owner"}},
         {"src"}},
        // Two paths compared by score and notation, or keyed by notation and
        // score glued together, instead of by path_identity.
        {"Is this the same path as that one?",
         "path_identity in src/core/model.cpp",
         R"re(totalscore\(\)\s*==[^;]*pathstring\(\)|pathstring_verbose\(\{\}\)\s*\+\s*"\|")re",
         "",
         {},
         {},
         "audit finding 249; derive-once review of M_C (6d86f1c), proposed scan row 2 "
         "(2026-10-04)",
         {"if (p->totalscore() == score && p->pathstring() == notation)",
          "return path->pathstring_verbose({}) + \"|\" + std::to_string(path->totalscore());"},
         {"if (path_identity(*p) == identity) return view;"}},
        // A moment's ms over the song length, clamped to 0..1, worked out
        // anywhere but song_fraction.
        {"How far into the song is this moment, as a share?",
         "song_fraction in src/app/preview_view.cpp",
         R"re(std::clamp\([^;]*/\s*\*?\w*length\w*\s*,\s*0\.0\s*,\s*1\.0)re",
         "",
         {},
         {},
         "phase 3 task C4a; derive-once review of M_C (6d86f1c), finding 4 (2026-10-04)",
         {"const double at = std::clamp(at / *song_length_ms, 0.0, 1.0);",
          "marks.push_back(std::clamp(a.ms / length_ms, 0.0, 1.0));"},
         {"marks.push_back(*song_fraction(a.ms, length_ms));"},
         {{"src/app/preview_view.cpp", "return std::clamp(ms / length_ms, 0.0, 1.0);",
           "song_fraction, the owner"}}},
        // The two-hit budget written out as twice the window (either way
        // round), as squeeze_budget_ms at a hand-typed identity scale, or the
        // backend rescale written out as 2 / (1 + r), instead of
        // nominal_budget_ms and squeeze_budget_ms.
        {"What is the two-hit budget at the identity scale?",
         "nominal_budget_ms beside squeeze_budget_ms in src/core/squeeze_rating.cpp",
         R"re(\b2(\.0)?\s*\*\s*(w|hit_window\w*|kDefaultHitWindowMs)\b|\b(w|hit_window\w*|kDefaultHitWindowMs)\s*\*\s*2(\.0)?\b|squeeze_budget_ms\(\s*(1(\.0)?|kIdentityScale)\s*,|\*\s*2\.0\s*/\s*\(1\.0\s*\+)re",
         "",
         {},
         {},
         "phase 3 task C4c; derive-once review of M_C (6d86f1c), finding 3, and round 2, "
         "finding 2 (2026-10-04)",
         {"{\"Insane+\", \"t4\", 2 * w},",
          "{\"Insane+\", \"t4\", squeeze_budget_ms(1.0, w)},",
          "const double budget = hit_window_ms * 2;",
          "return std::abs(offset_ms) * 2.0 / (1.0 + transfer_r);"},
         {"{\"Insane+\", \"t4\", nominal_budget_ms(w)},"},
         {{"src/core/squeeze_rating.cpp",
           "return squeeze_budget_ms(kIdentityScale, hit_window_ms);",
           "nominal_budget_ms, the owner"}}},
        // A ByteSource's reads come from a file or from memory, and tests
        // count them through one wrapper; a lambda written elsewhere would be
        // a second reader of the same bytes.
        {"How does a ByteSource read its bytes?",
         "file_byte_source and memory_byte_source in src/core/winstr.cpp; "
         "testbytes::counting in tests/byte_source_util.h",
         R"(\.read\s*=\s*\[)",
         "",
         {"src/core/winstr.cpp", "tests/byte_source_util.h"},
         {},
         "container ranged-reads derive-once review of cbd1ee6, finding 2 (2026-10-04)",
         {"src.read = [file, total = *size](uint64_t offset, size_t length) {",
          "out.read = [inner, &bytes_read](uint64_t offset, size_t length) {"},
         {"const std::vector<uint8_t> b = src.read(0, 16);"},
         {},
         {"src", "tools", "tests"}},
        {"When are two Songs the same in a test?",
         "testsong::songs_equal in tests/song_equal.h",
         R"((bool|void)\s+\w*(same|equal)\w*\(const (hydra::)?Song&|\.sequence\[i\]\.timecode\.ticks\(\)\s*==)",
         "",
         {},
         {},
         "audit finding 122; container ranged-reads derive-once review of cbd1ee6, "
         "finding 1 (2026-10-04)",
         {"void check_same_notes(const Song& got, const Song& want) {",
          "CHECK(via_bytes.sequence[i].timecode.ticks() == via_path.sequence[i].timecode.ticks());"},
         {"CHECK(testsong::songs_equal(direct, via_srb));"},
         {{"tests/song_equal.h",
           "inline bool songs_equal(const hydra::Song& a, const hydra::Song& b) {",
           "testsong::songs_equal, the owner"}},
         {"tests"}},
        {"How much of a container is read first?",
         "kFirstPieceRead in src/core/winstr.h (reads then grow by next_piece_read)",
         R"(constexpr\s+size_t\s+k\w*First\w*Read\s*=)",
         "",
         {},
         {},
         "ADR 0024 (the user chose 64 KB, then double, on 2026-10-04)",
         {"constexpr size_t kSngFirstRead = 64 * 1024;", "constexpr size_t kFirstRead = 64 * 1024;"},
         {"std::vector<uint8_t> head = src.read(0, kFirstPieceRead);"},
         {{"src/core/winstr.h", "constexpr size_t kFirstPieceRead = 64 * 1024;",
           "kFirstPieceRead, the owner"}},
         {"src", "tools", "tests"}},
        // A file can shrink after it was sized, so comparing what was read
        // with the stated size can wait forever (sng_read_head did).
        {"Has a ByteSource run out?",
         "the short-read rule on ByteSource in src/core/winstr.h",
         R"(\w+\.size\(\)\s*(<=?|>=?|[!=]=)\s*\w+\.size\b(?!\()|\w+\.size\s*(<=?|>=?|[!=]=)\s*\w+\.size\(\))",
         "",
         {},
         {},
         "container ranged-reads derive-once reviews of 8229b6e (finding 1) and 860cbd8 "
         "(finding 3), 2026-10-04",
         {"if (needed <= head.size() || head.size() == src.size) return head;",
          "if (bytes.size() != src.size) return std::nullopt;",
          "if (head.size() >= src.size) return head;", "if (src.size == head.size()) return head;"},
         {"if (bytes.size() != n) return std::nullopt;  // the file shrank since it was opened",
          "if (needed <= head.size() || head.size() < asked) return head;"},
         {},
         {"src", "tools", "tests"}},
        {"What are all of a ByteSource's bytes?",
         "read_all in src/core/winstr.cpp",
         R"(\.read\(\s*0\s*,\s*((static_cast<size_t>\(|\(size_t\)\s*)?\w+\.size\)?|SIZE_MAX|std::numeric_limits<size_t>::max\(\))\s*\))",
         "",
         {},
         {},
         "container ranged-reads derive-once reviews of 8229b6e (finding 2) and 860cbd8 "
         "(finding 3), 2026-10-04",
         {"return load_songbytes_mid(src.read(0, static_cast<size_t>(src.size)), pro, bass2x,",
          "std::vector<uint8_t> all = src.read(0, src.size);",
          "std::vector<uint8_t> all = src.read(0, SIZE_MAX);",
          "std::vector<uint8_t> all = src.read(0, (size_t)src.size);"},
         {"std::vector<uint8_t> head = src.read(0, asked);"},
         {{"src/core/winstr.cpp", "return src.read(0, static_cast<size_t>(src.size));",
           "read_all, the owner"}},
         {"src", "tools", "tests"}},
        {"How many bytes from a position does a buffer hold?",
         "range_length in src/core/winstr.cpp",
         R"(std::min\(\s*\w+\s*-\s*\w+\s*,\s*k\w+\)|\w+\s*>=\s*\w+\)\s*return\s*\{\s*nullptr\s*,\s*0\s*\})",
         "",
         {},
         {},
         "container ranged-reads derive-once review of 860cbd8, finding 2 (2026-10-04)",
         {"if (pos >= size) return {nullptr, 0};",
          "const size_t n = std::min(size - pos, kMaxPiece);"},
         {"return static_cast<size_t>(std::min<uint64_t>(length, available));"},
         {},
         {"src"}},
        // ---- the Preview (derive-once review of M_D, phase 3 task FX-P) ----
        // The last drawn note, or the last timestamp's onset, read anywhere
        // but last_drawn_note. The song's length has its own owner
        // (app::song_length_ms, D75), whose backup is listed below.
        {"When is the song's last note?",
         "last_drawn_note in src/app/preview_view.cpp",
         R"(notes\.back\(\)|sequence\.back\(\)\.timecode\.ms\(\))",
         "",
         {},
         {},
         "audit finding 9; derive-once review of M_D, preview finding 1 (phase 3 task FX-P); "
         "re-pointed by D69 (phase 7 task AL1)",
         {"if (scene.has_notes) scene.song_length_ms = scene.notes.back().ms;",
          "return song.sequence.back().timecode.ms();",
          "const int64_t last_tick = scene.notes.back().tick;"},
         {"scene.song_length_ms = song_length_ms.value_or(0.0);", "const int64_t last_tick = last->tick;"},
         {{"src/app/preview_view.cpp", "return scene.notes.empty() ? nullptr : &scene.notes.back();",
           "last_drawn_note, the owner"}}},
        // Frames times 1000 over a sample rate, or ms times a rate over 1000,
        // written out instead of calling the frames helpers.
        {"How many ms do audio frames last, and how many frames do ms hold?",
         "ms_of_frames and frames_of_ms in src/audio/frames.h",
         R"(\*\s*1000(\.0)?\s*/\s*[\w.>()-]*([sS]ample_?[rR]ate|[rR]ate)|\*\s*[\w.>-]*([sS]ample_?[rR]ate|[rR]ate)\w*(\(\))?\s*/\s*1000(\.0)?\b)",
         "",
         {},
         {},
         "audit findings 182 and R7.21; derive-once review of M_D, preview finding 2 (phase 3 "
         "task FX-P)",
         {"seek_frames(static_cast<int64_t>(std::llround(ms * sample_rate_ / 1000.0)));",
          "return sample_rate_ > 0 ? length_ * 1000.0 / sample_rate_ : 0.0;",
          "return static_cast<double>(audio.length_frames()) * 1000.0 / audio.sample_rate() -",
          "front_pad = static_cast<int64_t>(std::llround(-offset_ms * kOutRate / 1000.0));"},
         {"return audio::ms_of_frames(audio.length_frames(), audio.sample_rate()) - audio_offset_ms;",
          "front_pad = audio::frames_of_ms(-offset_ms, kOutRate);",
          "const double seconds = ms / 1000.0;"},
         {{"src/audio/frames.h",
           "return sample_rate > 0 ? static_cast<double>(frames) * 1000.0 / sample_rate : 0.0;",
           "ms_of_frames, the owner"},
          {"src/audio/frames.h",
           "return static_cast<int64_t>(std::llround(ms * sample_rate / 1000.0));",
           "frames_of_ms, the owner"}}},
        // A note's ms compared with the playhead by hand instead of through
        // struck_at.
        {"Is a note struck with the playhead at now?",
         "struck_at in src/app/preview_view.h",
         R"(\b\w+\.ms\s*<=\s*now\w*\b|now\w*\s*>=\s*\w+\.ms\b|\bnote_ms\s*<=\s*now_ms\b)",
         "",
         {},
         {},
         "D48, Q27; derive-once review of M_D, preview finding 4 (phase 3 task FX-P)",
         {"if (a.has_sp_end && a.ms <= now && now < a.sp_end_ms) return &a;",
          "if (now_ms >= s.ms) last = &s;", "return note_ms <= now_ms;"},
         {"if (w && struck_at(now, w->first) && now < w->second) return &a;",
          "return struck_at(now, s.ms);", "if (now < a.sp_end_ms) return &a;"},
         {{"src/app/preview_view.h",
           "inline bool struck_at(double now_ms, double note_ms) { return note_ms <= now_ms; }",
           "struck_at, the owner"}}},
        // A measuring lambda handed to ellipsize with text_width's own body.
        {"How wide does a string draw in the current font?",
         "text_width in src/ui/widgets.h",
         R"(\[[^\]]*\]\s*\(const std::string&\s*\w+\)\s*\{\s*return ImGui::CalcTextSize\(\w+\.c_str\(\)\)\.x;)",
         "",
         {},
         {},
         "derive-once review of M_D, library finding 4 (phase 3 task FX-P)",
         {"current, box_w - chrome, [](const std::string& s) { return ImGui::CalcTextSize(s.c_str()).x; });"},
         {"const std::string shown = render::ellipsize(current, box_w - chrome, text_width);",
          "auto text_width = [font](float sz, const char* s) {"}},
        // ---- the Preview, round 2 (derive-once review of M_D, phase 3 task FX2-P) ----
        // The transport's length read straight off it, beside the accessor
        // that names it as where playback stops.
        {"Where does the Preview's playback stop?",
         "PreviewController::playback_end_ms in src/ui/preview_controller.cpp",
         R"(\btransport_?\.length_ms\(\))",
         "",
         {},
         {},
         "D50 item 4 and D56 item 3; derive-once review of M_D round 2, preview finding C (phase 3 "
         "task FX2-P)",
         {"transport_.length_ms());",
          "return hydra::app::build_time_box(scene_, transport_.now_ms(), transport_.length_ms());"},
         {"return hydra::app::build_time_box(scene_, transport_.now_ms(), playback_end_ms());",
          "scrub_marks_cache_.length_ms = length;"},
         {{"src/ui/preview_controller.cpp",
           "double PreviewController::playback_end_ms() const { return transport_.length_ms(); }",
           "PreviewController::playback_end_ms, the owner"}}},
        // A progress bar's "42%" overlay typed beside the widget that draws it.
        {"How is a progress bar's percent written?",
         "progress_bar_percent in src/ui/widgets.h",
         R"("%\.0f%%")",
         "",
         {},
         {},
         "derive-once review of M_D round 2, library finding 3 (phase 3 task FX2-P)",
         {R"(std::snprintf(overlay, sizeof(overlay), "%.0f%%", f * 100.0f);)",
          R"(std::snprintf(overlay, sizeof(overlay), "%.0f%%", lp.fraction * 100.0f);)"},
         {R"(ImGui::SliderInt("##volume", &volume, 0, 100, "%d%%"))",
          "progress_bar_percent(lp.fraction);"},
         {{"src/ui/widgets.h",
           R"(std::snprintf(overlay, sizeof(overlay), "%.0f%%", fraction * 100.0f);)",
           "progress_bar_percent, the owner"}}},
        // "Not the red lane" written as the cymbal rule instead of asking
        // allows_cymbals, which names the three lanes that carry one.
        {"Which drum lanes can carry a cymbal?",
         "allows_cymbals in src/core/model.cpp",
         R"([!=]=\s*(app::)?(PreviewLane|NoteColor)::Red\b|\b(PreviewLane|NoteColor)::Red\s*[!=]=)",
         "",
         {},
         {},
         "derive-once review of M_D round 2, library finding 8 (phase 3 task FX2-P)",
         {"g.cymbal = pro && n.cymbal && n.lane != PreviewLane::Red;",
          "if (c == NoteColor::Red) return false;"},
         {"g.cymbal = pro && n.cymbal && allows_cymbals(app::color_of(n.lane));",
          "case PreviewLane::Red:    return Pad::Red;"}},
        // ---- the report pages (phase 3 task K1a) ----
        // A bare toLocaleString() groups by the browser's own language, so a
        // German browser prints 1.234 under a subtitle that says 1,234. The
        // pages' shared fmt groups with one fixed rule.
        {"Which locale groups thousands on a report page?",
         "fmt in src/app/html_page.cpp",
         R"(\.toLocaleString\(\))",
         "",
         {},
         {},
         "audit findings 14 and 99; D48, Q12 (phase 3 task K1a)",
         {"idx.textContent = (++n).toLocaleString();",
          "['Paths shown', rows.length.toLocaleString()],"},
         {"const fmt = n => n === null || n === undefined ? DASH : n.toLocaleString('en-US');",
          "['Paths shown', fmt(rows.length)],"},
         {},
         // The pages live in src/app; a scope entry with a slash names one
         // file, so the row scans all of src, where nothing else has one.
         {"src"}},
        // toFixed rounds the float a page holds, which can tip an exact half
        // the wrong way (99.005% read 99.00%). The pages show numbers the C++
        // wrote as text, or work in whole numbers the C++ sent.
        {"How is a number rounded on a report page?",
         "the C++ text the payload carries: format_percent and percent_steps, "
         "format_avg_mult and format_ms in src/app/display_format.cpp",
         R"(\.toFixed\()",
         "",
         {},
         {},
         "audit finding 51; M_D review findings 2 and 8 (phase 3 task FX-R)",
         {"['num', r.mult.toFixed(3)],",
          "? (withPct.reduce((a, r) => a + r.pct, 0) / withPct.length).toFixed(2) + '%' : DASH;"},
         {"['num', r.mult_text],",
          "avgPct = Math.floor(h / 100) + '.' + String(h % 100).padStart(2, '0') + '%';"},
         {},
         {"src"}},
        // Task E2: the multiplier steps live in to_multiplier only. A combo
        // tested against 10, 20 or 30, or taken mod 10, is a second answer
        // (MultSqueeze::applies held one before D51 call 3).
        {"Where does the combo multiplier step up?",
         "to_multiplier in src/core/timing.cpp",
         R"(\bcombo\w*\)?\s*%\s*10\b|\bcombo\w*\s*[<>]=?\s*(10|20|30)\b|\b(10|20|30)\s*[<>]=?\s*combo)",
         "",
         {},
         {},
         "D51 calls 3 and 13 (audit findings 49 and 335), 2026-10-04",
         {"const int mod = (chord.count() + combo) % 10;", "return (combo_ % 10 == 7) ? \"high\" : \"low\";",
          "if (combo >= 20) return 3;", "if (30 <= combo_after) ok = true;"},
         {"if (to_multiplier(combo + 1) < to_multiplier(combo + chord.count())) ok = true;",
          "if (count % 10 == 0) return;", "if (combo > 100) return;"},
         {{"src/core/timing.cpp", "if (combo < 10) return 1;", "to_multiplier, the owner"},
          {"src/core/timing.cpp", "if (combo < 20) return 2;", "to_multiplier, the owner"},
          {"src/core/timing.cpp", "if (combo < 30) return 3;", "to_multiplier, the owner"}}},
        // Phase 7 task SE1: the Settings owner.
        {"What text turns an INI setting on or off?",
         "parse_bool in src/core/strutil.cpp",
         R"re([!=]=\s*"(0|1|true|false|yes|no)")re",
         "",
         {},
         {},
         "audit finding 68 (code-only: on/off text is 0 or 1 only), phase 7 task SE1",
         {"else if (key == \"is_rescan\") s.is_rescan = (value == \"1\");",
          "bool flag_bool(const std::string& v) { return v == \"1\" || v == \"true\" || v == \"yes\"; }",
          "if (v != \"0\") on = true;"},
         {"if (a.ms == \"off\") s.mslimit_enabled = false;",
          "if (const std::optional<bool> on = parse_bool(line.value)) s.**b = *on;"},
         {{"src/core/strutil.cpp", "if (text == \"1\") return true;", "parse_bool, the owner"},
          {"src/core/strutil.cpp", "if (text == \"0\") return false;", "parse_bool, the owner"}}},
        // A setting's range, or the cap floor of 1 bar, worked out again
        // outside the key table.
        {"What range may a number setting hold?",
         "Settings::clamp in src/app/config.cpp (the key table)",
         R"(\b(mslimit_value|backendlimit_value|preview_volume|depth_value|depth_mode|sp_cap|hit_window_ms|volume_pct_)\s*=\s*(std::(clamp|max|min)\(|[^;]*>\s*100\s*\?)|\b(sp_)?cap\s*<\s*1\b|std::max\(\s*1\s*,[^;]*cap\b)",
         R"(Settings::clamp\()",
         {"src/app/config.cpp"},
         {},
         "D51 Q14 (nearest edge of the box's range, volume 0 to 100) and Q16 (a 1-bar cap stays "
         "allowed); audit findings 138, 139, 322; phase 7 task SE1",
         {"app.settings.sp_cap = std::max(1, cap);",
          "app.settings.backendlimit_value = std::clamp(app.settings.backendlimit_value, 0,",
          "volume_pct_ = percent < 0 ? 0 : percent > 100 ? 100 : percent;",
          "curve.cap = sp_cap < 1 ? 1 : sp_cap;", "if (cap < 1)",
          "const int cap = std::max(1, pc->sp_meter_cap());"},
         {"app.settings.mslimit_value = Settings::clamp(&Settings::mslimit_value, v);",
          "workers_ = std::max(1, workers);", "d.width = std::max(1, width);",
          "const int window = static_cast<int>(kSqueezeWindowMs);"}},
        {"What gain does a volume percent play at?",
         "Settings::volume_gain in src/app/config.cpp",
         R"((volume|percent|pct)\w*\)*\s*/\s*100\b)",
         R"(volume_gain\()",
         {},
         {},
         "audit finding 72, phase 7 task SE1",
         {"transport_.set_gain(static_cast<float>(volume_pct_) / 100.0f);",
          "const float gain = percent / 100.0f;"},
         {"transport_.set_gain(Settings::volume_gain(volume_pct_));",
          "const double bars = phrase_count / 4.0;"},
         {{"src/app/config.cpp",
           "return static_cast<float>(clamp(&Settings::preview_volume, percent)) / 100.0f;",
           "volume_gain, the owner"}}},
        // The bestpath column and hydra_replay's result block both show this
        // text, so they read it from one function.
        {"What text is a record's best path?",
         "best_path_text in src/store/record_store.cpp",
         R"(best_path\(\)\.pathstring\(\))",
         "",
         {},
         {},
         "ST1 (audit findings 116 and 132); phase 7 M7-1 derive-once review finding 2 (2026-10-04)",
         {"return record.paths.empty() ? std::string() : record.best_path().pathstring();",
          "{\"bestpath\", best ? rec.best_path().pathstring() : std::string()}};"},
         {"CHECK(p->pathstring() == \"0 E3+ E5 E1\");"},
         {{"src/store/record_store.cpp",
           "return record.paths.empty() ? std::string() : record.best_path().pathstring();",
           "best_path_text, the owner"}}},
        // rules_fp_of builds the SQL from the column or parameter it is
        // given, so its own line never spells structure's bytes 5 to 12.
        {"Which rules was a stored row made under (SQL)?",
         "rules_fp_of in src/store/record_store.cpp",
         R"(substr\(\s*structure\s*,\s*5\s*,\s*8\s*\))",
         "",
         {},
         {},
         "ST1 (audit findings 116 and 132); phase 7 M7-1 derive-once review finding 4 (2026-10-04)",
         {"\" WHERE substr(structure,5,8) = ?\");",
          "\"DELETE FROM results WHERE substr(structure,5,8) = ?\"}) {"},
         {"\") AND substr(structure,1,4) IN (\" + placeholders(kPathFormatStamp.accepted.size()) +"},
         {},
         {"src"}},
        // hydra_replay score writes each chord's gems once, from ReplayNote;
        // play_chart reads them there.
        {"Which pads (color, cymbal) does a replayed chord hit?",
         "the \"notes\" list cmd_score writes in tools/replay.cpp, from ReplayNote",
         R"(\{\s*"cymbal"\s*,)",
         "",
         {},
         {},
         "phase 7 M7-1 derive-once review round 2, finding 1 (2026-10-04)",
         {"{\"cymbal\", n.cymbal},",
          "lanes.push_back(nlohmann::json{{\"color\", color_str(n.colortype)}, {\"cymbal\", "
          "n.is_cymbal()}});"},
         {"{\"is_fill\", c.is_fill},", "{\"color\", color_str(n.color)},"},
         {{"tools/replay.cpp", "{\"cymbal\", n.cymbal},", "the chord's \"notes\" list, the owner"}}},
        // reduce_group reads this one answer both where a tied over-limit path
        // is folded and where an over-limit leader is dropped.
        {"What is the best score allowed to eliminate a path in its group?",
         "Engine::best_eligible_score in src/search/engine.cpp",
         R"(if \(!can_outscore\(|beating_\.back\(\))",
         "",
         {},
         {},
         "phase 7 M7-1 derive-once review round 2, finding 2 (2026-10-04)",
         {"if (!can_outscore(idx)) continue;",
          "const int64_t best = beating_.empty() ? 0 : beating_.back();"},
         {"if (can_outscore(idx)) beating_.push_back(cur_[(size_t)idx].score);",
          "return !filtered_[(size_t)idx] ||"},
         {{"src/search/engine.cpp", "if (!can_outscore(idx)) continue;",
           "best_eligible_score, the owner"}},
         {"src/search/engine.cpp"}},
        // The CJK fallback is merged into the main font, and ImGui scales
        // merged glyphs by the ratio of the two sizes, so every load reads
        // the one size.
        {"What size do the UI fonts load at?",
         "kFontSize in src/ui/app_shell.cpp",
         R"((^|[^0-9.])18\.0f)",
         "",
         {},
         {},
         "audit finding 208 (phase 6 task J1-2)",
         {"(resource_dir + \"ShipporiAntiqueB1-Regular.ttf\").c_str(), 18.0f);",
          "if (io.Fonts->AddFontFromFileTTF(path, 18.0f, &merge)) break;"},
         {"(resource_dir + \"CourierPrime-Regular.ttf\").c_str(), kFontSize);",
          "const float w = 118.0f;"},
         {{"src/ui/app_shell.cpp", "constexpr float kFontSize = 18.0f;", "kFontSize, the owner"}},
         {"src/ui/app_shell.cpp"}},
        {"Where does the startup UI scale come from?",
         "ui_scale_for_dpi in src/ui/app_shell.cpp",
         R"(ImGui_ImplWin32_GetDpiScaleFor(Monitor|Hwnd)\b)",
         "",
         {},
         {},
         "D54 (audit finding 210)",
         {"float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(",
          "const float window_scale = ImGui_ImplWin32_GetDpiScaleForHwnd(hwnd);"},
         {"const float window_scale = hydra::ui::ui_scale_for_dpi(",
          "ImGui_ImplWin32_EnableDpiAwareness();"},
         {},
         {}},
        {"Is a library split share valid?",
         "share_is_valid in src/ui/app_shell.cpp",
         R"(>\s*0\.0f\s*&&.*<\s*1\.0f)",
         "",
         {},
         {},
         "audit finding 211 (phase 6 task J1-2)",
         {"if (end == value.c_str() || *end != '\\0' || !(v > 0.0f && v < 1.0f)) return;",
          "if (!(share > 0.0f && share < 1.0f) || share == g_layout.library_share) return;"},
         {"if (!share_is_valid(share) || share == g_layout.library_share) return;"},
         {{"src/ui/app_shell.cpp",
           "bool share_is_valid(float share) { return share > 0.0f && share < 1.0f; }",
           "share_is_valid, the owner"}},
         {"src/ui/app_shell.cpp"}},
        {"What colour clears the frame?",
         "kClearColor in src/ui/app_shell.h",
         R"(0\.10?f\s*,\s*0\.11f\s*,\s*0\.13f)",
         "",
         {},
         {},
         "audit finding 270 (phase 6 task J1-2)",
         {"const ImVec4 clear_color = ImVec4(0.10f, 0.11f, 0.13f, 1.00f);",
          "const float clear[4] = {0.10f, 0.11f, 0.13f, 1.0f};"},
         {"context->ClearRenderTargetView(rtv.Get(), hydra::ui::kClearColor);"},
         {{"src/ui/app_shell.h",
           "inline constexpr float kClearColor[4] = {0.10f, 0.11f, 0.13f, 1.00f};",
           "kClearColor, the owner"}},
         {"src", "tests"}},
        // D3D11CreateDeviceAndSwapChain (Hydra.exe's window) is another
        // question; the parenthesis keeps it out.
        {"How do the tests make a D3D11 device?",
         "warp::make_device in tests/warp_util.h",
         R"(\bD3D11CreateDevice\()",
         "",
         {},
         {},
         "audit finding 293 (phase 6 task J1-2)",
         {"if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, &want, 1,"},
         {"HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,",
          "if (!warp::make_device(device, context)) {"},
         {{"tests/warp_util.h",
           "HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, &want, 1,",
           "warp::make_device, the owner"}},
         {"tests"}},
        {"Which GUI tests does a name select?",
         "uitest::selects in tests/ui/uitest_harness.cpp",
         R"(==\s*"all")",
         "",
         {},
         {},
         "audit finding 294 (phase 6 task J1-2)",
         {"if (what == \"all\" || what == t->Name) {", "bool found = w == \"all\";"},
         {"if (uitest::selects(w, t->Name)) {", "if (a == \"--all\") wanted.push_back(\"all\");"},
         {{"tests/ui/uitest_harness.cpp", "return what == \"all\" || what == test_name;",
           "uitest::selects, the owner"}},
         {"tests"}},
        // J1-3 (phase 6): the track state's owners. toggle_at was a second
        // copy of the span rule beside SpanSweep and is deleted (D54).
        {"What state does a span have at an instant?",
         "SpanSweep in src/render/track_state.h and src/render/track_state.cpp",
         R"(\btoggle_at\s*\()",
         "",
         {},
         {},
         "audit R7.13; D54 (toggle_at deleted, not kept as a test oracle)",
         {"inst.overdrive = toggle_at(overdrive_, t);",
          "CHECK(toggle_at(ivs, 1.0) == Toggle::Start);"},
         {"CHECK(sweep.at(1.0) == Toggle::Start);", "inst.overdrive = overdrive_.at(t);"},
         {},
         {"src", "tools", "tests"}},
        // "On before this instant" (End, Restart, On: the state entering a
        // window) is a different question and is not flagged.
        {"Is a span on after this instant?",
         "toggle_on_after in src/render/track_state.h",
         R"(Toggle::Start\s*\|\|.*Toggle::Restart|!=\s*Toggle::Empty\s*&&.*!=\s*Toggle::End)",
         "",
         {},
         {},
         "audit finding 223",
         {"on = tg == Toggle::Start || tg == Toggle::Restart || tg == Toggle::On;",
          "bool toggle_on(Toggle t) { return t != Toggle::Empty && t != Toggle::End; }"},
         {"bool on = first == Toggle::End || first == Toggle::Restart || first == Toggle::On;",
          "on = toggle_on_after(inst.*field);"},
         {{"src/render/track_state.h",
           "return t == Toggle::Start || t == Toggle::Restart || t == Toggle::On;",
           "toggle_on_after, the owner"}}},
        {"Which overlay spans does the timeline merge?",
         "TrackState::merge_overlay_edges in src/render/track_state.cpp",
         R"(&\s*(st\.)?fill_\s*,\s*&\s*(st\.)?fill_taken_\s*,\s*&\s*(st\.)?sp_active_\s*,\s*&\s*(st\.)?fill_lane_ivs_\b)",
         "",
         {},
         {},
         "audit R7.15",
         {"{&st.fill_, &st.fill_taken_, &st.sp_active_, &st.fill_lane_ivs_})",
          "sorted_edges({&st.fill_, &st.fill_taken_, &st.sp_active_, &st.fill_lane_ivs_});"},
         {"for (const std::vector<TrackState::Interval>* ivs : {&st.overdrive_, &st.solo_})"},
         {{"src/render/track_state.cpp",
           "sorted_edges({&fill_, &fill_taken_, &sp_active_, &fill_lane_ivs_});",
           "merge_overlay_edges, the one merge build_track_state and rebuild_overlay_fields "
           "use"}}},
        // The tests read the per-instant pad under other names (i.fill_lane_pad,
        // taken->fill_lane_pad), so the row scans src only.
        {"Which pad colours a fill lane?",
         "LaneSweep and TrackState::make_lane_bounds in src/render/track_state.cpp",
         R"(\binst\.fill_lane_pad\b)",
         "",
         {},
         {},
         "audit R7.14 and finding 76; D53 item 3 (two touching taken fills each light their own "
         "lane)",
         {"if (inst.fill_lane_pad) { pad = inst.fill_lane_pad; break; }",
          "inst.fill_lane_pad = li.pad;"},
         {"const std::optional<Pad> pad = opened ? opened->fill_lane_pad : entering;",
          "for (const LaneSpan& s : state.make_lane_bounds(win, near_t, far_t)) {"},
         {{"src/render/track_state.cpp", "inst.fill_lane_pad.reset();",
           "TrackState::FieldSweep, which asks LaneSweep"},
          {"src/render/track_state.cpp",
           "if (std::optional<size_t> w = lane_pad_.at(t)) inst.fill_lane_pad = lanes_[*w].pad;",
           "TrackState::FieldSweep, which asks LaneSweep"}},
         {"src"}},
        // The first notes.mid, else the last notes.chart. A loop over a
        // listing that takes a notes file's format from an entry's name, or
        // the scan's "mid if found, else chart", writes that preference
        // again. Naming the one .srb notes stream (md.notes_filename) is a
        // different question: there is only one stream to name.
        {"Which notes file wins when a song has both?",
         "pick_notes_file in src/parse/chart_files.cpp",
         R"(\bfound_mid\s*\?|ChartFormat\s+\w+\s*=\s*notes_file_format\(\s*\w+\.name\s*\))",
         "",
         {"src/parse/chart_files.cpp"},
         {},
         "audit finding 186; phase 6 task J1-5 (D53)",
         {"const DirEntry* notes = found_mid ? found_mid : found_chart;",
          "const ChartFormat f = notes_file_format(e.name);"},
         {"const ChartFormat named = notes_file_format(md.notes_filename);",
          "const DirEntry* found_mid = nullptr;",
          "const std::optional<NotesFilePick> pick = pick_notes_file(names);"}},
        // An inflate at kSrbHeaderSize is a read of the metadata stream. The
        // streams after it (the notes, the Preview's audio) start at an offset
        // a reader handed back, a different question.
        {"Where does an .srb's metadata stream start?",
         "srb_read_metadata in src/parse/srb.cpp",
         R"(\bkSrbHeaderSize\s*,)",
         "",
         {"src/parse/srb.cpp"},
         {},
         "audit finding 188; phase 6 task J1-5 (D53)",
         {"buf.data(), buf.size(), kSrbHeaderSize, kSrbMaxMetadata, nullptr);",
          "srb_inflate_stream_reading(src, kSrbHeaderSize, kSrbMaxMetadata, &notes_offset);",
          "srb_inflate_stream(buf.data(), buf.size(), kSrbHeaderSize,"},
         {"std::vector<uint8_t> stream = srb_inflate_stream(",
          "buf.data(), buf.size(), offset, kSrbMaxStream, &next);",
          R"(if (src.size <= kSrbHeaderSize) throw std::runtime_error("Truncated SRB file.");)",
          "if (buf.size() <= kSrbHeaderSize) return stems;"}},
        {"Which file is the folder's song.ini?",
         "find_song_ini in src/parse/chart_files.cpp",
         R"(\bis_song_ini\s*\()",
         "",
         {"src/parse/chart_files.cpp", "src/parse/chart_files.h"},
         {},
         "audit finding 189; first match wins (phase 6 task J1-5, decided at launch)",
         {"else if (is_song_ini(e.name)) found_ini = &e;",
          R"(if (!e.is_dir && is_song_ini(e.name)) return folder + "\\" + e.name;)"},
         {"const std::string ini = find_song_ini(folder);",
          "const DirEntry* ini = find_song_ini(listing);"}},
        // Keyed on the cut beside its "no separator" test, not on
        // find_last_of: a file's base name (substr(slash + 1)) and a name's
        // stem (substr(0, dot)) find the same separator for other questions.
        {"What is a path's parent folder?",
         "parent_folder in src/core/winstr.cpp",
         R"(\b(pos|slash)\s*[!=]=\s*(std::string::)?npos\b.*\.substr\(\s*0\s*,\s*(pos|slash)\s*\))",
         "",
         {},
         {},
         "audit finding 251; the scan's rule, kept for the stored rootfolder (phase 6 task J1-5, "
         "decided at launch)",
         {"return pos == std::string::npos ? std::string() : p.substr(0, pos);",
          R"(return pos == std::string::npos ? std::string(".") : path.substr(0, pos);)",
          R"(return slash == std::string::npos ? std::string(".") : path.substr(0, slash);)"},
         {"return slash == std::string::npos ? path : path.substr(slash + 1);",
          "return dot == std::string::npos ? b : b.substr(0, dot);",
          R"(size_t slash = path.find_last_of("/\\");)"},
         {{"src/core/winstr.cpp",
           "return pos == std::string::npos ? std::string() : p.substr(0, pos);",
           "parent_folder, the owner"}}},
        // The "above 32" test on ShellExecute's result. Which code makes the
        // call is the "Which code launches the Windows shell?" row.
        {"Did a ShellExecute launch succeed?",
         "shell_execute_ok in src/core/winstr.cpp",
         R"(reinterpret_cast<\s*(INT_PTR|intptr_t|LONG_PTR)\s*>\s*\(\s*\w+\s*\)\s*>\s*32\b|\(\s*(INT_PTR|intptr_t|LONG_PTR)\s*\)\s*\w+\s*>\s*32\b)",
         "",
         {},
         {},
         "audit finding 217; ShellExecute's documented rule (32 and below means failure)",
         {"return reinterpret_cast<INT_PTR>(rc) > 32;",
          "return reinterpret_cast<INT_PTR>(r) > 32;  // ShellExecute's documented success test",
          "if ((INT_PTR)h > 32) ok = true;"},
         {"return shell_execute_ok(rc);", "ov.OffsetHigh = static_cast<DWORD>(at >> 32);"},
         {{"src/core/winstr.cpp", "return reinterpret_cast<INT_PTR>(shell_execute_result) > 32;",
           "shell_execute_ok, the owner"}}},
        // A .sng's magic, or a make_sng definition (the return type right
        // before the name; a call assigns to a variable in between).
        {"Which test helper builds a .sng?",
         "make_sng in tests/sng_util.h",
         R"("SNGPKG"|\bstd::vector<uint8_t>\s+make_sng\s*\()",
         "",
         {"tests/sng_util.h"},
         {},
         "audit finding 117; phase 6 task J1-5 (D53)",
         {R"(std::string header = "SNGPKG";)", "std::vector<uint8_t> make_sng(",
          "inline std::vector<uint8_t> make_sng("},
         {R"(std::vector<uint8_t> buf = make_sng({{"name", "Song"}}, {});)",
          R"(const std::vector<uint8_t> whole = make_sng({{"name", "Song"}}, {});)"},
         {},
         {"tests"}},
        // A brace list of a scan row's fields, md5 first, pushed into the
        // library's entries or built as a ChartLibraryEntry. The ScanJob's
        // list of ScanItems read back from the library (items_) is the other
        // direction, a different question.
        {"How does a scan row become a library entry?",
         "to_library_entry in src/app/analysis.cpp",
         R"(\b(ChartLibraryEntry(\s+\w+)?\s*\{|entries\.push_back\(\s*\{)\s*\w+\.md5\b)",
         "",
         {"src/app/analysis.cpp"},
         {},
         "audit finding 256; phase 6 task J1-6 (D53)",
         {"entries.push_back({it.md5, it.title, it.artist, it.charter, it.notespath,",
          "store::ChartLibraryEntry e{item.md5, item.title, item.artist, item.charter,"},
         {"for (const ScanItem& it : items) entries.push_back(to_library_entry(it));",
          "items_.push_back({e.md5, e.title, e.artist, e.charter, e.notespath, e.rootfolder});"},
         {},
         {"src", "tools", "tests"}},
        // A floor of 1 on a worker count. batch_worker_count keeps the floor
        // (D54 records its rule as it is); run_work_pool refuses a count
        // below 1 rather than raising it.
        {"How many workers does a batch get?",
         "batch_worker_count in src/app/analysis.cpp",
         R"(std::max\(\s*1\s*,\s*(std::min\(\s*)?(worker\w*|guess)\b)",
         "",
         {},
         {},
         "audit R7.22; D54 (the worker rule recorded as it is)",
         {"workers_ = std::max(1, workers);",
          "std::min(count, static_cast<size_t>(std::max(1, worker_count)));"},
         {"const int w = std::max(1, width);",
          "const int cap = std::max(1, pc->sp_meter_cap());",
          "const size_t nworkers = std::min(count, static_cast<size_t>(worker_count));"},
         {{"src/app/analysis.cpp", "return std::max(1, std::min(guess, kBatchMaxWorkers));",
           "batch_worker_count, the owner"}}},
        // Lowering a hash, a leaderboard identifier or a stored md5 so two
        // spellings match. Lowering an ini key or a search query is a
        // different question.
        {"How is a chart hash spelled for matching?",
         "normalize_chart_hash in src/app/analysis.cpp",
         R"(\bto_lower_ascii\(.*\b(md5|hash|hyhash|identifier)\b)",
         "",
         {},
         {},
         "audit finding 192; phase 6 task J1-6 (D53)",
         {"by_hash.emplace(to_lower_ascii(r.hyhash), std::move(r));",
          "in_library.insert(to_lower_ascii(e.md5));",
          R"(s.identifier = to_lower_ascii(jstr(entry, "identifier"));)"},
         {"const std::string key = to_lower_ascii(raw_key);",
          "std::string key = to_lower_ascii(line.substr(0, eq));",
          "in_library.insert(normalize_chart_hash(e.md5));"},
         {{"src/app/analysis.cpp",
           "std::string normalize_chart_hash(std::string_view hash) { return to_lower_ascii(hash); }",
           "normalize_chart_hash, the owner"}}},
        // A folder and a file name glued with a bare backslash. join_folder
        // adds none when the folder already ends in a slash, which a bare join
        // gets wrong. The owner's own line is listed, so a second copy of it
        // fails too.
        {"How are a folder and a file name joined?",
         "join_folder in src/core/winstr.cpp",
         R"(\bfolder\s*\+\s*"\\\\"\s*\+)",
         "",
         {},
         {},
         "review of M6-J1a, finding 1 (phase 6 wave J1 sweep)",
         {R"(return ini ? folder + "\\" + ini->name : std::string();)",
          R"(if (!e.is_dir && is_song_ini(e.name)) return folder + "\\" + e.name;)"},
         {R"(return (last == '\\' || last == '/') ? a + b : a + "\\" + b;)"},
         {{"src/core/winstr.cpp",
           R"(return (last == '\\' || last == '/') ? folder + name : folder + "\\" + name;)",
           "join_folder, the owner"}},
         {"src"}},
        // The same pattern as "Is a value inside the squeeze window?", over
        // tests/. That row's scope stays src and tools; rows are append-only.
        // Arithmetic and == on the constant (test_squeeze_rating.cpp) and
        // storing it (test_docs_match_code.cpp) answer other questions.
        {"Is a value inside the squeeze window? (tests)",
         "within_squeeze_window in src/core/model.h",
         R"([<>]=?\s*(\w+::)*kSqueezeWindowMs|(\w+::)*kSqueezeWindowMs\s*[<>])",
         "",
         {},
         {},
         "audit finding 283; phase 6 task J1-1 (D53, D54)",
         {"c.ms - phrase_note->ms < kSqueezeWindowMs)",
          "if (!long_after && c.ms - last_phrase->ms > kSqueezeWindowMs)",
          "if (hydra::kSqueezeWindowMs > gap) continue;"},
         {"CHECK(kSqueezeWindowMs == 500.0);",
          "const double offsets[] = {0.0, 180.0, -300.0, kSqueezeWindowMs - 1.0,",
          "out[\"kSqueezeWindowMs\"] = {false, hydra::kSqueezeWindowMs};",
          "-static_cast<int>(hydra::kSqueezeWindowMs));",
          "if (!long_after && !within_squeeze_window(c.ms - last_phrase->ms))"},
         {},
         {"tests"}},
        // A hand-built phrase end sets the flag and the phrase start together,
        // as the parser's close_sp_phrase does, through one fixture helper.
        // Reads and comparisons of the flag are not assignments.
        {"What fields does a phrase-end note carry in a hand-built Song?",
         "mark_phrase_end in tests/record_fixtures.h",
         R"(\bflag_sp\s*=(?!=))",
         "",
         {},
         {},
         "audit finding 300; phase 6 task J1-1 (D53, D54)",
         {"ts.flag_sp = tick == 3256;", "ts.flag_sp = phrase;", "ts.flag_sp = true;"},
         {"if (!ts.flag_sp) continue;",
          "if (x.flag_solo != y.flag_solo || x.flag_sp != y.flag_sp) return false;",
          "CHECK(song.sequence[0].flag_sp == true);",
          "if (phrase) test::mark_phrase_end(ts, tick, song.tick_resolution());"},
         {{"tests/record_fixtures.h", "ts.flag_sp = true;", "mark_phrase_end, the owner"}},
         {"tests"}},
        // Picking a tick's meter section by ordering it against the sections'
        // first ticks. section_at answers that with keys_ directly, so no
        // keys_at ordering is the owner's. An equality test against a section
        // start (plusmeasure's barline check), reading a section's bounds, and
        // the measure-space entry in tick_at_measures_f answer other
        // questions.
        {"Which meter section is a tick measured in?",
         "MeasureIndex::section_at in src/core/timing.cpp",
         R"(keys_at\([^()]*\)\)*\s*[<>]|(^|[^-])[<>]=?\s*(\w+(\.|->))?keys_at\()",
         "",
         {},
         {},
         "audit finding 58; D51 call 22; phase 7 task TM (D58)",
         {"while (i > 0 && static_cast<double>(idx.keys_at(i)) > ticks) --i;",
          "if (mi.keys_at(i) <= tick) s = i;",
          "while (s + 1 < n && tick >= mi->keys_at(s + 1)) ++s;"},
         {"if (j < len && handled_ticks == idx.keys_at(j)) {",
          "const int64_t section_end = last_section ? last_tick : mi.keys_at(i + 1);",
          "static_cast<double>(idx.keys_at(s) - idx.starts_at(s)) /",
          "const int64_t first = mi->keys_at(i);",
          "const int i = idx.section_at(static_cast<int64_t>(std::ceil(ticks)));"}},
        // Fingerprinting a chart's files by hand: anything that calls sig_of
        // has chosen which files go in. Only pending_chart_of chooses, and
        // the scan walk and chart_files_sig both ask it.
        {"Which files make up a chart's fingerprint?",
         "pending_chart_of in src/app/analysis.cpp",
         R"(\bsig_of\()",
         "",
         {},
         {},
         "derive-once review of M7-2c, finding 1 (phase 7 task PV join)",
         {"pc.sig = sig_of(*notes, found_ini);",
          "return sig_unchanged(sig, sig_of(*notes, ini));"},
         {"return sig_of_chart(entries, notes);",
          "const std::optional<PendingChart> now = pending_chart_of(dir, e, entries);"},
         {{"src/app/analysis.cpp", "std::string sig_of(const DirEntry& notes, const DirEntry* ini) {",
           "sig_of, the string format pending_chart_of uses"},
          {"src/app/analysis.cpp", "pc.sig = sig_of(chart, ini);", "pending_chart_of, the owner"}}},
        // Moving between chart time and audio time with the offset written
        // out, either way, instead of calling the sync rule's two owners.
        {"How does chart time become audio time, and back?",
         "audio_ms_of_chart_ms and chart_ms_of_audio_ms in src/audio/frames.h",
         R"(\w*chart_ms\s*[+-]\s*audio_offset_ms|\bms\s*[+-]\s*audio_offset_ms_|-\s*audio_offset_ms\b)",
         "",
         {},
         {},
         "derive-once review of M7-2c, finding 2 (phase 7 task PV join); moved into hydra_audio "
         "by D69 (phase 7 task AL1)",
         {"playhead_->seek_ms(ms + audio_offset_ms_);", "return chart_ms + audio_offset_ms;",
          "return audio::ms_of_frames(audio.length_frames(), audio.sample_rate()) - audio_offset_ms;"},
         {"playhead_->seek_ms(audio_ms_of_chart_ms(ms, audio_offset_ms_));",
          "front_pad = frames_of_ms(-offset_ms, kOutRate);"},
         {{"src/audio/frames.h", "return chart_ms + audio_offset_ms;",
           "audio_ms_of_chart_ms, the owner"},
          {"src/audio/frames.h", "return audio_ms - audio_offset_ms;",
           "chart_ms_of_audio_ms, the owner"}}},
        // ---- audio owners (phase 6 task J1-4) ----
        // The frame pair's row is phase 3's ("How many ms do audio frames
        // last...", above).
        {"What is the lowest legal output gain?",
         "Playhead::set_gain in src/audio/player.h",
         R"(gain < 0\.0f \? 0\.0f)",
         "",
         {},
         {},
         "audit finding 225, folded under D53 (phase 6 task J1-4)",
         {"gain_ = gain < 0.0f ? 0.0f : gain;"},
         {"float gain() const { return gain_; }"},
         {{"src/audio/player.h",
           "void set_gain(float gain) { gain_ = gain < 0.0f ? 0.0f : gain; }",
           "Playhead::set_gain, the owner"}}},
        {"How is a stem converted to the output format?",
         "stem_converter_config in src/audio/mixer.cpp",
         R"(ma_data_converter_config_init\()",
         "",
         {},
         {},
         "audit finding R7.28; D54 records miniaudio's converter defaults as they are",
         {"ma_data_converter_config cfg = ma_data_converter_config_init(",
          "s->init_converter(ma_data_converter_config_init("},
         {"s->init_converter(sc.config);"},
         {{"src/audio/mixer.cpp", "c.config = ma_data_converter_config_init(",
           "stem_converter_config, the owner"}}},
        {"Which test helper reads an audio fixture?",
         "fixture_path and read_fixture in tests/audio_util.h",
         // No closing quote after /audio/, so a build that names the file
         // inside the literal is caught too (review of M6-J1c finding 1).
         R"(HYDRA_TESTDATA_DIR\) \+ "/audio/)",
         "",
         {},
         {},
         "audit finding 277, folded under D53 (phase 6 task J1-4)",
         {"return hydra::read_file_bytes(std::string(HYDRA_TESTDATA_DIR) + \"/audio/\" + name);",
          R"(copy_file_utf8(std::string(HYDRA_TESTDATA_DIR) + "/audio/sine220.ogg", d + "\\song.ogg");)"},
         {"ogg.path = fixture_path(\"sine220.ogg\");", "#ifndef HYDRA_TESTDATA_DIR"},
         {{"tests/audio_util.h",
           "return std::string(HYDRA_TESTDATA_DIR) + \"/audio/\" + name;",
           "fixture_path, the owner (read_fixture reads through it)"}},
         {"tests"}},
        {"Which test helper estimates a tone's frequency?",
         "estimate_freq_hz in tests/audio_util.h",
         R"(\b(double|float|auto)\s+estimate_freq_hz\s*\()",
         "",
         {},
         {},
         "audit finding 277, folded under D53 (phase 6 task J1-4)",
         {"double estimate_freq_hz(const DecodedAudio& a) {",
          "double estimate_freq_hz(const DecodedAudio& a, int channel) {"},
         {"CHECK(estimate_freq_hz(out, 0) == doctest::Approx(220.0).epsilon(0.07));"},
         {{"tests/audio_util.h",
           "inline double estimate_freq_hz(const hydra::audio::DecodedAudio& a, int channel) {",
           "estimate_freq_hz, the owner"}},
         {"tests"}},
        // The join row above, for any name: a string literal that starts with
        // a backslash and a name, glued on with +, or a bare backslash added
        // with +=. join_folder's own "\\" is followed by a quote, not a name.
        // Rows are append-only, so the first row stays as it is.
        {"How are a folder and a file name joined? (any name, not only folder)",
         "join_folder in src/core/winstr.cpp",
         R"(\+\s*"\\\\\w|\+=\s*"\\\\")",
         "",
         {},
         {},
         "audit finding 209; phase 6 task J2-1 (D53)",
         {R"(return exe_dir() + "\\hydra.db";)",
          R"(if (resource_dir.back() != '\\' && resource_dir.back() != '/') resource_dir += "\\";)"},
         {R"(return (last == '\\' || last == '/') ? folder + name : folder + "\\" + name;)",
          R"(constexpr std::wstring_view kLongPrefix = L"\\\\?\\";)",
          R"(case '\\': out += "\\\\"; break;)",
          R"(return join_folder(exe_dir(), "hydra.db");)"},
         {},
         {"src", "tools"}},
        // A string literal naming the resource folder under the exe. The
        // owner joins the bare name "resource", so no line matches it.
        {"Where is the resource folder?",
         "resource_dir in src/app/config.cpp",
         R"("[^"]*\\\\resource)",
         "",
         {},
         {},
         "audit finding 209; phase 6 task J2-1 (D53)",
         {R"(options.resource_dir.empty() ? app::exe_dir() + "\\resource" : options.resource_dir;)",
          R"(const std::string dir = hydra::app::exe_dir() + "\\resource\\";)"},
         {R"(std::string resource_dir() { return join_folder(exe_dir(), "resource"); })",
          R"(#include "ui/resource.h")"},
         {},
         {"src"}},
        // A byte lowered through the C locale, or a private definition of one
        // of strutil's ASCII helpers (a definition line has no ; after its
        // open paren). C2's wide starts_with in winstr.cpp stays where it is
        // (J2-1, decided at launch).
        {"How is text compared ignoring ASCII case, and which bytes are whitespace?",
         "lower_ascii, equals_ci, starts_with(_ci), ends_with(_ci) and is_ascii_space in "
         "src/core/strutil.cpp",
         R"(std::tolower\(|\b(bool|char)\s+(starts_with|starts_with_ci|ends_with|ends_with_ci|equals_ci|iequals_ascii|is_ascii_space|ascii_lower|lower_ascii)\s*\([^;]*$)",
         "",
         {},
         {},
         "audit finding 201; phase 6 task J2-1 (D53)",
         {"bool starts_with(std::string_view s, std::string_view prefix) {",
          "bool ends_with(std::string_view s, std::string_view suffix) {",
          "return std::tolower(static_cast<unsigned char>(a)) =="},
         {"bool starts_with(std::string_view s, std::string_view prefix);",
          "bool starts_with_any(std::string_view s, std::initializer_list<std::string_view> prefixes) {",
          R"(if (starts_with(what, "cannot write ")) return kReportWrite;)",
          "c = lower_ascii(c);"},
         {{"src/core/strutil.cpp", "char lower_ascii(char c) {", "lower_ascii, the owner"},
          {"src/core/strutil.cpp", "bool is_ascii_space(char c) {", "is_ascii_space, the owner"},
          {"src/core/strutil.cpp", "bool equals_ci(std::string_view a, std::string_view b) {",
           "equals_ci, the owner"},
          {"src/core/strutil.cpp", "bool starts_with(std::string_view s, std::string_view prefix) {",
           "starts_with, the owner"},
          {"src/core/strutil.cpp",
           "bool starts_with_ci(std::string_view s, std::string_view prefix) {",
           "starts_with_ci, the owner"},
          {"src/core/strutil.cpp", "bool ends_with(std::string_view s, std::string_view suffix) {",
           "ends_with, the owner"},
          {"src/core/strutil.cpp", "bool ends_with_ci(std::string_view s, std::string_view suffix) {",
           "ends_with_ci, the owner"},
          {"src/core/winstr.cpp", "bool starts_with(const std::wstring& s, std::wstring_view prefix) {",
           "the wide prefix test winstr keeps for its long-path prefixes (J2-1, decided at launch)"}}},
        // The root and one more byte cut off a path. relpath in analysis.cpp
        // cuts only after a separator and keeps backslashes: Python's
        // os.path.relpath for the library's stored path, another question.
        {"How is a scanned path keyed in the scan snapshot?",
         "relative_slash_path in src/core/strutil.cpp",
         R"(\.substr\(\s*\w+\.size\(\)\s*\+\s*1\s*\))",
         "",
         {},
         {{"src/app/analysis.cpp",
           "relpath: the library's stored relative path (a separator is required and backslashes "
           "stay), not the snapshot key"}},
         "audit finding 274; phase 6 task J2-1 (D53, D54)",
         {"p = p.substr(rel.size() + 1);", "rel = rel.substr(root.size() + 1);"},
         {"return slash == std::string::npos ? path : path.substr(slash + 1);",
          "const std::string tail = s.substr(s.size() - 4);"},
         {{"src/core/strutil.cpp", "path = path.substr(root.size() + 1);",
           "relative_slash_path, the owner"}},
         {"src", "tools", "tests"}},
        // A depth unit word chosen from the search's enum, or bench's header
        // words. The GUI's own wordings ("Within 4 scores", the score-range
        // box) are separate display text and do not name the enum.
        {"How do the analysis settings read as text?",
         "describe_settings in src/app/config.cpp",
         R"re(DepthMode::\w+.*"(scores|points)"|score range %d|%dms limit)re",
         "",
         {},
         {},
         "audit finding 206; phase 6 task J2-1 (D53, D54)",
         {R"(case hydra::DepthMode::Scores: depth_name = "scores"; break;)",
          R"(case hydra::DepthMode::Points: depth_name = "points"; break;)",
          R"(std::printf("Settings: the GUI default (SP cap %d, score range %d, %dms limit).\n\n",)"},
         {R"(const char* modes[] = {"scores", "points"};)",
          R"(hydra::counted(out.stats.total, "score", "scores") + ": " +)",
          R"(if (a.depth_mode) s.depth_mode = *a.depth_mode == "points" ? 1 : 0;)"},
         {{"src/app/config.cpp",
           R"(const char* unit = settings.depth_mode == DepthMode::Points ? "points" : "scores";)",
           "describe_settings, the owner"}}},
        // The two strings typed anywhere but CMakeLists.txt, which the walk
        // does not read (so no owner line is listed). The installer script is
        // checked by the case below this table's scan.
        {"What name and taskbar identity does the app present?",
         "HYDRA_APP_NAME and HYDRA_APP_USER_MODEL_ID in CMakeLists.txt",
         R"(Hydra\.Hydra|Hydra Deluxe)",
         "",
         {},
         {},
         "audit finding 257; phase 6 task J2-1 (D53)",
         {R"(inline constexpr const wchar_t* kWindowTitleW = L"Hydra Deluxe";)",
          R"(inline constexpr const wchar_t* kAppUserModelIDW = L"Hydra.Hydra";)",
          "AppName=Hydra Deluxe"},
         {"inline constexpr const wchar_t* kWindowTitleW = HYDRA_WIDEN(HYDRA_APP_NAME);",
          "AppName={#HYDRA_APP_NAME}", R"(DefaultDirName={autopf}\Hydra)",
          "OutputBaseFilename=HydraDeluxe-{#HYDRA_VERSION}-setup"},
         {},
         {"src", "installer/hydra.iss"}},
        // A drum-track pitch typed as a bare case label, or the old note-off
        // gate's bare comparison. The marker table names each pitch once and
        // has no case label, so after the fold nothing in song.cpp matches
        // and no owner line is listed.
        {"Which MIDI pitches does the drum parser act on?",
         "the marker pitch table (kMarkerPitches, is_midi_marker_pitch) in src/parse/song.cpp",
         R"(\bcase\s+(95|103|109|110|111|112|116|120)\s*:|\bnote\s*<\s*103\b)",
         "",
         {},
         {},
         "audit finding R7.19; phase 6 task J2-3 (D54)",
         {"case 103: return mop_flag(MAct::Solo, true);",
          "if (is_noteoff && note < 103) return {};",
          "case 120:"},
         {"case kSoloMarkerPitch: return mop_flag(MAct::Solo, true);",
          "if (is_noteoff && !is_midi_marker_pitch(note)) return {};",
          "constexpr int kSoloMarkerPitch = 103;"},
         {},
         {"src/parse/song.cpp"}},
        // Whether a solo run ends here, read off the next timestamp's flag.
        // The Preview's span join walks the flags without looking ahead, so
        // no one-line pattern tells it apart from any other solo check; J3-4
        // repoints it.
        {"Where does a solo section start and end?",
         "Song::solo_sections, built by find_solo_sections in src/parse/song.cpp",
         R"(sequence\[[^\]]*\+\s*1\s*\]\.flag_solo)",
         "",
         {},
         {},
         "audit finding 167; phase 6 task J2-3 (D54)",
         {"i + 1 >= n || !song.sequence[i + 1].flag_solo;",
          "if (song.sequence[k + 1].flag_solo) continue;"},
         {"if (!sequence[i].flag_solo) continue;", "if (ts.flag_solo) {",
          "CHECK(song.sequence[2].flag_solo);"},
         {},
         {"src"}},
        // A test telling a file's format from its extension by hand: a
        // case-sensitive ends_with, or a path's last six bytes compared with
        // ".chart". The tests use the case-sensitive ends_with for nothing
        // else, so any call is flagged.
        {"Which chart format is a test file? (tests)",
         "chart_format_of in src/parse/chart_files.cpp",
         R"(\bends_with\s*\(|\.compare\([^;]*\b6\s*,\s*"\.chart"\))",
         "",
         {},
         {{"tests/test_strutil.cpp", "its ends_with calls test strutil's ends_with itself"}},
         "audit finding 119; phase 6 task J2-3 (D54)",
         {R"(const bool is_mid = ends_with(path, ".mid");)",
          R"(if (!hydra::ends_with(path, ".mid")) continue;)",
          R"(if (p.size() < 6 || p.compare(p.size() - 6, 6, ".chart") != 0) continue;)"},
         {"if (chart_format_of(path) != ChartFormat::Mid) continue;",
          R"(CHECK(ends_with_ci("SONG.Mid", ".mid"));)",
          R"(const std::string chart = corpus::first_chart_with_suffix(".chart");)"},
         {},
         {"tests"}},
        // The stars filter's range typed out as text. The owner builds the
        // top of the range from kMaxStars, so no line matches it. The tests
        // pin the whole sentence on purpose, so they are outside the scope.
        {"What does the stars filter say when its number is out of range?",
         "stars_error in src/app/library_query.cpp",
         R"(\b0 to 7\b)",
         "",
         {},
         {},
         "audit finding 171; phase 6 task J2-5 (D53)",
         {R"(constexpr const char* kStarsError = "stars: needs a number from 0 to 7";)"},
         {R"(return "stars: needs a number from 0 to " + std::to_string(kMaxStars);)",
          "std::optional<int> stars;             // stars:N, N in 0..kMaxStars"},
         {},
         {"src"}},
        // Whether a term limited to one field counts in a column. Matching
        // and highlighting each asked it their own way; both now call the
        // owner. Any line naming QueryField::Any inside either function
        // answers it again.
        {"Does a term apply to a column? (matching)",
         "term_applies_to in src/app/library_query.cpp",
         R"(QueryField::Any)",
         R"(term_applies_to\()",
         {},
         {},
         "audit finding 204; phase 6 task J2-5 (D53)",
         {"case QueryField::Any:"},
         {"if (term_applies_to(term.field, c.column) && contains(c.text, term.folded))",
          "return term_applies_to(QueryField::Any, column);"},
         {},
         {},
         "src/app/library_query.cpp",
         "bool term_matches("},
        {"Does a term apply to a column? (highlighting)",
         "term_applies_to in src/app/library_query.cpp",
         R"(QueryField::Any)",
         R"(term_applies_to\()",
         {},
         {},
         "audit finding 204; phase 6 task J2-5 (D53)",
         {"if (field != QueryField::Any && term.field != QueryField::Any && term.field != field)"},
         {"if (!term_applies_to(term.field, field)) continue;",
          "return term_applies_to(QueryField::Any, column);"},
         {},
         {},
         "src/app/library_query.cpp",
         "std::vector<MatchSpan> match_spans("},
        // Ghosts plus accents, on a bare count or through a variable. The
        // three-way sum that goes on to add normals is all(), another
        // question.
        {"How many dynamic notes does a count hold?",
         "DynamicsCounts::dynamic in src/app/dynamics_breakdown.h",
         R"((\b\w+\.)?\bghost\s*\+\s*\w*\.?accent\b(?!\s*\+))",
         "",
         {},
         {},
         "audit finding 205; phase 6 task J2-5 (D53)",
         {"bool has_dynamics() const { return ghost + accent > 0; }",
          "int dyn = played.ghost + played.accent;"},
         {"int all() const { return ghost + accent + normal; }",
          "int dyn = played.dynamic();",
          "bool has_dynamics() const { return dynamic() > 0; }"},
         {{"src/app/dynamics_breakdown.h", "int dynamic() const { return ghost + accent; }",
           "DynamicsCounts::dynamic, the owner"}},
         {"src", "tests"}},
        // A private byte-by-byte little-endian helper. src/core/little_endian.h
        // is the codec; the store's own read_le/write_le are task J3-6's and
        // are not in this row.
        {"How is a little-endian number written byte by byte?",
         "append_le_u32 and read_le_u32 in src/core/little_endian.h",
         R"(\b(write|read)_u32_le\()",
         "",
         {},
         {},
         "audit finding 195 (the Dynamics half); phase 6 task J2-5 (D53)",
         {"write_u32_le(out, static_cast<uint32_t>(b.rows[i].ghost));",
          "const uint32_t tag_ms = read_u32_le(p);                        p += 4;"},
         {"w.u32(static_cast<uint32_t>(b.rows[i].ghost));", "const uint32_t tag_ms = r.u32();"},
         {},
         {"src"}},
        // A path's suffix tested against a chart extension. A test of
        // ends_with_ci itself passes a literal as the text, not a path, and
        // asks how the suffix test works, so a literal first argument is not
        // flagged.
        {"Which chart format is a path?",
         "chart_format_of in src/parse/chart_files.cpp",
         R"re(ends_with_ci\(\s*[^"\s].*"\.(sng|srb|mid|chart)")re",
         "",
         {"src/parse/chart_files.cpp"},
         {},
         "audit findings 187 and R7.17; phase 6 task J2-5 (D53)",
         {R"(return ends_with_ci(notespath, ".sng") || ends_with_ci(notespath, ".srb");)",
          R"(if (ends_with_ci(notespath, ".sng")) {)",
          R"(if (ends_with_ci(notespath, ".sng")) return sng_audio_from(*container, keep_going);)"},
         {R"(CHECK(ends_with_ci("SONG.Mid", ".mid"));)",
          R"(CHECK_FALSE(ends_with_ci("s", ".sng"));)",
          "if (chart_format_of(notespath) == ChartFormat::Sng) {",
          R"(return ends_with_ci(filename, ".ogg") || ends_with_ci(filename, ".opus") ||)"},
         {},
         {"src", "tools", "tests"}},
        // The path report's page script once found the Beyond edge as the
        // largest tier cutoff and counted rows past it by their ms. The page
        // now reads the edge from the payload and counts rows whose tier
        // tier_for already set to Beyond.
        {"Where does the Beyond tier start on the report page?",
         "beyond_edge_ms and tier_for, carried in the payload by src/app/report.cpp",
         R"(Math\.max\(\.\.\.DATA\.tiers|r\.ms\s*>=?\s*BEYOND)",
         "",
         {},
         {},
         "audit finding 155; phase 6 task J2-2 (D53)",
         {"const BEYOND = Math.max(...DATA.tiers.filter(t => t.cutoff !== null).map(t => t.cutoff));",
          "const beyond = rows.filter(r => r.ms !== null && r.ms > BEYOND).length;"},
         {"const beyond = rows.filter(r => r.tier === 'Beyond').length;",
          "return name === 'Beyond' ? 'Beyond ' + DATA.beyond_edge_ms + ' ms'"},
         {},
         {"src"}},
        // A record's paths come best first from all_paths() (pather::read
        // sorts the roots; each variant sits under its parent). Sorting them
        // by score again is a second answer.
        {"In what order does a record list its paths?",
         "HydraRecord::all_paths, in the order pather::read builds",
         R"(totalscore\(\)\s*>\s*\w+->totalscore\(\))",
         "",
         {},
         {},
         "audit finding 169; phase 6 task J2-2 (D53)",
         {"return a->totalscore() > b->totalscore();"},
         {"CHECK(all[i]->totalscore() <= all[i - 1]->totalscore());",
          "const std::vector<const Path*> paths = record->all_paths();"},
         {},
         {"src", "tools", "tests"}},
        // The subtitle's chart count and the page's chart ids come from one
        // file-local helper in report.cpp.
        {"How many charts does a report page list?",
         "page_charts in src/app/report.cpp",
         R"(\.insert\(\s*r\.hyhash|\.emplace\(\s*r\.hyhash)",
         "",
         {},
         {},
         "audit finding 242; phase 6 task J2-2 (D53); D77 (copies)",
         {"songs.insert(r.hyhash);",
          "if (seen.insert(r.hyhash).second) n += r.copies;"},
         {"for (const auto& [hash, chart] : page_charts(rows)) out.songs += chart.copies;"},
         {{"src/app/report.cpp",
           "charts.emplace(r.hyhash, PageChart{static_cast<int>(charts.size()), r.copies});",
           "page_charts, the owner"}},
         {"src"}},
        // collect_dm_rows and collect_fill_rows set each row's status from
        // the one comparison; the page scripts read it instead of testing the
        // delta's sign. The leaderboard row also keeps that answer in
        // above_optimal, which an off-speed status hides (D64). The C++
        // assignments carry no `r.` and are the owner.
        {"Which side of a report comparison is higher?",
         "the status field set by collect_dm_rows and collect_fill_rows, and the above_optimal "
         "field collect_dm_rows sets from the same comparison",
         R"(r\.delta\s*[<>]\s*0\s*\?)",
         "",
         {},
         {},
         "audit finding 168; phase 6 task J2-2 (D53)",
         {"const deltaCls = (noDelta || r.status === 'other speed') ? 'num dim' : (r.delta < 0 ? 'num neg' : 'num');",
          ": (r.delta < 0 ? '+' + fmt(-r.delta) + ' over' : fmt(r.delta));",
          "const left = under.reduce((a, r) => a + (r.delta > 0 ? r.delta : 0), 0);",
          "const deltaCls = !hasDelta ? 'num dim' : (r.delta > 0 ? 'num pos'"},
         {R"(row.status = above       ? "above optimal")",
          R"(row.status = delta == 0 ? "same" : (delta > 0 ? "1.1 higher" : "1.0 higher");)",
          "const deltaCls = (noDelta || r.status === 'other speed') ? 'num dim' : (r.status === 'above optimal' ? 'num neg' : 'num');",
          "row.above_optimal = above;",
          ": (r.above_optimal ? '+' + fmt(-r.delta) + ' over' : fmt(r.delta));"},
         {},
         {"src"}},
        // A record key's fill flag is encoded once, by Lens::from.
        {"Which fill rule does a record key name?",
         "Lens::from in src/store/record_store.h",
         R"(legacy_fills\s*\?\s*1\s*:\s*0)",
         "",
         {"src/store/record_store.h"},
         {},
         "audit finding 240; phase 6 task J2-2 (D53)",
         {"lens.legacy_fills = legacy_fills ? 1 : 0;"},
         {"store::Lens::from(std::nullopt, 0, 0, legacy_fills)};",
          "old_lens.legacy_fills = 1;"},
         {},
         {"src", "tests"}},
        // Walking the corpus for the first charts that analyze to a path,
        // skipping the rest (continue) or stopping on the first (return).
        {"Which corpus chart is the first with paths?",
         "charts_with_paths and its forms (analyzed_with_paths, first_chart_with_paths, "
         "first_analyzed_with_paths) in tests/corpus_util.h",
         R"(\.record\.paths\.empty\(\)\)\s*(continue|return))",
         "",
         {"tests/corpus_util.h"},
         {},
         "audit finding 277; phase 6 task J2-2 (D53); the return form added by task J4-4",
         {"if (result.song.is_empty() || result.record.paths.empty()) continue;",
          "if (r.song.is_empty() || r.record.paths.empty()) continue;",
          "if (!r.song.is_empty() && !r.record.paths.empty()) return r;",
          "if (!r.record.paths.empty()) return {p, r.record.best_path()};"},
         {"for (const AnalysisResult& result : corpus::analyzed_with_paths(settings, names.size())) {",
          "CHECK(!r.paths.empty());"},
         {},
         {"tests"}},
        // The WCAG relative-luminance formula, typed once with 2.2's
        // threshold.
        {"What is the contrast of two colours?",
         "tests/wcag_util.h",
         R"(0\.03928|0\.04045)",
         "",
         {"tests/wcag_util.h"},
         {},
         "audit finding 288; D54 (the WCAG 2.2 threshold, 0.04045); phase 6 task J2-2",
         {"return c <= 0.03928 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);",
          "return c <= 0.04045f ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);"},
         {"return testwcag::relative_luminance(channel(1), channel(3), channel(5));"},
         {},
         {"src", "tests"}},
        // The game counts stars on the total less the solo bonus. Adding the
        // categories up into the total is a different question.
        {"What is a path's score without the solo bonus?",
         "score_without_solo in src/core/stars.cpp",
         R"(\btotalscore\(\)\s*-\s*(\w+(\.|->))?score_solo\b)",
         "",
         {},
         {},
         "audit finding 166; phase 6 task J2-7 (D53, D54)",
         {"return stars_for_score(star_cutoffs(path), path.totalscore() - path.score_solo);",
          "int64_t multscore = totalscore() - score_solo;"},
         {"return score_base + score_combo + score_sp + score_solo + score_accents +",
          "return stars_for_score(star_cutoffs(path), score_without_solo(path));",
          "CHECK(path.totalscore() == 6680);"},
         {{"src/core/stars.cpp",
           "int64_t score_without_solo(const Path& path) { return path.totalscore() - path.score_solo; }",
           "score_without_solo, the owner"}},
         {"src", "tools", "tests"}},
        // The Stars tab's "With full solo bonus" column and its GUI test read
        // StarCutoffs::with_solo instead of adding the two themselves.
        {"What score shows at a star cutoff with the full solo bonus?",
         "star_cutoffs filling StarCutoffs::with_solo in src/core/stars.cpp",
         R"(\bcutoffs?(\[[^\]]*\])?\s*\+\s*(\w+(\.|->))?solo_bonus\b)",
         "",
         {},
         {},
         "audit finding 166; phase 6 task J2-7 (D53, D54)",
         {"ImGui::TextUnformatted(group_thousands(cutoff + sc.solo_bonus).c_str());",
          "IM_CHECK(text.find(hydra::group_thousands(cutoff + sc.solo_bonus)) != std::string::npos);"},
         {"ImGui::TextUnformatted(group_thousands(sc.with_solo[stars - 1]).c_str());",
          "out.solo_bonus = path.score_solo;",
          "const bool has_solo = sc.solo_bonus > 0;"},
         {{"src/core/stars.cpp",
           "out.with_solo[stars - 1] = out.cutoffs[stars - 1] + out.solo_bonus;",
           "star_cutoffs, the owner"}},
         {"src", "tests"}},
        // The constant multiplied by a note count. Pinning the constant
        // (test_model.cpp) is not an answer.
        {"How many solo-bonus points does a chord earn?",
         "solo_bonus in src/core/scoring.cpp",
         R"(\bkSoloBonusPerNote\)?\s*\*|\*\s*(\w+::)*kSoloBonusPerNote\b)",
         "",
         {},
         {},
         "audit finding 163; phase 6 task J2-7 (D53, D54)",
         {"store_soloscore(kSoloBonusPerNote * timestamp.chord.count());",
          "ts.flag_solo ? static_cast<int64_t>(kSoloBonusPerNote) * ts.chord.count() : 0;",
          "int solo = ts.chord.count() * hydra::kSoloBonusPerNote;"},
         {"CHECK(kSoloBonusPerNote == 100);",
          "inline constexpr int kSoloBonusPerNote = 100;",
          "store_soloscore(solo_bonus(timestamp.chord, timestamp.flag_solo));"},
         {{"src/core/scoring.cpp", "return flag_solo ? kSoloBonusPerNote * chord.count() : 0;",
           "solo_bonus, the owner"}},
         {"src", "tools", "tests"}},
        // A running combo stepped by a whole chord's note count. The scorer
        // steps one note at a time (combo += 1), so it has no owner line.
        {"What is the combo after a chord?",
         "CategoryScores::combo_after in src/core/scoring.h",
         R"(\bcombo_?\s*\+=\s*.*\bcount\(\))",
         "",
         {},
         {},
         "audit finding 164; phase 6 task J2-7 (D53, D54)",
         {"combo_ += timestamp.chord.count();", "combo += ts.chord.count();"},
         {"combo += 1;", "note_scores.combo_after = combo;", "combo_ = scores.combo_after;"},
         {},
         {"src", "tools", "tests"}},
        // The chart's note total: chord note counts added up, or a note total
        // added to as a running sum. A running combo is the row above's
        // question, so it is left out here. Tests stay out of scope: they
        // build records by changing a stored notecount on purpose.
        {"How many notes does the chart have?",
         "Song::note_count in src/parse/song.cpp",
         R"(\b(?!combo)\w+\s*\+=\s*\w+(\.|->)chord\.count\(\)|\bnotecount\s*\+=)",
         R"(\bnote_count\(\))",
         {},
         {},
         "audit finding 255 (D74)",
         {"p.notecount += e.notecount;", "proto_base_edge_->notecount += count;",
          "total += ts.chord.count();"},
         {"combo += ts.chord.count();", "combo_ += timestamp.chord.count();",
          "path.notecount = note_count;"},
         {{"src/parse/song.cpp",
           "for (const SongTimestamp& ts : sequence) n += ts.chord.count();",
           "Song::note_count"}}},
        // ---- phase 6 task J2-8: render callers and the Onyx numbers ----
        // The highway's far end is now plus speed times secs_future; time_to_z,
        // z_to_time and the draw list's window all ask far_time.
        {"Where is the far end of the highway?",
         "far_time in src/render/highway_draw.cpp",
         R"(\b(?:speed\s*\*\s*(?:cfg\.track|T)\.secs_future|(?:cfg\.track|T)\.secs_future\s*\*\s*speed)\b)",
         "",
         {},
         {},
         "audit finding 224; phase 6 task J2-8 (D54)",
         {"const double far_time = now_s + speed * cfg.track.secs_future;",
          "const double far_t = now_s + speed * T.secs_future;",
          "const double far_t = T.secs_future * speed;"},
         {"const double far_t = far_time(cfg, now_s, speed);",
          "CHECK(c.track.secs_future == doctest::Approx(1.35));"},
         {{"src/render/highway_draw.cpp", "return now_s + speed * cfg.track.secs_future;",
           "far_time, the owner"}}},
        // The track rectangle: the size floor (below 1 counts as 1), the
        // camera aspect (width over track height) and the bottom anchor (the
        // track's top row is the image height less the track height). The last
        // alternative is the renderer's old fade rectangle, which divided the
        // track height by the image height; it now converts track_rect's top
        // row to screen coordinates.
        {"Where does the track rectangle sit, and at what size?",
         "track_rect in src/render/highway_draw.cpp",
         R"(std::max\(\s*1\s*,\s*(?:width|height)\s*\))"
         R"(|static_cast<float>\([^()]*\)\s*/\s*static_cast<float>\((?:\w+\.)*(?:th|track_h|track_height)\))"
         R"(|\b(?:\w+\.)*(?:h|height)\s*-\s*(?:\w+\.)*(?:th|track_h|track_height)\b)"
         R"(|static_cast<float>\((?:\w+\.)*(?:th|track_h|track_height)\)\s*/\s*static_cast<float>\()",
         "",
         {},
         {},
         "audit finding 221; phase 6 task J2-8 (D54)",
         {"const int w = std::max(1, width);",
          "const int h = std::max(1, height);",
          "const HighwayCamera cam = make_camera(cfg, static_cast<float>(w) / static_cast<float>(th));",
          "out.y = static_cast<float>(h - th) + (1.0f - XMVectorGetY(ndc)) * 0.5f * static_cast<float>(th);",
          "return highway_span_at(cfg, width, height, static_cast<float>(std::max(1, height))).left - gap;",
          "d.width = std::max(1, width);",
          "d.height = std::max(1, height);",
          "HighwayCamera cam = make_camera(cfg, static_cast<float>(d.width) / static_cast<float>(d.track_h));",
          "fc.rect_max = XMFLOAT2(1.0f, -1.0f + 2.0f * static_cast<float>(d.track_h) / "
          "static_cast<float>(d.height));"},
         {"const int want = std::max(1, cfg.hydra.msaa);",
          "XMFLOAT2(1.0f, 1.0f - 2.0f * static_cast<float>(d.rect.top) / static_cast<float>(d.rect.height));",
          "const HighwayCamera cam = make_camera(cfg, r.aspect);",
          "r.track_height = std::max(1, t);"},
         {{"src/render/highway_draw.cpp", "r.width = std::max(1, width);", "track_rect, the owner"},
          {"src/render/highway_draw.cpp", "r.height = std::max(1, height);", "track_rect, the owner"},
          {"src/render/highway_draw.cpp", "r.top = r.height - r.track_height;", "track_rect, the owner"},
          {"src/render/highway_draw.cpp",
           "r.aspect = static_cast<float>(r.width) / static_cast<float>(r.track_height);",
           "track_rect, the owner"}}},
        // A polygon becomes triangles as a fan from its first corner (Onyx's
        // triangulate). load_obj's faces and the built flat and box meshes all
        // push their vertices through push_fan.
        {"In what order does a polygon become triangles?",
         "push_fan in src/render/obj_loader.cpp",
         R"(\.vertices\.push_back\()",
         "",
         {},
         {},
         "audit finding 226; phase 6 task J2-8 (D54)",
         {"m.vertices.push_back(a);", "m.vertices.push_back(b);", "m.vertices.push_back(c);",
          "m.vertices.push_back(a);", "m.vertices.push_back(c);", "m.vertices.push_back(d);",
          "mesh.vertices.push_back(make_vertex(corners[0]));"},
         {"for (int k = 0; k < 3; ++k) sorted.push_back(mesh.vertices[tri * 3 + k]);",
          "push_fan(m, {a, b, c, d});"},
         {{"src/render/obj_loader.cpp", "mesh.vertices.push_back(corners[0]);", "push_fan, the owner"},
          {"src/render/obj_loader.cpp", "mesh.vertices.push_back(corners[i]);", "push_fan, the owner"},
          {"src/render/obj_loader.cpp", "mesh.vertices.push_back(corners[i + 1]);",
           "push_fan, the owner"}},
         {"src/render/obj_loader.cpp"}},
        // Which end of a railing's X extent is its outer edge. railing_x owns
        // the extent; railing_outer_x picks the outer end, and the SP end
        // markers and highway_span_at ask it. A railing_x call with a literal
        // side is how a caller picked an end by hand.
        {"Which X edge of a railing faces away from the lanes?",
         "railing_outer_x in src/render/highway_draw.cpp",
         R"(\bright\s*\?\s*x2\s*:\s*x1\b|railing_x\(\s*cfg\s*,\s*(?:true|false)\s*,)",
         "",
         {},
         {},
         "SPTRI derive-once review, finding 1 (D81)",
         {"const float outer = right ? x2 : x1;", "railing_x(cfg, false, xl, inner);"},
         {"railing_x(cfg, right, rail.lo[0], rail.hi[0]);",
          "const float outer = railing_outer_x(cfg, right);"},
         {{"src/render/highway_draw.cpp", "return right ? x2 : x1;", "railing_outer_x, the owner"}}},
        // Where a Preview text box's line may break: wrap_words walks the
        // spaces, and widest_word takes the widest line it makes at width 0.
        {"Where may a line break fall in a Preview text box?",
         "wrap_words in src/render/overlay_layout.cpp",
         R"(\.find\(\s*' ')",
         "",
         {},
         {},
         "audit finding 75; phase 6 task J2-8 (D54)",
         {"size_t end = text.find(' ', start);"},
         {"while (start < text.size() && text[start] == ' ') ++start;",
          "for (const std::string& line : wrap_words(text, 0.0f, width_of, keep_last))"},
         {{"src/render/overlay_layout.cpp", "size_t next = text.find(' ', end);",
           "wrap_words, the owner"}},
         {"src/render/overlay_layout.cpp"}},
        // The Preview's numbers live in assets/preview/3d-config.json alone; a
        // PreviewConfig that was not loaded holds zeros. A field set to a
        // number, or a brace initializer holding one, would be a second copy.
        // Color's white and Vec3's origin are the types' own defaults, not
        // Onyx's numbers, so their two member lines are left out.
        {"Whose numbers does the Preview draw with?",
         "assets/preview/3d-config.json, read by load_preview_config (docs/adr/0008)",
         R"(^(?!\s*float [rx] = ).*(?:\b\w+\s*=\s*-?\d|\w\{[^}]*\d))",
         "",
         {},
         {},
         "audit finding 220; phase 6 task J2-8 (D54)",
         {"float secs_future = 1.35f;  // events this far ahead sit at z_future",
          "Color background{0x1c / 255.0f, 0x1d / 255.0f, 0x2b / 255.0f, 1};",
          "Vec3 camera_position{0, 1.4f, 3};",
          "LightConfig light{{0, -0.5f, 0.5f}};",
          "int msaa = 4;  // mirrors Onyx's prefMSAA default",
          "float y = -1;  // the floor"},
         {"float r = 1, g = 1, b = 1, a = 1;",
          "float x = 0, y = 0, z = 0;",
          "float secs_future{};  // events this far ahead sit at z_future",
          "LightConfig light{};"},
         {},
         {"src/render/preview_config.h"}},
        // ---- tool text and the stem readers (phase 6 task J2-6) ----
        // The dump writer's line reads act.sqout_tick off the engine's
        // activation, not the JSON, so it does not match.
        {"Which field names a dump window's squeezed-out chord?",
         "read_sqout in tools/replay_json.cpp",
         R"(act\["sqout_tick"\])",
         "",
         {},
         {},
         "audit finding 128 (tool half); phase 6 task J2-6 (D53)",
         {R"(if (act.contains("sqout_tick") && act["sqout_tick"].is_number() &&)",
          R"(w.sqout_tick = act["sqout_tick"].get<int64_t>();)"},
         {R"({"sqout_tick", act.sqout_tick ? *act.sqout_tick : -1},)"},
         {{"tools/replay_json.cpp",
           R"(if (act.contains("sqout_tick") && act["sqout_tick"].is_number() &&)",
           "read_sqout, the owner"},
          {"tools/replay_json.cpp", R"(act["sqout_tick"].get<int64_t>() >= 0) {)",
           "read_sqout, the owner"},
          {"tools/replay_json.cpp", R"(w.sqout_tick = act["sqout_tick"].get<int64_t>();)",
           "read_sqout, the owner"}},
         {"tools"}},
        {"How many frames are the Opus packet and pre-roll?",
         "frames_of_ms in src/audio/frames.h",
         R"(\b(5760|19200)\b)",
         "",
         {},
         {},
         "audit finding 227; phase 6 task J2-6 (D53)",
         {"constexpr int kMaxFrame = 5760;        // 120 ms, the largest Opus packet",
          "constexpr int64_t kPreRoll = 19200;    // 400 ms decoder warm-up before a seek target"},
         {"const int kMaxFrame = static_cast<int>(frames_of_ms(120.0, kRate));",
          "const int64_t kPreRoll = frames_of_ms(400.0, kRate);"},
         {},
         {"src"}},
        {"How long is an ID3v2 tag, and what sits behind it?",
         "id3v2_tag_length in src/core/audio_sniff.cpp",
         R"(&\s*0x7F\)\s*<<\s*21)",
         "",
         {},
         {},
         "audit R7.23; phase 6 task J2-6 (D53)",
         {"std::size_t tag = (static_cast<std::size_t>(d[6] & 0x7F) << 21) |",
          "pos = 10 + ((static_cast<std::size_t>(b[6] & 0x7F) << 21) | ((b[7] & 0x7F) << 14) |"},
         {"std::size_t pos = id3v2_tag_length(b.data(), b.size());"},
         {{"src/core/audio_sniff.cpp",
           "const std::size_t body = (static_cast<std::size_t>(data[6] & 0x7F) << 21) |",
           "id3v2_tag_length, the owner"}},
         {"src", "tools", "tests"}},
        {"How many reservoir bytes does an MP3 frame leave?",
         "reservoir_after in src/audio/ma_reader.cpp",
         R"(std::min\(kMaxReservoir)",
         "",
         {},
         {},
         "audit R7.24; phase 6 task J2-6 (D53)",
         {"reserv = std::min(kMaxReservoir, std::min(reserv, back) + static_cast<int>(s.own[j]));",
          "reserv = std::min(kMaxReservoir, std::min(reserv, static_cast<int>(s.back[j])) +"},
         {"reserv = reservoir_after(reserv, back, s.own[j]);"},
         {{"src/audio/ma_reader.cpp",
           "return std::min(kMaxReservoir, std::min(reserv, back) + own);",
           "reservoir_after, the owner"}},
         {"src/audio/ma_reader.cpp"}},
        // The copied dr_mp3 declarations are verbatim and guarded by the
        // miniaudio version static_assert, so their two 511s stay.
        {"How big is dr_mp3's bit reservoir?",
         "MA_DR_MP3_MAX_BITRESERVOIR_BYTES, in the copied dr_mp3 declarations of "
         "src/audio/ma_reader.cpp",
         R"(\b511\b)",
         "",
         {},
         {},
         "audit R7.24; phase 6 task J2-6 (D53)",
         {"constexpr int kMaxReservoir = 511;  // dr_mp3's MA_DR_MP3_MAX_BITRESERVOIR_BYTES"},
         {"constexpr int kMaxReservoir = MA_DR_MP3_MAX_BITRESERVOIR_BYTES;"},
         {{"src/audio/ma_reader.cpp", "#define MA_DR_MP3_MAX_BITRESERVOIR_BYTES      511",
           "the macro, in the verbatim dr_mp3 copy"},
          {"src/audio/ma_reader.cpp", "ma_uint8 header[4], reserv_buf[511];",
           "dr_mp3's own struct field, in the verbatim copy (kept byte for byte)"}},
         {"src/audio/ma_reader.cpp"}},
        {"Which test helper builds a small ID3v2 tag?",
         "id3_tag in tests/audio_util.h",
         R"(\{\s*'I',\s*'D',\s*'3')",
         "",
         {},
         {},
         "phase 6 task J2-6 derive-once review, finding 1",
         {"std::vector<uint8_t> t = bytes({'I', 'D', '3', 3, 0, flags, 0, 0, 0, 20});",
          "std::vector<uint8_t> tagged = {'I', 'D', '3', 3, 0, 0, 0, 0, 0, 20};",
          "CHECK(sniff_format(bytes({'I', 'D', '3', 3, 0, 0})) == AudioFormat::Mp3);"},
         {"std::vector<uint8_t> tagged = id3_tag(0);"},
         {{"tests/audio_util.h",
           "std::vector<uint8_t> t = {'I', 'D', '3', 3, 0, static_cast<uint8_t>(flags),",
           "id3_tag, the owner"}},
         {"tests"}},
        // ---- report pages (phase 7 task RP) ----
        // A report page's file name typed outside its constant. The GUI test
        // harness clears both pages, so it is scanned too.
        {"What file name does each report page have?",
         "kPathReportFileName and kDmReportFileName in src/app/report_files.h",
         R"(hydra_(paths|dmcompare)\.html)",
         "",
         {},
         {},
         "audit finding 202, D51's code-only calls (phase 7 task RP)",
         {"std::string out = \"hydra_paths.html\";",
          "std::wstring dm_report_html_path() { return html_artifact_path(L\"hydra_dmcompare.html\"); }"},
         {"std::string out = hydra::app::kPathReportFileName;",
          "std::string out = \"fill_compare.html\";"},
         {{"src/app/report_files.h",
           "inline constexpr const char* kPathReportFileName = \"hydra_paths.html\";",
           "kPathReportFileName, the owner"},
          {"src/app/report_files.h",
           "inline constexpr const char* kDmReportFileName = \"hydra_dmcompare.html\";",
           "kDmReportFileName, the owner"}},
         {"src", "tools", "tests/ui/uitest_harness.cpp"}},
        // Clone Hero's SP cap written as text, where the leaderboard page
        // names it.
        {"Which SP cap does the leaderboard comparison name?",
         "kCloneHeroSpCap in src/core/model.h",
         R"(SP cap 4\b)",
         "",
         {},
         {},
         "audit finding 170, D51's code-only calls (phase 7 task RP)",
         {"{k:'optimal', t:'Hydra opt', num:true,  d:'The optimal score Hydra found for the chart "
          "at SP cap 4, the Clone Hero rule.'},",
          "\"at SP cap 4: analyze them, then compare again.\";"},
         {"\"at SP cap \" + std::to_string(kCloneHeroSpCap) + \": analyze them, then compare "
          "again.\";",
          "\"SP cap 40\""},
         {}},
        // ---- app state, jobs and the toolbar (phase 6 task J2-4) ----
        // A batch or analyze job tested for "held and not finished". The scan
        // job's guard is a different job with no owner yet; the GUI harness's
        // dump prints each job's state, a different question.
        {"Is a batch or an analysis running?",
         "batch_running in src/ui/app_state.cpp",
         R"(\b(batch_job\s*&&\s*!\s*((a|app)\.)?batch_job->snapshot\(\)\.finished|view_job\s*&&\s*!\s*((a|app)\.)?view_job->finished\(\)))",
         "",
         {},
         {},
         "audit finding 254; phase 6 task J2-4 (D53); storage-T2 review finding 3 (the click's job)",
         {"if (batch_job && !batch_job->snapshot().finished) return;",
          "if (view_job && !view_job->finished()) view_job->cancel();",
          "const bool batch_busy = app.batch_job && !app.batch_job->snapshot().finished;",
          "if (a.view_job && !a.view_job->finished()) return true;"},
         {"if (scan_job && !scan_job->snapshot().finished) return;",
          R"(a.batch_job ? (a.batch_job->snapshot().finished ? "finished" : "running") : "-",)",
          "if (batch_job && !batch_finish_seen_ && batch_job->snapshot().finished) {",
          "if (view_thread_alive()) view_job->cancel();"},
         {{"src/ui/app_state.cpp",
           "bool AppState::batch_running() const { return batch_job && !batch_job->snapshot().finished; }",
           "batch_running, the owner"},
          {"src/ui/app_state.cpp",
           "bool AppState::view_thread_alive() const { return view_job && !view_job->finished(); }",
           "view_thread_alive, the owner"}},
         {"src", "tests"}},
        // A cached copy of the library's chart count. The model's own rows
        // are the count; the cache is gone, so no line may name it.
        {"How many charts does the library hold?",
         "LibraryModel::rows() in src/ui/library_model.h",
         R"(\blibrary_total\b)",
         "",
         {},
         {},
         "audit finding 131; phase 6 task J2-4 (D53)",
         {"const int64_t analyzable = searching ? static_cast<int64_t>(app.library_match_count()) : app.library_total;"},
         {"const int64_t analyzable = searching ? static_cast<int64_t>(app.library_match_count()) : library_size;"},
         {},
         {"src", "tests"}},
        // A "checked at" time tested against an interval, or the report
        // check's own interval, which D54 folded into kFileCheckSeconds.
        {"How long does the UI trust a cached file-exists answer?",
         "AppState::cached_file_check in src/ui/app_state.h",
         R"(\bkReportCheckSeconds\b|_checked_at\s*(<|>=))",
         "",
         {"src/ui/app_state.h"},
         {},
         "audit finding 207; D54 (one interval, one caching helper)",
         {"now - details_ui.file_checked_at >= kFileCheckSeconds) {",
          "now - library_ui.report_checked_at >= kReportCheckSeconds) {",
          "if (library_ui.report_checked_at < 0.0 ||"},
         {"details_ui.file_checked_at = -1.0;",
          "library_ui.report_checked_at = -1.0;  // a new report: look at once"}},
        // Publishing a job's finished flag by hand. Every job ends through
        // run_guarded or fail in job_base.h.
        {"What does a failed job record?",
         "ResultJobBase::fail in src/ui/job_base.h",
         R"(finished_\.store\(true\))",
         "",
         {"src/ui/job_base.h"},
         {},
         "audit finding 212; phase 6 task J2-4 (D53)",
         {"finished_.store(true);"},
         {"bool finished() const { return finished_.load(); }"},
         {},
         {"src"}},
        // The removed Analyze button's two labels typed as text (D87 item 6:
        // a click analyzes). Only the GUI test that checks the button is
        // gone may name them. The closing quote keeps the "Re-analyze to
        // refresh it." sentences out.
        {"Where may the removed Analyze button's labels appear?",
         "test_no_analyze_button in tests/ui/uitest_details.cpp, which checks they are gone",
         R"re((Analyze this song|Re-analyze)")re",
         "",
         {"tests/ui/uitest_details.cpp"},
         {},
         "audit finding 289; phase 6 task J2-4 (D53); D87 item 6",
         {R"(return h.app->viewed.status == hydra::store::RecordStatus::NotAnalyzed ? "**/Analyze this song")",
          R"(: "**/Re-analyze";)"},
         {R"(IM_CHECK(visible_text(h).find("Also re-analyze") != std::string::npos);)",
          R"("from different rules in hydra_rules.ini. Re-analyze to refresh it.");)",
          R"("Click the song or run a batch to refresh it.")"},
         {},
         {"src", "tests"}},
        // The batch button's search label typed as text.
        {"What does the batch button say while a search is typed?",
         "analyze_search_label in src/ui/library_toolbar.cpp",
         R"("Analyze search \()",
         R"(\banalyze_search_label\()",
         {"src/ui/library_toolbar.cpp"},
         {},
         "audit finding 290; phase 6 task J2-4 (D53)",
         {R"(? "Analyze search (" + group_thousands(analyzable) + ")...")",
          R"("Analyze search (" +)"},
         {R"(CHECK(hydra::ui::detail::analyze_search_label(1) == "Analyze search (1)...");)",
          R"(const std::string label = searching ? analyze_search_label(analyzable) : "Analyze library...";)"},
         {},
         {"src", "tests"}},
        // A scan row built from a library entry's fields by position. The
        // forward direction (a scan row into the library) is the row above
        // owned by to_library_entry.
        {"How does a library entry become a scan row?",
         "scan_item_of in src/ui/library_jobs.cpp",
         R"(items_\.push_back\(\s*\{\s*\w+\.md5|\bScanItem(\s+\w+)?\s*\{\s*\w+\.md5\b)",
         "",
         {"src/ui/library_jobs.cpp"},
         {},
         "audit finding 256 (the reverse, from the phase 6 ledger); phase 6 task J2-4 (D53)",
         {"items_.push_back({e.md5, e.title, e.artist, e.charter, e.notespath, e.rootfolder});",
          "app::ScanItem item{e.md5, e.title, e.artist};"},
         {"entries.push_back({item.md5, item.title, item.artist, item.charter, item.notespath,",
          "for (const store::ChartLibraryEntry& e : entries) items_.push_back(scan_item_of(e));"},
         {},
         {"src", "tools", "tests"}},
        // A batch that picks its own charts with the store's SQL search. The
        // library screen's own search decides which rows match.
        {"Which charts does a batch analyze?",
         "the rows the library screen hands over: AppState::open_batch_confirm and BatchJob's "
         "vector constructor (src/ui/app_state.cpp, src/ui/library_jobs.h)",
         R"(BatchJob\(std::optional<std::string>|list_chart_library\(\s*search)",
         "",
         {},
         {},
         "audit finding 113; phase 6 task J2-4 (D53)",
         {"BatchJob(std::optional<std::string> search, app::BatchRun run,",
          ": store_.list_chart_library(search_, 0, -1);  // LIMIT -1 = no limit"},
         {"library.set_charts(store->list_chart_library(std::nullopt, 0, -1));  // -1 = no limit",
          "BatchJob(std::vector<store::ChartLibraryEntry> charts, app::BatchRun run,"},
         {},
         {"src", "tests"}},
        // A batch count worked out again from other counts: "skipped" as the
        // scan list less the total, or "analyzed" as finished less failed.
        // BatchProgress carries every count, filled by run_batch.
        {"How many charts did a batch analyze, skip and fail?",
         "run_batch in src/app/analysis.cpp (BatchProgress's counts)",
         R"(size\(\)\)?\s*-\s*(\w+(\.|->))?total\b|\bcompleted\s*-\s*(\w+(\.|->))?failed\b)",
         "",
         {},
         {},
         "audit finding 142; D51 call 26 (phase 7 task LB1); D76 (every copy counted)",
         {"snap_.skipped = static_cast<int>(items_.size()) - p.total;",
          "skipped = static_cast<int>(scanitems.size()) - p.total;",
          "skipped = scanitems.size() - p.total;",
          R"(return counted(s.completed - s.failed, "analyzed", "analyzed") + " \xC2\xB7 " +)",
          "const int kept = s.completed - s.failed;"},
         {"progress.completed = progress.analyzed + progress.failed;",
          "snap_.completed = p.completed;",
          "for (int i = static_cast<int>(digits.size()) - 1; i >= 0; --i) {"},
         {},
         {"src"}},
        // The settings bar's lock decision: read once per frame so a batch
        // ending mid-frame cannot send the bar to the one-song message with
        // no analyze job behind it.
        {"What locks the analysis settings?",
         "settings_lock in src/ui/app_state.cpp",
         R"(\b(app|a)\.(batch_running|analyze_running)\(\))",
         R"(\bsettings_lock\b)",
         {},
         {},
         "p6-uicrash (the crash this fix addresses)",
         {"const bool batch_busy = app.batch_running();"},
         {"const AppState::SettingsLock lock = app.settings_lock();",
          "const bool locked = lock != AppState::SettingsLock::None;"},
         {},
         {"src/ui/settings_bar.cpp"}},
        // The hit window's default cut to a whole number. The setting, the
        // report job and the report keep its decimal (D51 call 15).
        {"What is the hit window's default?",
         "kDefaultHitWindowMs in src/core/model.h",
         R"(static_cast<int>\(\s*(hydra::)?kDefaultHitWindowMs\s*\))",
         "",
         {},
         {},
         "audit finding 140; D51 call 15 (phase 7 task LB1)",
         {"int hit_window_ms = static_cast<int>(kDefaultHitWindowMs);",
          "int hit_window_ms = static_cast<int>(kDefaultHitWindowMs));",
          "CHECK(s.hit_window_ms == static_cast<int>(hydra::kDefaultHitWindowMs));"},
         {"double hit_window_ms = kDefaultHitWindowMs;",
          "bool open_when_done, double hit_window_ms = kDefaultHitWindowMs);"},
         {},
         {"src", "tests"}},
        // ---- app state, library screen and toolbar (phase 7 task LB2) ----
        // A row's notespath compared with the selection's. Tests that check
        // which song a click selected read the selection first and are left
        // alone.
        {"Which library row is the selected one?",
         "AppState::is_selected_row in src/ui/app_state.cpp",
         R"(\bnotespath\s*(==|!=)\s*(\w+(\.|->))*selected|\bselected(\w*(\.|->))*notespath\s*(==|!=))",
         "",
         {},
         {},
         "audit finding 143 (phase 7 task LB2)",
         {"if (view_row(i).notespath != selected->notespath) continue;",
          "if (rows[order[k]].entry.notespath == selected_path) {",
          "const bool selected = !selected_path.empty() && row.entry.notespath == selected_path;",
          "analyze_job->song().notespath == selected->notespath;",
          "if (selected->notespath == e.notespath) return true;",
          "if (selected->notespath != row.notespath) continue;",
          "if (selected->notespath == view_row(i).notespath) return i;"},
         {"const std::string selected_path = app.selected ? app.selected->notespath : std::string();",
          "const bool selected = app.is_selected_row(row.entry);"},
         {{"src/ui/app_state.cpp", "return selected && row.notespath == selected->notespath;",
           "is_selected_row, the owner"}},
         {"src"}},
        // Song folders and the scan job tested together, or the old scan-only
        // guard in start_scan.
        {"May a library scan start now?",
         "AppState::can_scan in src/ui/app_state.cpp",
         R"(chartfolders\.empty\(\)[^;]*scan_job|scan_job[^;]*chartfolders\.empty\(\)|if\s*\(\s*scan_job\s*&&\s*!\s*scan_job->snapshot\(\)\.finished\)\s*return;)",
         "",
         {},
         {},
         "audit finding R7.3; D51 call 24 (phase 7 task LB2)",
         {"const bool can_scan = !app.settings.chartfolders.empty() && !batch_busy && !app.scan_job;",
          "if (!app.settings.chartfolders.empty() && !app.scan_job) {",
          "if (scan_job && !scan_job->snapshot().finished) return;"},
         {"const bool scan_off = !app.can_scan();",
          "if (app.library_ui.folders_changed && !app.settings.chartfolders.empty()) {",
          "if (app.scan_job) ImGui::OpenPopup(\"Scanning charts\");"},
         {{"src/ui/app_state.cpp",
           "return !settings.chartfolders.empty() && !scan_job && !batch_running();",
           "can_scan, the owner"}},
         {"src", "tests"}},
        // A job list of its own: one job tested for "held and not finished"
        // and the answer returned as busy. A wait on one Preview load (no
        // "held and" test) is a different question.
        {"Is any background job still running?",
         "AppState::any_job_running in src/ui/app_state.cpp",
         R"(&&\s*!?\s*[\w.]*(->\w+\(\))*->(finished\(\)|snapshot\(\)\.finished|loading\(\)|busy\(\))\)\s*return true;)",
         "",
         {},
         {},
         "audit finding 109 (phase 7 task LB2)",
         {"if (a.scan_job && !a.scan_job->snapshot().finished) return true;",
          "if (a.report_job && !a.report_job->finished()) return true;",
          "if (a.preview && a.preview->loading()) return true;"},
         {"bool jobs_busy(Harness& h) { return h.app->any_job_running(); }",
          "if (!job || !job->finished()) return;",
          "if (!h.app->preview->loading()) return true;"},
         {},
         {"src", "tests"}},
        // A summary's score or stars tested for presence: whether the record
        // has a scored best path. test_store checks the stored columns
        // themselves, which is what the owner reads.
        {"Does this result have a scored best path?",
         "PathSummary::has_scored_best_path in src/store/record_store.h",
         R"(\bsummary\.(score|stars)\.has_value\(\)|!\s*(\w+\.)*summary\.score\b|\bsummary\.(score|stars)\s*\?)",
         "",
         {"src/store/record_store.h"},
         {{"tests/test_store.cpp",
           "checks that only a Ready answer carries the stored summary columns, the fields the "
           "owner reads"}},
         "D51 call 11; audit finding 88 (phase 7 task LB2)",
         {"const bool xs = x.summary.score.has_value();",
          "if (!summary.score) return bestpath;",
          "row.summary.score ? hydra::group_thousands(*row.summary.score) : \"-\";"},
         {"const bool xs = x.summary.has_scored_best_path();",
          "if (summary.stars) facts += \" \\xC2\\xB7 \" + counted(*summary.stars, \"star\", \"stars\");"},
         {},
         {"src", "tests"}},
        // The Compare button's three refusal sentences typed outside the gate.
        {"Why can't these settings be compared with the leaderboard?",
         "why_not_comparable in src/app/dm_report.cpp",
         R"(Needs (Expert:|SP cap|%s fills|Clone Hero 1\.1 fills)|untick \\"1\.0 fills\\")",
         "",
         {"src/app/dm_report.cpp"},
         {},
         "audit finding 170 (phase 7 tasks RP and LB2)",
         {R"(ImGui::SetTooltip("Needs Expert: the leaderboard only has Expert scores.");)",
          R"(ImGui::SetTooltip("Needs SP cap %d, Clone Hero's rule: the leaderboard's scores ")",
          R"(ImGui::SetTooltip("Needs %s fills: untick \"1.0 fills\". The leaderboard is ")"},
         {R"(ImGui::SetTooltip("%s", refused.c_str());)",
          R"(ImGui::SetTooltip("Compare a dmleaderboards.com player's scores against your library");)"},
         {}},
        // The row's full SP value less what backend_row_value pays it as the
        // squeezed-out chord.
        {"What does squeezing out a row cost?",
         "core::sqout_cost in src/core/backend_value.h",
         R"((\.|->)points\s*-\s*value\b)",
         "",
         {"src/core/backend_value.h"},
         {},
         "audit finding 150; phase 6 task J3-1 (D53, D54)",
         {"const int lost = row.points - value;",
          "\" <-- squeezed out (-%d)\", bsq.points - value);"},
         {"return points - backend_row_value(offset_ms, points, sqout_points, SqOutPosition::Exact,",
          "const int lost = core::sqout_cost(off, row.points, row.sqout_points, leeway_ms);"},
         {},
         {"src"}},
        // The value_or fallback. A bare dereference of the optional reads the
        // same fact; the J3-9 row at the end of rules() scans for those.
        {"What is a backend row's offset?",
         "BackendSqueeze::offset in src/core/model.cpp",
         R"(offset_ms\.value_or\()",
         "",
         {},
         {},
         "audit finding 346; phase 6 task J3-1 (D53, D54)",
         {"double off = offset_ms.value_or(0.0);",
          "if (within_squeeze_window(bsq.offset_ms.value_or(0.0)) || is_sqout_backend(bsq))"},
         {"if (!sqout || !sqout->offset_ms || *sqout->offset_ms != sq.offset_ms)",
          "const double off = offset();"},
         {},
         {}},
        // A comparison with the kick lane beside a flag field, or opening a
        // branch, decides the flag split itself.
        {"Which lanes may carry a flag, and what does the flag mean?",
         "lane_allows_flag, set_lane_flag and lane_flag in src/core/model.cpp",
         R"([!=]=\s*NoteColor::Kick\b.*(is2x|cymbal|\)\s*\{\s*$))",
         "",
         {},
         {},
         "audit finding 190; phase 6 task J3-1 (D53, D54)",
         {"if (color == NoteColor::Kick) {", "return c == NoteColor::Kick || allows_cymbals(c);",
          "if (note.colortype == NoteColor::Kick) return note.is2x ? \"2x kick\" : \"Kick\";"},
         {"set_lane_flag(add_note(NoteColor::Kick));",
          "if (note.colortype == NoteColor::Kick) return lane_flag(note) ? \"2x kick\" : \"Kick\";",
          "CHECK(k2.colortype == NoteColor::Kick);"},
         {{"src/core/model.cpp", "return c == NoteColor::Kick || allows_cymbals(c);",
           "lane_allows_flag, the owner"},
          {"src/core/model.cpp", "if (note.colortype == NoteColor::Kick) note.is2x = true;",
           "set_lane_flag, the owner"},
          {"src/core/model.cpp",
           "return note.colortype == NoteColor::Kick ? note.is2x : note.is_cymbal();",
           "lane_flag, the owner"}},
         {"src", "tests"}},
        // Tests pin the shown text and may type it. "Ghost Notes:" is a score
        // category's name, a different question.
        {"What is a note's dynamic called in text?",
         "dynamic_str and dynamic_label in src/core/model.cpp",
         R"("[ (]*(Ghost|Accent)\)?")",
         "",
         {},
         {},
         "audit finding 241; phase 6 task J3-1 (D53, D54)",
         {"ImGui::TableSetupColumn(\"Ghost\");",
          "case NoteDynamicType::Ghost: mod = \" (Ghost)\"; break;"},
         {"ImGui::TableSetupColumn(dynamic_label(NoteDynamicType::Ghost).c_str());",
          "lines.push_back(\"Ghost Notes:      \" + right10(path.score_ghosts));"},
         {},
         {"src"}},
        // Test JSON fixtures pin the wire format and stay out of scope.
        {"How is a squeeze kind spelled?",
         "SPSqueeze::type_name in src/core/model.h, read back by squeeze_kind_from_name",
         R"re("(SqIn|SqOut)")re",
         "",
         {},
         {},
         "audit finding 203; phase 6 task J3-1 (D53, D54)",
         {"if (sq.value(\"kind\", std::string()) != \"SqOut\") continue;"},
         {"if (squeeze_kind_from_name(sq.value(\"kind\", std::string())) != SqueezeKind::SqOut)"},
         {{"src/core/model.h", "return kind == SqueezeKind::SqIn ? \"SqIn\" : \"SqOut\";",
           "type_name, the owner"}},
         {}},
        {"How is a multiplier squeeze written as text?",
         "MultSqueeze::notationstr in src/core/model.cpp",
         R"(multiplier\(\)\)\s*\+\s*"x")",
         "",
         {},
         {},
         "audit finding 248; phase 6 task J3-1 (D53, D54)",
         {"s += std::to_string(multsqueezes[i].multiplier()) + \"x\";"},
         {"s += multsqueezes[i].notationstr();"},
         {{"src/core/model.cpp", "return std::to_string(multiplier()) + \"x\";",
           "MultSqueeze::notationstr, the owner"}},
         {"src"}},
        // prepare_variants writes the tail; only reading it is the question.
        {"Which activations does a path have, and in what order?",
         "ActivationWalk (Path::walk_activations) in src/core/model.h",
         R"(variant_tail\.(begin|end|empty)\(\))",
         "",
         {},
         {},
         "audit finding 268; phase 6 task J3-1 (D53, D54)",
         {"out.insert(out.end(), variant_tail.begin(), variant_tail.end());",
          "return !activations.empty() || !variant_tail.empty();"},
         {"v.variant_tail.clear();",
          "for (size_t i = vp; i < mine.size(); ++i) v.variant_tail.push_back(mine[i]);"},
         {},
         {"src"}},
        // The engine's running p.score is a different sum, per edge; task
        // J3-2 checks it against score_total at copy-out.
        {"What is a path's total score?",
         "score_total in src/core/model.h",
         R"(score_base\s*\+\s*score_combo\s*\+\s*score_sp|\bbase\s*\+\s*combo\s*\+\s*sp\s*\+\s*solo\b)",
         "",
         {},
         {},
         "audit finding 165; phase 6 task J3-1 (D53, D54)",
         {"return score_base + score_combo + score_sp + score_solo + score_accents +",
          "int64_t total() const { return base + combo + sp + solo + accent + ghost; }"},
         {"return score_base + score_ghosts + score_accents;",
          "p.score += (int64_t)e.basescore + e.comboscore + e.spscore + e.soloscore +"},
         {{"src/core/model.h", "return base + combo + sp + solo + accents + ghosts;",
           "score_total, the owner"}},
         {"src"}},
        // ---- phase 6 task J3-2: the graph, the engine and the replay ----
        // The engine's message "a variant under 2 bars at its fold passed a
        // fill" names the number in words and is not a comparison.
        {"How many SP bars does an activation need?",
         "kSpActivationBars in src/core/timing.h",
         R"(\bsp\s*(<|>=)\s*2\b|\bkept\s*<\s*2\b|for \(int sp = 2\b|sp_cap\s*==\s*1\b)",
         "",
         {},
         {},
         "audit finding 334; D54 (kSpActivationBars, a code-only call); M7-2a review row 3; "
         "phase 6 task J3-2",
         {"if (p.sp < 2 || !has_value(p.sp_ready_ms)) return 0;", "if (kept < 2 && sp >= 2) {",
          "for (int sp = 2; sp <= max_sp_bars(); ++sp) {",
          "const bool one_bar_cap = record.sp_cap && *record.sp_cap == 1;"},
         {"if (p.sp < kSpActivationBars) return false;", "inline constexpr int kSpActivationBars = 2;"},
         {},
         {"src"}},
        // graph_build_cap's floor of one is the pather's own and stays there.
        {"How tall can the SP meter get?",
         "max_sp_bars in src/search/graph.cpp",
         R"(std::min\([^;]*(cap[^;]*phrase|phrase[^;]*cap))",
         "",
         {},
         {},
         "audit finding 260; phase 6 task J3-2 (D53, D54)",
         {"return std::min(*sp_meter_cap_, sp_phrase_count_);",
          "return std::min(sp_cap, std::max(sp_phrase_count, 1));"},
         {"bank = std::min(bank + 1.0, cap);",
          "return std::max(max_sp_bars(sp_cap, sp_phrase_count), 1);"},
         {{"src/search/graph.cpp", "return std::min(*sp_meter_cap, sp_phrase_count);",
           "max_sp_bars, the owner"}},
         {"src"}},
        // Only activation edges carry a deadline; the engine refuses one
        // without it instead of filling in a number.
        {"What does the engine read when an activation edge has no fill deadline?",
         "enumerate and index_fills in src/search/engine.cpp",
         R"(activation_fill_deadline_ms\.value_or\()",
         "",
         {},
         {},
         "audit R7.35; phase 6 task J3-2 (D53, D54)",
         {"v.activation_fill_deadline_ms = o->activation_fill_deadline_ms.value_or(0.0);"},
         {"v.activation_fill_deadline_ms = o->activation_fill_deadline_ms",
          "fills.emplace_back(n.tick, deadline);"},
         {},
         {"src"}},
        // ---- phase 6 task J3-6: store rows and bytes ----
        // A structure blob's head offset typed as a digit in SQL, or read by
        // hand in C++. Only an older file still has the blob (D87); the
        // upgrade reads its fingerprint through the named offsets.
        {"Where do the format and fingerprint sit in a structure blob?",
         "kOldRulesFingerprintOffset, kOldRulesFingerprintBytes and rules_fp_of in "
         "src/store/record_store.cpp",
         R"(substr\(\s*(\w+\.)?structure\s*,\s*\d|read_le\(structure_head)",
         "",
         {},
         {},
         "audit finding 194, folded under D53 (phase 6 task J3-6)",
         {"\") AND substr(structure,1,4) IN (\" + placeholders(kPathFormatStamp.accepted.size()) +",
          "std::string(\"SELECT hyhash, hyversion, bestpath, result_id, substr(structure,1,12), \") +",
          "\"r.hyversion, r.result_id, substr(r.structure,1,12) \"",
          "kPathFormatStamp.is_current(static_cast<uint32_t>(read_le(structure_head, 0, 4)));"},
         {"return head_field_sql(blob, kRulesFingerprintOffset, kRulesFingerprintBytes);"},
         {},
         {}},
        // A hand-written little-endian helper beside the shared codec. The
        // byte-at-a-time shift loop is the form most copies use, whatever the
        // helper is called. A fully unrolled helper with no telling name is
        // not caught. The hash's byte loops store no number, so they are
        // listed as owner lines, not copies (review of J3-6 finding 1). The
        // J2-5 row's question is the same with the u32 helper names; the
        // suffix keeps the two rows' questions apart. Reading moved down to
        // core/ in task J3-9d, because parse/ cannot include store/; the
        // pattern also catches a helper named for its width alone (le32) and
        // the second byte of an unrolled read OR-ed in shifted by 8. A
        // big-endian read (MIDI) and the hash's XOR-ed tail bytes are other
        // questions and are not flagged.
        {"How is a little-endian number written byte by byte? (any width or name)",
         "read_le and append_le in src/core/little_endian.h (rules_fp_bytes and "
         "testbytes::put_le call the writer)",
         R"(\b(uint16_t|uint32_t|uint64_t|size_t|void|std::vector<uint8_t>)\s+(read|write)_(u16_|u32_|u64_)?le\(|<<\s*\(8\s*\*\s*i\)|>>\s*\(8\s*\*\s*i\)|\b\w*le(16|32|64)\s*\(\s*const\s+(uint8_t|unsigned char)\s*\*|\|\s*\(*\s*(static_cast<\w+>|u?int\d*_t)?\s*\(*\s*[\w.>-]+\[[^\]]*\]\s*\)*\s*<<\s*8\b)",
         "",
         {},
         {},
         "audit finding 195, folded under D53 (phase 6 tasks J3-6 and J3-9d); widened by review of J3-6 "
         "finding 1 and by J3-9d; tests scanned since the J3 join; writing folded in the J3 review fix",
         {"uint64_t read_le(const std::vector<uint8_t>& b, size_t at, int bytes) {",
          "std::vector<uint8_t> write_le(uint64_t v, int bytes) {",
          "for (int i = 0; i < 8; ++i) v |= static_cast<uint64_t>(buf[pos + i]) << (8 * i);",
          "for (int i = 0; i < 4; ++i) v |= static_cast<uint32_t>(buf[pos + i]) << (8 * i);",
          "len |= static_cast<uint32_t>(meta[pos + i]) << (8 * i);",
          "for (int i = 0; i < 4; ++i) v |= static_cast<uint32_t>(bytes_[pos_++]) << (8 * i);",
          "uint32_t le32(const uint8_t* p) {", "int64_t le64(const uint8_t* p) {",
          "return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |",
          "link->preskip = pos[10] | (pos[11] << 8);",
          "link->gain_q78 = static_cast<int16_t>(pos[16] | (pos[17] << 8));",
          "skip_remaining = op.packet[10] | (static_cast<int>(op.packet[11]) << 8);",
          "const uint32_t first_four = uint32_t(structure[0]) | uint32_t(structure[1]) << 8 |",
          "for (int i = 0; i < bytes; ++i) out.push_back(static_cast<uint8_t>(v >> (8 * i)));"},
         {"void BinaryWriter::u32(uint32_t v) {",
          "out.push_back(static_cast<uint8_t>(v >> 8));",
          "const uint32_t len = core::read_le_u32(meta.data() + pos);",
          "uint64_t blob_size = core::read_le_u64(buf.data() + cursor + 16);",
          "constexpr uint32_t read_le_u32(const uint8_t* p) { return read_le<uint32_t>(p); }",
          "link->gain_q78 = static_cast<int16_t>(core::read_le_u16(pos + 16));",
          "(uint32_t(d[at + 2]) << 8) | uint32_t(d[at + 3]);",
          "return int16_t((uint16_t(d[at]) << 8) | uint16_t(d[at + 1]));",
          "case 10: k2 ^= static_cast<uint64_t>(tail[9]) << 8;    [[fallthrough]];",
          "link->channels = pos[9];",
          "void BinaryWriter::u32(uint32_t v) { core::append_le_u32(bytes, v); }",
          "void BinaryWriter::u64(uint64_t v) { core::append_le_u64(bytes, v); }",
          "hydra::core::append_le(out, v, bytes);"},
         {{"src/core/little_endian.h",
           "for (std::size_t i = 0; i < sizeof(T); ++i) v = static_cast<T>(v | (static_cast<T>(p[i]) << (8 * i)));",
           "read_le, the owner of reading"},
          {"src/core/little_endian.h",
           "for (int i = 0; i < bytes; ++i) out.push_back(static_cast<uint8_t>(v >> (8 * i)));",
           "append_le, the owner of writing"}},
         {"src", "tests"}},
        // A second read of a row's head to say why it is Stale.
        {"Why is a stored row Stale?",
         "rank_row in src/store/record_store.cpp",
         R"(stale_reasons\(|layout_is_current\()",
         "",
         {},
         {},
         "audit finding 196, folded under D53 (phase 6 task J3-6)",
         {"stale_reasons(best.hyversion, best.structure, rules_fingerprint_);"},
         {"const Candidate& rank = picker.rank(*won);"},
         {},
         {"src"}},
        // One identity column compared at a time instead of a RecordKey.
        {"Is this still the row the report walk listed?",
         "RecordKey::operator== in src/store/record_store.h",
         R"(!=\s*meta\.(hyhash|chartmode|sp_cap)\b)",
         "",
         {},
         {},
         "audit finding 197, folded under D53 (phase 6 task J3-6)",
         {"if (column_text(stmt, 0) != meta.hyhash) return false;"},
         {"if (column_text(stmt, 2) != meta.hyversion) return false;"},
         {},
         {"src"}},
        {"Which PathSummary fields must match? (tests)",
         "PathSummary::operator== in src/store/record_store.h",
         R"(\bdiff_summary\s*\()",
         "",
         {},
         {},
         "audit finding 121, folded under D53 (phase 6 task J3-6)",
         {"std::string diff_summary(const PathSummary& a, const PathSummary& b) {"},
         {"CHECK(summarize_record(back) == summarize_record(rec));"},
         {},
         {"tests"}},
        // Schema 2's column list typed out again.
        {"Which columns did schema 2 have? (tests)",
         "kSchema2ResultsColumns in src/store/record_store.cpp",
         R"(result_id, hyhash, chartmode, hyversion,)",
         "",
         {},
         {},
         "audit finding 267, folded under D53 (phase 6 task J3-6)",
         {"\"INSERT INTO results SELECT result_id, hyhash, chartmode, hyversion,\""},
         {"std::string(\"INSERT INTO results (\") + kSchema2ResultsColumns +"},
         {{"src/store/record_store.cpp",
           "\"result_id, hyhash, chartmode, hyversion, sp_cap, ms_enabled, ms_value, depth_mode,\"",
           "kSchema2ResultsColumns, the owner"}},
         {"src", "tests"}},
        // Summing each root's tied count instead of counting all_paths().
        {"How many paths did a record keep?",
         "HydraRecord::all_paths in src/core/model.h",
         R"(\+=\s*\w+\.tied_pathcount\(\))",
         "",
         {},
         {},
         "audit finding 172, folded under D53 (phase 6 task J3-6)",
         {"for (const Path& p : record.paths) pathcount += p.tied_pathcount();"},
         {"tied_count += v.tied_count;"},
         {},
         {}},
        {"When is the song's last note? (tests)",
         "last_drawn_note in src/app/preview_view.cpp, and last_note_ms for its onset",
         R"(sequence\.back\(\)\.timecode\.ms\(\))",
         "",
         {},
         {},
         "audit finding 191, the store test's compare (phase 6 task J3-6); re-pointed by D69 "
         "(phase 7 task AL2)",
         {"const double expected = song.sequence.back().timecode.ms();"},
         {"const double expected = hydra::app::last_note_ms(scene);",
          "scene.song_length_ms = song_length_ms.value_or(0.0);"},
         {},
         {"tests"}},
        // ---- the J3 join's leftovers (task J3-9) ----
        // The J3-1 offset row above catches the value_or fallback; this one
        // catches a bare dereference of the same optional. The star must
        // follow an opening, an operator or return, so a product with a local
        // named offset_ms is not flagged. Tests pin stored offsets and stay
        // out of scope.
        {"What is a backend row's offset? (a bare dereference)",
         "BackendSqueeze::offset in src/core/model.cpp",
         R"((^|[(,=!&|?:{]|\breturn)\s*\*\s*(\w+(\.|->)|\w+\(\)(\.|->))*offset_ms\b)",
         "",
         {},
         {},
         "audit finding 346; the J3 join (task J3-9)",
         {"!core::counted_without_squeeze(*row.row.offset_ms, backend_leeway_ms));",
          "const double o = *bsq.offset_ms;",
          "const double stored_ms = *act.sqout_row()->offset_ms;"},
         {"const double o = bsq.offset();", "*w.sqout_offset_ms);",
          "const double t = scale * offset_ms;"},
         {{"src/core/model.cpp", "return *offset_ms;", "BackendSqueeze::offset, the owner"}},
         {}},
        // ---- phase 6 tasks J3-3, J3-4 and J3-5 ----
        // A line that asks sqout_position for the position, as
        // Activation::is_sqout_backend does, passes.
        {"Which position does a backend row take when the Paths tab prices it?",
         "core::sqout_position in src/core/backend_value.h",
         R"(SqOutPosition::(Exact|NoSqOut)\b)",
         R"(\bsqout_position\()",
         {"src/core/backend_value.h"},
         {},
         "audit findings 150 and 346; phase 6 task J3-3 (D53, D54)",
         {"br.squeezed_out ? core::SqOutPosition::Exact",
          "off, row.points, row.sqout_points, core::SqOutPosition::Exact, leeway_ms);"},
         {"core::sqout_position(bsq.timecode.ticks(), act.sqout_tick),",
          "return core::sqout_position(bsq.timecode.ticks(), sqout_tick) == core::SqOutPosition::Exact;"},
         {},
         {"src"}},
        {"How is a measure number written as a label?",
         "measure_label in src/app/path_view.cpp",
         R"("m" \+ std::to_string|TextDisabled\("m1"\)|measure_beats_ticks\(\)\[0\] \+ 1)",
         "",
         {},
         {{"src/parse/song.cpp",
           "the parser steps its measure map by measure numbers there; it writes no label"}},
         "audit findings 183 and 184; phase 6 task J3-3 (D53, D54)",
         {R"x("m" + std::to_string((long long)timing->timecode(end_tick).measure_beats_ticks()[0] + 1);)x",
          R"x(ImGui::TextDisabled("m1");)x"},
         {"view.timeline_end = measure_label(timing->timecode(end_tick));",
          R"x(ImGui::TextDisabled("%s", view.timeline_start.c_str());)x"},
         {{"src/app/path_view.cpp",
           R"x(return "m" + std::to_string((long long)tc.measure_beats_ticks()[0] + 1);)x",
           "measure_label, the owner"}},
         {"src"}},
        // Tests pin the shown text and may type it.
        {"How is the first measure spelled?",
         "first_measure_label in src/app/path_view.cpp",
         R"("m1\.1\.0")",
         "",
         {},
         {},
         "audit R7.27; phase 6 task J3-3 (D53, D54)",
         {R"x(box.position = scene.timing ? format_measure(*scene.timing, now_tick) : "m1.1.0";)x"},
         {"std::string first_measure_label() { return format_measure(Timecode{}); }"},
         {},
         {"src"}},
        {"What id does an activation's backend table get?",
         "backend_table_id in src/app/path_view.cpp",
         R"(##backends)",
         "",
         {},
         {},
         "audit finding 291; phase 6 task J3-3 (D53, D54)",
         {R"x(std::snprintf(id, sizeof(id), "##backends%d_%d_%d_%d", a.number, static_cast<int>(w_timing),)x",
          R"x(const std::string prefix = "##backends" + std::to_string(number) + "_";)x"},
         {"app::backend_table_id(a.number, static_cast<int>(w_timing), static_cast<int>(w_chord),"},
         {{"src/app/path_view.cpp",
           R"x(std::snprintf(id, sizeof(id), "##backends%d_%d_%d_%d", number, w_timing, w_chord, w_points);)x",
           "backend_table_id, the owner"},
          {"tests/test_path_view.cpp",
           R"x(CHECK(backend_table_id(1, 48, 40, 56) == "##backends1_48_40_56");)x",
           "the owner's pinned case: finding 291's own example as a literal (D42)"}},
         {"src", "tests"}},
        // A letter before the figure: a badge's wording, not a decimal such
        // as "2.999 ms" in a leeway case.
        {"What is the longest badge an activation row can show?",
         "longest_activation_badge in src/app/path_view.cpp",
         R"([a-z] 999 ms|text\(18\))",
         "",
         {},
         {},
         "audit finding 292; phase 6 task J3-3 (D53, D54)",
         {R"x(check("m1024.1.120", "m1024.1.120", "12 bars", "early fill 999 ms");)x",
          R"x(check("m1024.1.120", "m1024.1.120", "12 bars", "squeeze out 999 ms");)x",
          R"x(const float badge_w = text(18);  // "squeeze out 999 ms", the longest wording)x"},
         {R"x(TEST_CASE("backend leeway: +2.999 ms is counted, exactly +3.0 ms is not (D29)") {)x",
          "const float badge_w = text(static_cast<int>(hydra::app::longest_activation_badge().size()));"},
         {},
         {"tests"}},
        {"Where do the measure and the bars start on an activation row?",
         "kRowMeasureX and kRowMinBarsX in src/ui/activation_row_layout.h",
         R"(\b(104|200)\.0f\b)",
         "",
         {},
         {{"tests/test_overlay_layout.cpp",
           "the Preview overlay's own test widths, not an activation row"}},
         "audit finding 276; phase 6 task J3-3 (D53, D54)",
         {"CHECK(l.measure_x == doctest::Approx(104.0f));"},
         {"CHECK(l.measure_x == doctest::Approx(kRowMeasureX));"},
         {{"src/ui/activation_row_layout.h",
           "inline constexpr float kRowMeasureX = 104.0f;   // where the measure starts",
           "kRowMeasureX, the owner"},
          {"src/ui/activation_row_layout.h",
           "inline constexpr float kRowMinBarsX = 200.0f;   // the bars never start before this",
           "kRowMinBarsX, the owner"}},
         {"src", "tests"}},
        {"Which moment do the Preview's overlay boxes read?",
         "shown_ms in src/app/preview_view.cpp",
         R"(now_ms < 0\.0 \? 0\.0 : now_ms|now_ms > len \? len)",
         "",
         {},
         {},
         "audit finding 185; phase 6 task J3-4 (D53, D54)",
         {"const double now = now_ms < 0.0 ? 0.0 : now_ms;"},
         {"const double now = shown_ms(now_ms, length_ms);"},
         {{"src/app/preview_view.cpp",
           "return now_ms < 0.0 ? 0.0 : (now_ms > len ? len : now_ms);", "shown_ms, the owner"}},
         {"src"}},
        // Tests pin the fact that a scene has no curve; those are assertions,
        // not a predicate.
        {"Does the Preview scene have an SP gauge?",
         "PreviewScene::has_sp_gauge in src/app/preview_view.h",
         R"(sp_meter\.segments\.empty\(\))",
         "",
         {},
         {},
         "audit finding 216; phase 6 task J3-4 (D53, D54)",
         {"if (!scene.timing || scene.sp_meter.segments.empty()) return box;",
          "return !scene_.sp_meter.segments.empty();"},
         {"if (!scene.has_sp_gauge()) return box;"},
         {{"src/app/preview_view.h",
           "bool has_sp_gauge() const { return timing.has_value() && !sp_meter.segments.empty(); }",
           "has_sp_gauge, the owner"}},
         {"src"}},
        {"Over which stretch is an activation's Star Power running?",
         "PreviewActivation::sp_window in src/app/preview_view.h",
         R"(has_sp_end && (struck_at\(now, a\.ms\)|a\.sp_end_ms > a\.ms))",
         "",
         {},
         {},
         "audit finding 157; phase 6 task J3-4 (D53, D54)",
         {"if (a.has_sp_end && struck_at(now, a.ms) && now < a.sp_end_ms) return &a;",
          "if (a.has_sp_end && a.sp_end_ms > a.ms)"},
         {"if (w && struck_at(now, w->first) && now < w->second) return &a;",
          "if (const std::optional<std::pair<double, double>> w = a.sp_window())"},
         {},
         {"src"}},
        // The owner is apply_preview_overlay's mark, which stores the fill on
        // the activation (PreviewActivation::taken_fill); readers take it from
        // there instead of searching the fills again.
        {"Which fill did an activation take?",
         "apply_preview_overlay's mark in src/app/preview_view.cpp, stored as "
         "PreviewActivation::taken_fill",
         R"(span\.end_tick\s*[!=]=\s*a\.tick)",
         "",
         {},
         {},
         "audit finding 175; phase 6 task J3-4 (D53, D54)",
         {"if (f.state != app::PreviewFillState::Taken || f.span.end_tick != a.tick) continue;"},
         {"if (next_fill >= fills.size() || fills[next_fill].span.end_tick != tick)"},
         {},
         {"src"}},
        // J2-3's row covers the replay's read; this one covers the Preview
        // scene's own join of flagged chords.
        {"Where does a solo section start and end? (the scene's join)",
         "Song::solo_sections in src/parse/song.h",
         R"(\b(in_solo|solo_start|solo_last)\b)",
         "",
         // The parsers read solo markers and build the sections there.
         {"src/parse/song.cpp"},
         {},
         "audit finding 167; phase 6 task J3-4 (D53, D54)",
         {"solo_start = ts.timecode.ticks();",
          "scene.solos.push_back(span_from_ticks(song, solo_start, solo_last));"},
         {"scene.solos.push_back(span_from_ticks(song, song.sequence[s.first].timecode.ticks(),"},
         {},
         {"src"}},
        {"Which gem does a Preview note draw as?",
         "pad_of in src/render/track_state.cpp",
         R"(n\.lane == PreviewLane::Kick)",
         "",
         {},
         {},
         "audit finding 190; phase 6 task J3-4 (D53, D54)",
         {"if (n.lane == PreviewLane::Kick) {"},
         {"case PreviewLane::Kick:   break;",
          "CHECK(scene.notes[3].lane == PreviewLane::Kick);"},
         {},
         {"src"}},
        // A jump through the controller pointer with a number typed in, a
        // quoted control word naming the step, or a second definition of
        // either constant. The tests drive the controller with typed
        // distances (pc.jump_ms(5000.0)) and pin the shown text, so src only.
        {"How far does a Preview jump move, and what do its controls call it?",
         "kJumpSeconds and kTickStep in src/ui/preview_tab.cpp",
         R"(->jump_ms\(\s*-?\d|"[^"]*\b(5 seconds|5 ticks|5 Ticks)\b[^"]*"|"[-+]5s"|\b(kJumpSeconds|kTickStep)\s*=\s*\d)",
         "",
         {},
         {},
         "audit finding 215; phase 6 task J3-5 (D53, D54)",
         {R"(if (ImGui::Button("-5s")) pc->jump_ms(-5000.0);)", R"({{",", "."}, "5 ticks"},)"},
         {"pc.jump_ms(5000.0);", "inline constexpr double kAudioTailMs = 5000.0;"},
         {{"src/ui/preview_tab.cpp", "constexpr int kJumpSeconds = 5;", "kJumpSeconds, the owner"},
          {"src/ui/preview_tab.cpp", "constexpr int kTickStep = 5;", "kTickStep, the owner"}},
         {"src"}},
        // highway_draw.cpp and overlay_layout.cpp have their own factors
        // under J2-8's rows, so only the tab is scanned.
        {"How big is each Preview overlay box at a scale?",
         "overlay_box_sizes and line_height in src/ui/preview_tab.cpp",
         R"(\* 1\.25f|\* 1\.8f|\* 1\.2f)",
         "",
         {},
         {},
         "audit finding R7.18; phase 6 task J3-5 (D53, D54)",
         {"const float line_h = size * 1.25f;",
          "const float score_size = score.available ? size * 1.8f : size;"},
         {"const float line_h = line_height(size);", "s.pad = px(8.0f) * scale;"},
         {{"src/ui/preview_tab.cpp", "float line_height(float size) { return size * 1.25f; }",
           "line_height, the owner"},
          {"src/ui/preview_tab.cpp", "s.score_size = score.available ? s.size * 1.8f : s.size;",
           "overlay_box_sizes, the owner"},
          {"src/ui/preview_tab.cpp", "s.score_line_h = s.score_size * 1.2f;",
           "overlay_box_sizes, the owner"}},
         {"src/ui/preview_tab.cpp"}},
        // A prefix test on a drawn overlay key, or a read of the key's path
        // part (overlay_key_path_part) anywhere but shows_path. Tests may
        // still compare two keys whole.
        {"Is the drawn overlay the selected path's?",
         "PreviewController::shows_path in src/ui/preview_controller.cpp",
         R"(overlay_path_key\(\)\.rfind\(|rfind\(\w*overlay_key\w*, 0\)|\boverlay_key_path_part\()",
         "",
         {},
         {},
         "audit finding 340; phase 6 task J3-5 (D53, D54)",
         {"if (jump && pc->overlay_path_key().rfind(ui.overlay_key, 0) == 0) {",
          "return h.app->preview->overlay_path_key().rfind(first_key, 0) == 0;"},
         {"if (jump && pc->shows_path(ui.overlay_key)) {",
          "IM_CHECK(h.app->preview->overlay_path_key() != first_overlay);"},
         {{"src/ui/preview_controller.cpp",
           "std::string overlay_key_path_part(const std::string& key) {",
           "the one reader of an overlay key's path part, beside overlay_key"},
          {"src/ui/preview_controller.cpp",
           "return !scene_path_key_.empty() && overlay_key_path_part(scene_path_key_) == path_key;",
           "shows_path, the owner"}},
         {"src", "tests"}},
        // Every test builds its options through track_options too (the J3
        // join folded the golden and renderer tests), so all of tests is
        // scanned.
        {"Which highway options does the Preview draw with?",
         "track_options in src/ui/preview_load_job.h",
         R"(\.pro\s*=\s*pro_?\b|\.pro\s*[!=]=\s*\w+\.pro\b)",
         "",
         {},
         {},
         "audit finding R7.16; phase 6 task J3-5 (D53, D54); widened to all tests by the J3 join",
         {"track_opts.pro = pro_;", "if (pending_track_ && pending_track_opts_.pro == opts.pro)"},
         {"plain.pro = false;", "if (pending_track_ && pending_track_opts_ == opts)"},
         {{"src/ui/preview_load_job.h", "opts.pro = pro;", "track_options, the owner"},
          {"src/render/track_state.h", "return a.pro == b.pro;",
           "TrackStateOptions::operator==, the struct's own compare"}},
         {"src", "tests"}},
        // A stem's size read from its path on disk: the load maps every loose
        // stem first and takes the mapped size.
        {"How big is a Preview stem?",
         "MappedFile::size in src/audio/mapped_file.h",
         R"(\bfile_size_bytes\(\s*\w+(\.|->)path\b)",
         "",
         {},
         {},
         "audit finding R7.25; phase 6 task J3-5 (D53, D54)",
         {"n = file_size_bytes(s.path);"},
         {"REQUIRE(hydra::file_size_bytes(song) >= 300000000ull);"},
         {},
         {"src"}},
        // ---- a song's length comes from its metadata, never its audio (D75) ----
        // The owner (app::song_length_ms) opens no audio. A song's stems
        // mixed, or the audio's end asked of a mix, anywhere but the Preview's
        // own mix step is a length worked out from audio again. The transport
        // asks the audio's end of its playhead for the playback range (D48),
        // which is not a song length; that line is allowed.
        {"How long is this song?",
         "song_length_ms and chart_song_length_ms in src/app/song_length.cpp; the only mix is "
         "mix_song_stems in src/audio/song_audio.cpp, for the Preview's playback",
         R"(make_(unique|shared)<\s*(\w+::)*StreamMix\s*>|\b(\w+::)*StreamMix\s+\w+\s*[({]|\baudio_end_chart_ms\s*\((?!\s*const\b))",
         "",
         {"src/audio/song_audio.cpp"},
         {},
         "D75 (a song's length comes from its chart metadata); phase 7 task AL1, task SL1",
         {"auto mix = std::make_unique<audio::StreamMix>(std::move(readers), kOutRate, kOutChannels,",
          "auto mix = std::make_unique<hydra::audio::StreamMix>(std::move(readers), kOutRate, kOutChannels,",
          "const std::optional<double> audio_end_ms = audio_end_chart_ms(*mix, offset_ms);",
          "audio::StreamMix mix(std::move(readers), 48000, 2, 0);"},
         {"audio::SongMix song_mix = audio::mix_song_stems(std::move(readers), ps.audio_offset_ms);",
          "std::optional<double> audio_end_chart_ms(const Audio& audio, double audio_offset_ms) {"},
         {{"src/ui/preview_transport.cpp",
           "playhead_ ? audio_end_chart_ms(*playhead_, audio_offset_ms_) : std::nullopt;",
           "PreviewTransport::load's playback range (D48), not a song length"}},
         {"src"}},
        // The owner's answer turned into a stored length anywhere but the one
        // helper an analysis saves through, so no third try/catch decides
        // what a failed read leaves.
        {"What does a failed length read leave in the store?",
         "analysis_song_length in src/app/analysis.cpp",
         R"(\bSongLength::found\s*\()",
         "",
         {},
         {},
         "derive-once review of AL2, finding 1 (D69); D75, task SL1",
         {"return store::SongLength::found(",
          "length_ = store::SongLength::found(app::chart_song_length_ms(",
          "wr.length = SongLength::found(len);"},
         {"static SongLength found(std::optional<double> ms) { return SongLength{true, ms}; }",
          "store::SongLength analysis_song_length(const std::optional<store::ChartTimingMeta>& scanned,",
          "wr.length = analysis_song_length(item.timing, item.notespath, ar.song, settings);"},
         {{"src/app/analysis.cpp", "return store::SongLength::found(",
           "analysis_song_length, the owner"}}},
        // Production's own temp folder for the shell (copy_to_short_temp in
        // src/app/report_files.cpp) answers a different question (audit
        // R7.12), so only tests/ is scanned. The temp_util case in
        // test_winstr.cpp reads the temp folder once as its input.
        {"How does a test build a per-process scratch path?",
         "temp_path and temp_dir in tests/temp_util.h",
         R"(\b(GetTempPath\w*|GetTempFileName\w*|temp_directory_path)\()",
         "",
         {},
         {},
         "audit finding 287; phase 6 tasks J4-6 and J4-7 (D53, D54)",
         {"GetTempPathW(MAX_PATH, tmp);", "DWORD n = GetTempPathW(MAX_PATH, buf);",
          "GetTempPathA(MAX_PATH, dir);", "GetTempFileNameA(dir, \"hyd\", 0, buf);",
          "dir = fs::temp_directory_path() /", "const fs::path dir = fs::temp_directory_path() /",
          "std::filesystem::temp_directory_path() / \"hydra_app_shell_test\";",
          "CHECK(app::load_rules_file(std::filesystem::temp_directory_path() /"},
         {"const std::string path = testtemp::temp_path(\"report_file\", \".html\");",
          "root = hydra::os_path(testtemp::temp_dir(std::string(\"docs_\") + tag));",
          "dir = hydra::os_path(testtemp::temp_dir(std::string(\"cli_\") + name));",
          "std::string path = testtemp::temp_path(\"dynamics_store\", \".db\");"},
         {{"tests/temp_util.h", "GetTempPathW(MAX_PATH, tmp);", "temp_path, the owner"},
          {"tests/test_winstr.cpp", "GetTempPathW(MAX_PATH, tmp);",
           "the temp_util case reads the temp folder once, the input its pins start "
           "from"}},
         {"tests"}},
        // The owner lives in src, so no line under tests/ is an owner line.
        {"How is wide text converted to UTF-8 in a test?",
         "wide_to_utf8 in src/core/winstr.cpp",
         R"(\bWideCharToMultiByte\()",
         "",
         {},
         {},
         "audit finding 287 (test_srb.cpp's hand conversion); phase 6 task J4-6 (D53, D54)",
         {"int len = WideCharToMultiByte(CP_UTF8, 0, d.c_str(), -1, nullptr, 0,",
          "WideCharToMultiByte(CP_UTF8, 0, d.c_str(), -1, &out[0], len, nullptr,"},
         {"return hydra::wide_to_utf8(dir.wstring());",
          "MultiByteToWideChar(CP_UTF8, 0, s.data(), n, out.data(), len);"},
         {},
         {"tests"}},
        // ---- phase 6 task J4-5: unit tests read production constants ----
        // A test that types the 1.1 deadline's tick math again instead of
        // pinning the ms from one run.
        {"By what millisecond must a 1.1 fill spawn? (tests)",
         "activation_fill_deadline_ms in src/search/graph.cpp",
         R"(fill_length - 4 \* res)",
         "",
         {},
         {},
         "audit finding 263; phase 6 task J4-5 (D53, D54)",
         {"const int64_t tick_e = fill_end - fill_length - 4 * res;"},
         {"CHECK(ch11(t, fill_end, fill_lengths[f]) == doctest::Approx(want[r][b][f]));"},
         {},
         {"tests"}},
        // A test that adds one to a combo to find a note's multiplier, where
        // category_scores already stamps it.
        {"What multiplier does each note in a chord score at? (tests)",
         "category_scores in src/core/scoring.cpp",
         R"(to_multiplier\([\w.]+ \+ 1)",
         "",
         {},
         {},
         "audit finding 266; phase 6 task J4-5 (D53, D54)",
         {"const int first_cut = notes[0].basescore() * to_multiplier(combo + 1);",
          "notes[i].basescore() * to_multiplier(combo + 1 + static_cast<int>(i));",
          "CHECK(c.multiplier == to_multiplier(c.combo_before + 1));"},
         {"CHECK(c.multiplier_after == to_multiplier(r.chords[i + 1].combo_before));"},
         {},
         {"tests"}},
        // The highway's half-tick span end and the glow's fade length typed
        // into the draw tests as numbers.
        {"Where does a highway span end, and how fast does a target glow fade? (tests)",
         "kSpanEndTicks in src/render/track_state.h and targets.secs_light in "
         "assets/preview/3d-config.json",
         R"(1\.5005|0\.1666666)",
         "",
         {},
         {},
         "audit finding 276 (the highway half); phase 6 task J4-5 (D53, D54)",
         {"CHECK(c.hi[2] == doctest::Approx(time_to_z(cfg, now, 1.5005, 1.0)));",
          "CHECK(c.alpha == doctest::Approx(1.0f - 0.05f / 0.1666666f));",
          "CHECK(c.alpha == doctest::Approx(1.0f - 0.15f / 0.1666666f));",
          "CHECK(c.hi[2] == doctest::Approx(time_to_z(cfg, 1.0, 1.5005, 1.0)));"},
         {"doctest::Approx(time_to_z(cfg, now, 1.5 + kSpanEndTicks / 1000.0, 1.0)));"},
         {{"tests/test_preview_config.cpp",
           "CHECK(c.track.targets_secs_light == doctest::Approx(0.1666666));",
           "the config test pins the shipped json's own value (D54, item 220)"}},
         {"tests"}},
        // A test that takes the solo bonus back off the running total itself.
        {"What running total does the score box show during a solo? (tests)",
         "replay_path's cum_onscreen_total in src/core/replay.cpp",
         R"(\.cum\.total\(\)\s*-\s*)",
         "",
         {},
         {},
         "audit finding 281; phase 6 task J4-5 (D53, D54)",
         {"CHECK(scene.score.steps[1].total == r.chords[1].cum.total() - "
          "r.chords[1].points.solo);"},
         {"CHECK(c.cum_onscreen_total <= c.cum.total());"},
         {},
         {"tests"}},
        // Copying a ReplayScore into a Path field by field anywhere but
        // assign_score.
        {"Which Path field holds which ReplayScore category?",
         "score_of and assign_score in src/core/replay.cpp",
         R"(score_ghosts\s*=\s*\w+\.ghost)",
         "",
         {},
         {},
         "audit finding 282; phase 6 task J4-5 (D53, D54)",
         {"p.score_ghosts = s.ghost;"},
         {"s.ghost = path.score_ghosts;"},
         {{"src/core/replay.cpp", "path.score_ghosts = s.ghost;", "assign_score, the owner"}},
         {"src", "tools", "tests"}},
        // A field written beside PreviewScene::timing that holds its
        // resolution a second time.
        {"How many ticks per quarter note does the Preview scene have?",
         "PreviewScene::timing in src/app/preview_view.h",
         R"(\.tick_resolution\s*=\s*)",
         "",
         {},
         {},
         "audit finding 135; phase 6 task J4-5 (D53, D54)",
         {"scene.tick_resolution = timing.tick_resolution();", "s.tick_resolution = 1000;"},
         {"CHECK(scene.timing->tick_resolution() == 480);"},
         {},
         {"src", "tools", "tests"}},
        // A button's width is its label's plus the frame padding each side.
        // An input with step buttons adds the buttons too, a different
        // question (the Score range box measures its sample on its own line).
        {"How wide is a button?",
         "button_slot_width in src/ui/widgets.h",
         R"(CalcTextSize\(.*FramePadding\.x \* 2)",
         "",
         {"src/ui/widgets.h"},
         {},
         "audit finding 213; phase 6 task J4-3 (D53, D54)",
         {"return ImGui::CalcTextSize(label, nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2.0f;",
          "ImGui::CalcTextSize(label.c_str(), nullptr, true).x + style.FramePadding.x * 2.0f;",
          "const float clear_w = ImGui::CalcTextSize(\"X\").x + style.FramePadding.x * 2.0f;"},
         {"const float six_digits = ImGui::CalcTextSize(widest_digits(6).c_str()).x +",
          "const float clear_w = button_slot_width(\"X##clearsearch\");"},
         {},
         {"src", "tools", "tests"}},
        // Placing an item after the last one and comparing with a right edge.
        {"Does the next item fit on this line?",
         "fits_in_row and fits_on_line in src/ui/widgets.h",
         R"(GetItemRectMax\(\)\.x \+ \w+(\.ItemSpacing\.x)?\s*\+|line_end \+ .*<= right_edge|\w+ \+ \w+_w > \w*width\b)",
         "",
         {"src/ui/widgets.h"},
         {},
         "audit finding 214; phase 6 task J4-3 (D53, D54); the key bar's form added by task J4-4",
         {"if (ImGui::GetItemRectMax().x + style.ItemSpacing.x + w <= right_edge)",
          "if (line_end + ImGui::GetStyle().ItemSpacing.x + w <= right_edge) ImGui::SameLine();",
          "if (line_end + separator_w + block_w[i] <= right_edge) {",
          "if (x > 0.0f && x + group_w > width) {"},
         {"if (fits_in_row(line_end + separator_w, block_w[i], right_edge)) {",
          "if (fits_on_line(w, ImGui::GetStyle().ItemSpacing.x)) ImGui::SameLine();",
          "if (x > 0.0f && !fits_in_row(x, group_w, width)) {"},
         {},
         {"src"}},
        // A quoted run of six 0s or 9s is a hand-picked "widest six digits".
        // The GUI test's pin is the one measured answer (D42).
        {"How wide can six digits get?",
         "widest_digits in src/ui/widgets.h",
         R"re("(0{6}|9{6})")re",
         "",
         {},
         {},
         "audit finding 112; phase 6 task J4-3 (D53 item 2, D54)",
         {"const float six_digits = ImGui::CalcTextSize(\"000000\").x + style.FramePadding.x * 2.0f +",
          "ImGui::CalcTextSize(\"999999\").x + ImGui::GetStyle().FramePadding.x * 2.0f;"},
         {"h.app->settings.depth_value = 999999;",
          ": \"00000000000000000000000000000000\";"},
         {{"tests/ui/uitest_library.cpp", "IM_CHECK_STR_EQ(widest.c_str(), \"000000\");",
           "the pin of one measured run of widest_digits(6) (D42)"}},
         {"src", "tools", "tests"}},
        // The library's floor beside the panel, or the panel's floor taken
        // off the room. The Paths tests' check that the panel sits at its own
        // minimum is a different question.
        {"How wide is the library with the song panel open?",
         "library_split_width and library_split_bounds in src/ui/library_view.cpp",
         R"(px\(32[01]\.0f\)|room - min_panel|((^|[^.>\w])\w+|\)(\.x)?)\s*-\s*(\w+::)*px\((\w+::)*kMinSongPanelW)",
         "",
         {},
         {},
         "audit finding 120; phase 6 task J4-3 (D53, D54)",
         {"const float min_library = px(320.0f);",
          "const float max_library = std::max(min_library, avail_w - px(kMinSongPanelW));",
          "const float opened = (std::min)(hydra::ui::kDefaultLibraryShare * room, room - min_panel);",
          "IM_CHECK_FLOAT_NEAR_EQ(library()->Size.x, (std::max)(hydra::ui::px(320.0f), 0.3f * room), 1.0f);",
          "IM_CHECK_LE(lib->Size.x, hydra::ui::px(321.0f));",
          "const float want = room - hydra::ui::px(hydra::ui::kMinSongPanelW);",
          "const float w = ImGui::GetContentRegionAvail().x - hydra::ui::px(hydra::ui::kMinSongPanelW);"},
         {"IM_CHECK_RETV(std::fabs(panel->Size.x - hydra::ui::px(hydra::ui::kMinSongPanelW)) <= 1.0f,",
          "IM_CHECK_LE(lib->Size.x, hydra::ui::px(hydra::ui::kMinLibraryW) + 1.0f);"},
         {{"src/ui/library_view.cpp", "return {min_w, std::max(min_w, room - px(kMinSongPanelW))};",
           "library_split_bounds, the owner"}},
         {"src", "tests"}},
        // The GUI tests put the cap back where the Compare button accepts it.
        // Other caps they set are deliberate what-ifs, and the unit tests'
        // engine settings are a different question.
        {"Which SP cap does the leaderboard comparison need? (tests)",
         "kCloneHeroSpCap in src/core/model.h",
         R"(sp_cap = 4;)",
         "",
         {},
         {},
         "audit finding 296; phase 6 task J4-3 (D53, D54)",
         {"h.app->settings.sp_cap = 4;"},
         {"h.app->settings.sp_cap = 8;", "h.app->settings.sp_cap = 6;",
          "h.app->settings.sp_cap = hydra::kCloneHeroSpCap;"},
         {},
         {"tests/ui/uitest_batch_reports.cpp", "tests/ui/uitest_details.cpp",
          "tests/ui/uitest_harness.cpp", "tests/ui/uitest_harness.h",
          "tests/ui/uitest_library.cpp", "tests/ui/uitest_main.cpp",
          "tests/ui/uitest_paths.cpp", "tests/ui/uitest_preview.cpp",
          "tests/ui/uitest_script.cpp", "tests/ui/uitest_tests.cpp"}},
        {"Which column is Best path, and how many columns are there?",
         "kColumnTitle to kColumnBestPath and kLibraryColumnCount in src/ui/library_model.h",
         R"(best = 4;|BeginTable\("##librarytable", 5|constexpr int kColumn\w+ = \d)",
         "",
         {"src/ui/library_model.h"},
         {},
         "audit finding 297; phase 6 task J4-3 (D53, D54)",
         {"const int best = 4;  // Best path's column index",
          "if (!ImGui::BeginTable(\"##librarytable\", 5, flags, size)) return used;",
          "constexpr int kColumnBestPath = 4;"},
         {"const int best = hydra::ui::kColumnBestPath;",
          "if (!ImGui::BeginTable(\"##librarytable\", kLibraryColumnCount, flags, size)) return used;"},
         {},
         {"src", "tests"}},
        // A struct default, not a test input: the leading int keeps lines that
        // set a depth on a built options struct out.
        {"What is the default search depth?",
         "kDefaultDepthValue in src/search/pather.h",
         R"(\bint\s+(depth_value|kDefaultDepthValue)\s*=\s*4\s*;)",
         "",
         {},
         {},
         "audit finding 181; phase 6 task J4-1 (D53, D54)",
         {"int depth_value = 4;"},
         {"int depth_value = kDefaultDepthValue;", "cfg.depth_value = 4;"},
         {{"src/search/pather.h", "constexpr int kDefaultDepthValue = 4;",
           "kDefaultDepthValue, the owner"}}},
        {"Which options make the all-0 search?",
         "allzero_options in src/search/pather.cpp",
         R"(\bhard_ms_filter\s*=\s*true)",
         "",
         {},
         {},
         "audit finding 284; phase 6 task J4-1 (D53, D54); D85",
         {"options.hard_ms_filter = true;", "allzero.hard_ms_filter = true;"},
         {"const std::vector<Path> z = run_search(graph, allzero_options());",
          "hard_ms_filter_(options.hard_ms_filter),"},
         {{"src/search/pather.cpp", "options.hard_ms_filter = true;",
           "allzero_options, the owner"}},
         {"src", "tools", "tests"}},
        // Any comparison against a ms limit by its name, on either side. The
        // walks (Engine::act_within_limit, Activation::within_ms_limit) call
        // SPSqueeze::within_limit and fill_within_limit, which ask the owner.
        {"Is a timing inside a ms limit?",
         "timing_within_limit in src/core/model.h",
         R"((<=|[^-]>)\s*\*?(\w+(\.|->))*(limit_ms|ms_filter_)\b)"
         R"(|(<=|[^-]>)\s*(\*\s*(\w+(\.|->))*|(\w+(\.|->))+)squeeze_max_ms\b)"
         R"(|\b(limit_ms|ms_filter_|squeeze_max_ms)\s*(>=|<))",
         "",
         {},
         {},
         "derive-once review of D85, finding 1; audit finding 152",
         {"if (sq.difficulty() > limit_ms) return false;", "return !e || *e <= limit_ms;",
          "bool within_ms_limit(double ms) const { return !has_ms_filter_ || ms <= ms_filter_; }",
          "if (limit_ms >= d) return true;",
          "if (q.squeeze_max_ms && facts.hardest_ms && *facts.hardest_ms > *q.squeeze_max_ms)"},
         {"if (!sq.within_limit(ms_filter_)) return false;",
          "!timing_within_limit(*facts.hardest_ms, *q.squeeze_max_ms))",
          "return fill_within_limit(a.e_offset, a.skips, ms_filter_);",
          "if (has_ms_filter_ && !fill_within_limit(e_offset, p.currentskips, ms_filter_)) ++over;"},
         {{"src/core/model.h",
           "inline bool timing_within_limit(double ms, double limit_ms) { return ms <= limit_ms; }",
           "timing_within_limit, the owner"}}},
        // A key streamed field by field. The owner is in src, so no line in
        // scope is an owner line.
        {"Which settings change a stored analysis?",
         "settings_key in src/search/pather.cpp",
         R"(\bs\.legacy_fill_deadline\s*<<|\brules\.fingerprint\(\)\s*<<)",
         "",
         {},
         {},
         "audit finding 275; phase 6 task J4-1 (D53, D54)",
         {"k << s.legacy_fill_deadline << '|' << s.rules.fingerprint() << '|';"},
         {"<< static_cast<int>(settings.difficulty) << '|' << hydra::settings_key(settings);",
          "<< rules.fingerprint();"},
         {},
         {"tests", "tools"}},
        // A key put together from a BatchRun's fields. The owner is in src, so
        // no line in scope is an owner line. run_batch's own key in
        // src/app/analysis.cpp holds only a BatchRun and is out of scope.
        {"Which RecordKey does a tool or test file a chart's result under?",
         "Settings::record_key in src/app/config.cpp",
         R"(RecordKey\{[^}]*run\.chartmode)",
         "",
         {},
         {},
         "audit finding 240; H1 derive-once review finding 1 (2026-10-06)",
         {"store::RecordKey{it.md5, run.chartmode, run.cap_query(), run.lens}, *rec);",
          "store::RecordKey{it.md5, run.chartmode, run.cap_query(), run.lens}, rec);"},
         {"store::PreparedRow row = store::prepare_row(st.record_key(it.md5), *rec);",
          "const store::RecordKey key = settings.record_key(it.md5);"},
         {},
         {"tests", "tools"}},
        // hydra.db stores no path details (D87), so no serializer is left to
        // compare records through; a test that brings one back is a second
        // answer to the hasher's question (R16).
        {"Are two engine results the same answer? (tests)",
         "digest::record_hash in tests/song_digest.h",
         R"(\b(record_bytes|flatten_record|encode_path_node)\s*\()",
         "",
         {},
         {},
         "audit finding 301; phase 6 task J4-1 (D53, D54); storage-T5 (R16)",
         {"CHECK_MESSAGE(record_bytes(tall) == record_bytes(built), \"a song with \"",
          "const HydraRecord back = store::rebuild_record(store::flatten_record(rec));"},
         {"CHECK_MESSAGE(digest::record_hash(tall) == digest::record_hash(built),"},
         {},
         {"tests"}},
        // A page that decides "no chart library" itself could stop where the
        // path report does not, or say it in other words (D89 item 2, D92).
        {"Does this database have a chart library to report on?",
         "report::lacks_chart_library in src/app/report.cpp",
         R"(chart_library_count\(\)\s*==\s*0)",
         "",
         {},
         {},
         "D89 item 2, D92 (task storage-T5)",
         {"} else if (store.chart_library_count() == 0) {",
          "if (old_store.chart_library_count() == 0 || new_store.chart_library_count() == 0)"},
         {"} else if (lacks_chart_library(store)) {",
          "if (report::lacks_chart_library(store))"},
         {{"src/app/report.cpp", "return any_results && store.chart_library_count() == 0;",
           "lacks_chart_library, the owner"}},
         {"src"}},
        // The path report and records_by_hash once each tested "is this chart
        // in the library" against their own map (the naming-copy files and
        // the copies). A page that looks a chart up in a library map itself
        // can disagree with them about which charts the library lists.
        {"Does the library list this chart?",
         "report::library_lists in src/app/report.cpp",
         R"(\blibrary\w*\.(find|end|count|contains)\(|\bfiles\.(find|end|count|contains)\()",
         "",
         {},
         {},
         "D87 item 4, D92 (task som-a)",
         {"if (library.find(hash) == library.end()) continue;",
          "const auto file = files.find(hash);",
          "if (file == files.end()) continue;"},
         {"if (!library_lists(library, hash)) continue;", "slot.file = &files.at(hash);",
          "const auto listed = copies.find(md5);"},
         {{"src/app/report.cpp", "return library.find(hash) != library.end();",
           "library_lists, the owner"}}},
        // The GUI harness writes its ini from scratch_settings(), and the
        // Burnout reference cases run through its to_analysis_settings. Other
        // depths in these files are a case's own input.
        {"Which settings do the GUI tests and the Burnout reference results run under? (tests)",
         "scratch_settings in tests/scratch_settings.h",
         R"re(depth_value\s*=\s*2;|"depth_value=2)re",
         "",
         {"tests/scratch_settings.h"},
         {},
         "audit finding 280; phase 6 task J4-4 (D53, D54)",
         {"f << \"depth_value=2\\n\";  // keep analyses short", "settings.depth_value = 2;"},
         {"settings.depth_value = 10;",
          "const AnalysisSettings settings = scratch_settings().to_analysis_settings();"},
         {},
         {"tests/ui/uitest_harness.cpp", "tests/test_path_view.cpp", "tests/test_stars.cpp"}},
        // The font size ImGui draws text at, for a GUI test that measures
        // text itself.
        {"At what pixel size does ImGui draw text? (tests)",
         "text_width in tests/ui/uitest_harness.cpp",
         R"(FontSizeBase \*)",
         "",
         {},
         {},
         "audit finding 295; phase 6 task J4-4 (D53, D54)",
         {"const float size = st.FontSizeBase * st.FontScaleMain * st.FontScaleDpi;",
          "const float size = s.FontSizeBase * s.FontScaleMain * s.FontScaleDpi;"},
         {"widest = (std::max)(widest, text_width(label.c_str(), hydra::ui::g_mono_font));"},
         {{"tests/ui/uitest_harness.cpp",
           "const float size = st.FontSizeBase * st.FontScaleMain * st.FontScaleDpi;",
           "text_width, the owner"}},
         {"tests"}},
        // The Preview picker's "label##index" id. The library's chip id
        // (")##" + c.id) is another widget's and does not fit.
        {"How is a picker item's id spelled?",
         "path_item_id in src/app/path_view.h",
         R"("##" \+ std::to_string\()",
         "",
         {},
         {},
         "audit finding 352; phase 6 task J4-4 (D53, D54)",
         {"const std::string item = hydra::app::preview_path_label(b) + \"##\" + std::to_string(i);",
          "hydra::app::preview_path_label(list.buttons[index]) + \"##\" + std::to_string(index);"},
         {"group_thousands(static_cast<int64_t>(n)) + \")##\" + c.id;",
          "const std::string item = hydra::app::path_item_id(hydra::app::preview_path_label(b), i);"},
         {{"src/app/path_view.h", "return label + \"##\" + std::to_string(index);",
           "path_item_id, the owner"}},
         {"src", "tools", "tests"}},
        // pick_preview_path finds the button whose path pointer is the one
        // asked for, so no caller names a place in the list. It lives in an
        // unnamed namespace in uitest_preview.cpp, so only that file can call it.
        {"Which path does the Preview picker's test click?",
         "pick_preview_path in tests/ui/uitest_preview.cpp, by the button's path pointer",
         R"(pick_preview_path\(ctx, \d)",
         "",
         {},
         {},
         "audit finding 352; phase 6 task J4-4 (D53, D54)",
         {"pick_preview_path(ctx, 0);", "pick_preview_path(ctx, 1);"},
         {"pick_preview_path(ctx, other);",
          "pick_preview_path(ctx, h.app->viewed.record->all_paths()[1]);"},
         {},
         {"tests/ui/uitest_preview.cpp"}},
        // write_row's replace purge matches the row's own unique key, bound
        // from the row's stored cap, not a lookup's CapQuery.
        {"Which rows were stored at this SP cap?",
         "cap_match in src/store/record_store.cpp",
         R"(sp_cap=\?)",
         "",
         {},
         {},
         "audit finding 252; phase 6 task J4-2 (D53, D54)",
         {"sql += \" AND \" + p + \"sp_cap=?\";", "sql += \" AND sp_cap=?\";"},
         {"sql += \" AND \" + cap_match(a);", "sql += \" AND \" + cap_match(\"\");"},
         {{"src/store/record_store.cpp", "return std::string(a) + \"sp_cap=?\";",
           "cap_match, the owner"},
          {"src/store/record_store.cpp",
           "purge(\"hyhash=? AND chartmode=? AND sp_cap=? AND \" + lens_match(\"\") + \" AND rules_fp = ?\",",
           "write_row's purge: the row's own unique key, not a lookup"}}},
        // The INSERT and list_records build their summary
        // slots from the list and its count, so a typed run of the ten
        // placeholders is a second spelling of the list.
        {"Which columns hold a path summary, and how many?",
         "kSummaryColumnList and kSummaryColumnCount in src/store/record_store.cpp",
         R"((\?,){9}\?)",
         "",
         {},
         {},
         "audit finding 253; phase 6 task J4-2 (D53, D54)",
         {"kSummaryColumnList + \", rules_fp) VALUES (?,?,?,?,?,?,?,?,?,?,?, ?,?,?,?,?,?,?,?,?,?, \" +"},
         {"placeholders(kSummaryColumnCount) + \", \" + rules_fp_of(structure_param.c_str()) +"},
         {},
         {"src"}},
        // list_records' slots after the summary, and where each SELECT's
        // summary starts, are counted from the column lists.
        {"Which columns hold a path summary, and how many? (slots)",
         "kSummaryColumnList and kSummaryColumnCount in src/store/record_store.cpp",
         R"(column_(text|blob|int|int64|opt_i64|opt_f64)\(s,\s*1[6-9]\)|sqlite3_column_\w+\(s,\s*1[6-9]\)|read_summary\(s,\s*\d)",
         "",
         {},
         {},
         "audit finding 253; phase 6 task J4-2 (D53, D54); get_summaries' start slot, J4 join",
         {"listing.sp_cap = sqlite3_column_int(s, 16);",
          "rank_row(column_text(s, 17), column_blob(s, 19),",
          "offered.push_back({std::move(hyhash), column_text(s, 2), read_summary(s, 5)});"},
         {"listing.sp_cap = sqlite3_column_int(s, kAfterSummary);",
          "rank_row(column_text(s, kAfterSummary + 1), column_blob(s, kAfterSummary + 3),",
          "{std::move(hyhash), column_text(s, 2), read_summary(s, kFirstSummary)});"},
         {},
         {"src/store/record_store.cpp"}},
        // The constructor adds the sig column on every open, so nothing asks
        // the table whether it has it. has_column's PRAGMA table_info is a
        // different spelling; the tests' seeding helpers are out of scope.
        {"Does the charts table have the sig column?",
         "has_column in src/store/record_store.cpp",
         R"(pragma_table_info\()",
         "",
         {},
         {},
         "audit finding 299; phase 6 task J4-2 (D53, D54)",
         {"Stmt probe = prepare(db_, \"SELECT COUNT(*) FROM pragma_table_info('charts') \""},
         {"std::string sql = std::string(\"PRAGMA table_info(\") + table + \")\";"},
         {},
         {}},
        // A read that took a failed step for "no more rows" answered empty
        // (D73). Every step goes through the read owner or the write owner.
        {"Did a database statement's step succeed, and what kind is its failure?",
         "step_row (reads) and step_done (writes) in src/store/record_store.cpp",
         R"(sqlite3_step\()",
         "",
         {},
         {},
         "D73",
         {"while (sqlite3_step(s) == SQLITE_ROW) out.insert(column_text(s, 0));",
          "if (sqlite3_step(s) != SQLITE_DONE)",
          "sqlite3_step(songs);"},
         {"while (step_row(s)) out.insert(column_text(s, 0));",
          "step_done(s, \"meta_set\");"},
         {{"src/store/record_store.cpp", "const int rc = sqlite3_step(s);", "step_row"},
          {"src/store/record_store.cpp", "if (sqlite3_step(s) != SQLITE_DONE)", "step_done"}}},
        // A tool that prints an error's block itself is a second way for a
        // tool to end on an error (D73 review finding 2).
        {"How does a command-line tool end on an error?",
         "tool_error in src/app/user_messages.cpp",
         R"(plain_error_block\()",
         "",
         {},
         {{"src/ui/main.cpp", "the startup message box shows the block in a window (D72 item 1)"}},
         "D72 item 5, D73 item 4",
         {"std::fprintf(stderr, \"%s\\n\", hydra::app::plain_error_block(e).c_str());"},
         {"return hydra::app::tool_error(e, 2);"},
         {{"src/app/user_messages.h", "std::string plain_error_block(const std::exception& e);",
           "the declaration"},
          {"src/app/user_messages.cpp", "std::string plain_error_block(const std::exception& e) {",
           "the definition"},
          {"src/app/user_messages.cpp",
           "std::fprintf(stderr, \"%s\\n\", plain_error_block(e).c_str());", "tool_error"}}},
        // A read that failed to compile read the save sentence (D73).
        {"Did a database statement compile, and what kind is its failure?",
         "prepare_as in src/store/record_store.cpp",
         R"(sqlite3_prepare)",
         "",
         {},
         {},
         "D73",
         {"if (sqlite3_prepare_v2(db, sql, -1, &s.p, nullptr) != SQLITE_OK)"},
         {"Stmt s = prepare_read(db_, \"SELECT value FROM meta WHERE key=?\");"},
         {{"src/store/record_store.cpp",
           "if (sqlite3_prepare_v2(db, sql, -1, &s.p, nullptr) != SQLITE_OK)", "prepare_as"}}},
        // Hydra never writes the slot (D53 item 1); the column checks are the
        // one upgrade gate. Comments are read so the header names no number.
        {"Which schema is this database at?",
         "the has_column probes in RecordStore::RecordStore (src/store/record_store.cpp)",
         R"(user_version)",
         "",
         {},
         {},
         "audit finding 298; phase 6 task J4-2 (D53 item 1, D54)",
         {"exec(\"PRAGMA user_version = 3\");",
          "// Three tables carry an analysis (schema user_version 3):"},
         {"// Three tables carry an analysis:",
          "if (!has_column(\"charts\", \"sig\")) exec(\"ALTER TABLE charts ADD COLUMN sig TEXT\");"},
         {},
         {},
         "",
         "",
         true},
        // The library's search folds case and accents and knows field
        // prefixes; an SQL LIKE would be a second, disagreeing rule.
        {"Which library charts match a typed search?",
         "query_matches in src/app/library_query.cpp",
         R"(LIKE \?)",
         "",
         {},
         {},
         "audit finding 113; phase 6 task J4-2 (D53, D54)",
         {"if (search) sql += \" WHERE name LIKE ? OR artist LIKE ? OR charter LIKE ?\";"},
         {"\" ORDER BY name LIMIT ? OFFSET ?\");"},
         {},
         {}},
        // The thrower names the kind; reading the exception's words back is a
        // second answer, anywhere in src, user_messages.cpp included.
        {"Which plain sentence does this failure show?",
         "plain_error's switch on ErrorKind in src/app/user_messages.cpp",
         R"re(\b(what|error)(\(\))?\)?\s*[!=]=\s*"|\b(starts_with|ends_with|starts_with_any)\(\s*(\w+\.)?(what|error)(\(\))?\s*,|\b(what|error)(\(\))?\)?\.(find|rfind|compare)\(|\b(what|error)(\(\))?\)?\.substr\(.*\)\s*[!=]=)re",
         "",
         {},
         {},
         "audit finding 193 and R7.20; phase 7 tasks ER1 and ER2 (D51 call 25, D71)",
         {"if (what == \"cancelled\") return kStopped;",
          "if (starts_with(what, \"cannot write \")) return kReportWrite;",
          "if (what.find(\"(error 12002)\") != std::string_view::npos) return kNetTimeout;",
          "if (what.substr(0, prefix.size()) == prefix) return true;",
          "if (std::string(e.what()).rfind(\"x\", 0) == 0) return true;"},
         {"error_ = e.what();", "if (job.error().empty()) return;", "std::string what = e.what();"},
         {},
         {"src"}},
        // The startup message box and the command-line tools show an error's
        // sentence and raw text together; gluing the two on one line anywhere
        // else is a second layout of that block.
        {"How do an error's sentence and its raw text read as one block?",
         "plain_error_block in src/app/user_messages.cpp",
         R"re(\bplain_error\(.*\+.*(\bplain_error_detail\(|\bwhat\(\))|(\bplain_error_detail\(|\bwhat\(\)).*\+.*\bplain_error\()re",
         "",
         {},
         {},
         "D72 items 1 and 5 (task DB1)",
         {"std::string text = app::plain_error(e) + \"\\n\" + e.what();",
          "msg = plain_error_detail(e) + \"\\n\\n\" + plain_error(e);"},
         {"set_problem(app::plain_error(e));",
          "std::fprintf(stderr, \"%s\\n\", hydra::app::plain_error_block(e).c_str());"},
         {{"src/app/user_messages.cpp",
           "return plain_error(e) + \"\\n\\n\" + plain_error_detail(e);",
           "plain_error_block, the owner"}},
         {"src"}},
        // D79 B1: the batch's skip list and the library's Analyzed chip
        // once read two spellings of Ready, one in SQL. A second spelling
        // compares a row's rules with this process's in SQL or by hand.
        {"Does this chart have a current result under these settings?",
         "RecordStore::get_summaries' winner, through Candidate::ready (rank_row) in "
         "src/store/record_store.cpp",
         R"(==\s*[\w.>-]*\bfixed\b|rules_fp_bytes\([^)]*\bfixed\b|row_readable_sql\(\)\s*\+\s*"\s*AND)",
         "",
         {},
         {},
         "D79 item B1 (task COUNT-B)",
         {"bind_blob(s, idx++, rules_fp_bytes(rules.fixed));",
          R"x("(" + row_readable_sql() + " AND " + rules_fp_of("structure") + " = ?)";)x",
          "if (head->rules_fingerprint == rules_fingerprint_.fixed) out.insert(hyhash);"},
         {"const std::vector<uint8_t> auto_fp = rules_fp_bytes(rules_fingerprint_.retired_auto);",
          R"x(purge("hyhash=? AND chartmode=? AND NOT " + row_readable_sql(),)x",
          "if (found[i].status == RecordStatus::Ready) out.insert(candidates[i]);"},
         {{"src/store/record_store.cpp",
           "return rules_fp_bytes(stamp.fixed);",
           "ready_rules_fp: this process's fingerprint as rank_row takes it; rank_row alone "
           "compares it"}},
         {"src"}},
        // D79 B3: every batch number is BatchProgress's. A count of the
        // failure lines, or a counter of its own, is a second count.
        {"How many library rows has a batch analyzed, failed or skipped?",
         "BatchProgress, written only by run_batch, with plan_batch's skipped, in "
         "src/app/analysis.cpp",
         R"(\+\+\s*[\w.>-]*\b(done|completed|analyzed|failed|skipped)\b|\b(done|completed|analyzed|failed|skipped)\s*\+\+|failures\.size\(\))",
         "",
         {},
         {},
         "D76, D79 item B3 (task COUNT-B)",
         {"++snap_.failed;", "++done;",
          "const std::string head = counted((int64_t)s.failures.size(), \"chart\", \"charts\") +",
          "std::printf(\"  ...and %zu more.\\n\", failures.size() - 20);"},
         {"const std::string head = counted(s.failed, \"chart\", \"charts\") + \" failed##batchfailures\";",
          "for (int i = 0; i < shown; ++i) std::printf(\"  %s\\n\", failures[i].c_str());",
          "snap_.failed = p.failed;"},
         {{"src/app/analysis.cpp", "if (wr.failed) ++progress.failed;", "run_batch, the owner"},
          {"src/app/analysis.cpp", "else ++progress.analyzed;", "run_batch, the owner"},
          {"src/app/analysis.cpp", "++plan.skipped;", "plan_batch, the owner of skipped"},
          {"src/app/analysis.cpp", "++done;", "discover_charts' scan progress, not a batch count"},
          {"src/ui/library_dialogs.cpp", "for (size_t i = 0; i < s.failures.size(); ++i) {",
           "walks the failure lines; the heading reads s.failed"}},
         {"src/app/analysis.cpp", "src/cli/batch.cpp", "src/ui/library_jobs.cpp",
          "src/ui/library_dialogs.cpp", "src/ui/app_state.cpp"}},
        // D79 B5: RecordStore::counts() adds up every stored row at every
        // setting, so a line that printed it next to the batch's counts read
        // as a second chart count. This row flags every call to a store's
        // counts() in src/ and tools/. The one call kept is holds_results in
        // src/app/report.cpp, which asks only whether the store is empty.
        {"Does a line print the store's raw row count as a count of charts?",
         "BatchProgress and the library table count charts (D76, D79); holds_results in "
         "src/app/report.cpp is the one reader of RecordStore::counts()",
         R"(\b\w*store\w*(\.|->)counts\(\))",
         "",
         {},
         {},
         "D79 item 2 (task COUNT-B)",
         {"auto [songs, records] = store.counts();", "const auto n = app.store->counts().first;"},
         {"CHECK(m.counts().all == 6);", "const ChipCounts& counts = app.library.counts();"},
         {{"src/app/report.cpp",
           "bool holds_results(store::RecordStore& store) { return store.counts().second > 0; }",
           "holds_results, the owner: it asks only whether the store is empty"}},
         {}},
        // D79 B4: Scan library, hydra_batch and the bench tool each saved a
        // scan their own way. One function saves it, so every one leaves the
        // same library behind.
        {"How does a finished scan become the library?",
         "save_scan_as_library in src/app/analysis.cpp",
         R"(\brebuild_chart_library\s*\()",
         "",
         {"src/store/record_store.h", "src/store/record_store.cpp"},
         {},
         "D79 item 1 (task COUNT-B)",
         {"store_.rebuild_chart_library(entries);", "store->rebuild_chart_library(entries);"},
         {"if (const std::optional<std::string> problem = app::save_scan_as_library(*store, items))",
          "step_done(s, \"rebuild_chart_library\");"},
         {{"src/app/analysis.cpp", "store.rebuild_chart_library(entries);",
           "save_scan_as_library, the owner"}},
         {}},
        // D79 B2: every number the confirm shows is read off its plans. A
        // field that holds a plan's count, or a subtraction of one count from
        // another, is a second answer to "how many will run".
        {"How many charts does the confirm's batch cover, run and skip?",
         "BatchPlan::todo_rows and BatchPlan::skipped in src/app/analysis.cpp, read through "
         "AppState's batch_scope_charts() and batch_scope_with_result()",
         R"(\bint64_t\s+batch_scope_\w+\s*=|=\s*total\s*-\s*with\b)",
         "",
         {},
         {},
         "D79 item B2 (task COUNT-B)",
         {"int64_t batch_scope_charts = 0;", "const int64_t without = total - with;"},
         {"int64_t batch_scope_charts() const { return batch_redo_plan.todo_rows(); }",
          "const int64_t to_run = app.batch_plan_for(app.batch_redo).todo_rows();"},
         {},
         {"src/ui/app_state.cpp", "src/ui/app_state.h", "src/ui/library_dialogs.cpp"}},
        // A chart on a page that the library doesn't list still counts. One
        // function says how much; a page that looks a chart up in the copies
        // map itself, or starts its count at a default, is a second answer.
        {"How many library rows does a chart on a page count as when the library doesn't list it?",
         "RecordStore::copies_of in src/store/record_store.cpp",
         R"(\bcopies\s*=\s*1\s*;|\bcopies\w*\.(find|end)\()",
         "",
         {},
         {},
         "D77 item 2, D79 item 3 (task COUNT-P)",
         {"int copies = 1;",
          "const auto listed = copies.find(meta.hyhash);",
          "if (listed != copies.end()) row.copies = listed->second;"},
         {"row.copies = store::RecordStore::copies_of(library, hash);",
          "int copies = 0;",
          "for (const auto& [md5, n] : store.library_copies()) by_hash[normalize_chart_hash(md5)] += n;"},
         {{"src/store/record_store.cpp", "const auto listed = copies.find(md5);",
           "copies_of, the owner"},
          {"src/store/record_store.cpp", "return listed == copies.end() ? 1 : listed->second;",
           "copies_of, the owner"}}},
        // The scan's purge and reidentify_chart both ask this of results; a
        // second SQL spelling could keep rows the other drops.
        {"Which stored rows belong to a chart no library row lists?",
         "not_in_library in src/store/record_store.cpp",
         R"(charts\.md5 = )",
         "",
         {},
         {},
         "D87 items 3 and 4 (task storage-T1 review finding 1)",
         {"NOT EXISTS (SELECT 1 FROM charts WHERE charts.md5 = results.hyhash)",
          "\"SELECT 1 FROM charts WHERE charts.md5 = \" + col"},
         {"bind_text(s, 1, item.md5);",
          "kNamingCopiesSql + \" AS c WHERE songmeta.hyhash = c.md5\")"},
         {{"src/store/record_store.cpp",
           "return std::string(\"NOT EXISTS (SELECT 1 FROM charts WHERE charts.md5 = \") + hash + \")\";",
           "not_in_library, the owner"}}},
        // The path report and the fill comparison count charts by adding up
        // each row's copies, in C++ and in the page script alike. Counting
        // rows instead drops every extra copy. The leaderboard page counts
        // posted scores, not charts (D79), so it is not scanned.
        {"How many charts does a page or its tool count?",
         "each row's copies from RecordStore::copies_of, added up by generate_report, "
         "tally_fill_rows and the pages' stats() through the payload's \"k\"",
         R"(\['Charts',\s*fmt\(rows\.length\)|rows\.filter\(r\s*=>\s*r\.status\s*===\s*s\)\.length|\+\+stats\.|stats\.total\s*=\s*static_cast<int>\(rows\.size\(\)\))",
         "",
         {},
         {},
         "D79 item 3 (task COUNT-P)",
         {"['Charts', fmt(rows.length)],",
          "const n = s => rows.filter(r => r.status === s).length;",
          "if (r.status == \"same\") ++stats.same;",
          "stats.total = static_cast<int>(rows.size());"},
         {"['Charts', fmt(charts(rows))],",
          "const n = s => charts(rows.filter(r => r.status === s));",
          "if (r.status == \"same\") stats.same += r.copies;",
          "const beyond = rows.filter(r => r.tier === 'Beyond').length;"},
         {},
         {"src/app/report.cpp", "src/app/fill_report.cpp", "src/cli/report.cpp",
          "src/cli/fillcompare.cpp"}},
        // A chord's note order prices it: ties keep lane order, and the first
        // note is the one a squeeze-out takes. One function sorts; a second
        // sort on basescore, stable_sort's comparator or an insertion sort's
        // shift test, could break ties another way.
        {"In what order does a chord list its notes?",
         "Chord::note_list in src/core/model.cpp",
         R"([<>]\s*[\w.\[\]\s\-]*\.basescore\(\))",
         "",
         {},
         {},
         "D86, the speedups plan's task G1",
         {"while (j > 0 && bx < out.n[j - 1].basescore()) {",
          "return a.basescore() < b.basescore();"},
         {"const int at_1x = note.basescore();"},
         {{"src/core/model.cpp", "while (j > 0 && bx < out.notes_[j - 1].basescore()) {",
           "Chord::note_list, the owner"}},
         {"src"}},
        // A chord's per-lane arrays must agree on its size: note_list copies
        // every slot of notemap_ into NoteList, so a NoteList one lane short
        // would be written past its end, and the MIDI parser's cymbal flags
        // hold one slot per lane too. All read the one count.
        {"How many notes can a chord hold?",
         "Chord::kLanes in src/core/model.h",
         R"(std::array<\s*(std::optional<\s*)?(ChordNote|NoteCymbalType)\s*>?\s*,\s*\d+\s*>|\bkLanes\s*=\s*\d+)",
         "",
         {},
         {},
         "the G1 derive-once review (finding 1), widened by the wave 1 join review",
         {"std::array<std::optional<ChordNote>, 5> notemap_{};",
          "std::array<ChordNote, 5> notes_{};",
          "std::array<NoteCymbalType, 5> flag_cymbals_{};"},
         {"std::array<ChordNote, kLanes> notes_{};",
          "std::array<std::optional<ChordNote>, kLanes> notemap_{};",
          "std::array<NoteCymbalType, Chord::kLanes> flag_cymbals_{};"},
         {{"src/core/model.h", "static constexpr size_t kLanes = 5;", "Chord::kLanes, the owner"}},
         {"src"}},
        // ---- speedups task P1: the lean chart and MIDI readers ----
        // The lean MIDI read keeps a drum note only when the song parser can
        // act on it. A filter entry set from anything but the parser's own
        // pitch rules is a second list of pitches.
        {"Which drum-track notes does the lean MIDI read keep?",
         "load_songbytes_mid in src/parse/song.cpp, from midi_note_is_read",
         R"(\bnote_(on|off)\s*\[[^\]]*\]\s*=(?!=)\s*[^\s{])",
         R"(\bmidi_note_is_read\()",
         {},
         {},
         "the speedups plan, task P1 (D86)",
         {"f.note_on[n] = (n >= base && n <= base + 4) || (n == kick2x && bass2x) || marker;",
          "f.note_off[n] = marker;",
          "filter.note_on[pitch] = is_handled_note(pitch, base, kick2x);"},
         {"filter.note_on[pitch] = midi_note_is_read(pitch, true, base, kick2x);",
          "return (msg.is_note_on() ? note_on : note_off)[msg.note];"}},
        // Whether a note message starts a note is decided once, on the
        // Message. A velocity compared with 0 anywhere else, or a raw data
        // byte tested before the Message is built, is a second copy.
        {"Is this MIDI note message a note-on?",
         "Message::is_note_on in src/parse/midi.h",
         R"(\bvelocity\s*(>|==|!=|<=|>=)\s*0\b|\bclip_data_byte\s*\(\s*d2\s*\)\s*(>|==|!=)\s*0\b)",
         "",
         {},
         {},
         "the speedups plan, task P1 (D86); derive-once review of P1",
         {"bool is_noteon = (msg.type == MType::NoteOn && velocity > 0);",
          "(msg.type == MType::NoteOff || (msg.type == MType::NoteOn && velocity == 0));",
          "const bool on = high == 0x90 && clip_data_byte(d2) > 0;"},
         {"msg.velocity = clip_data_byte(d2);", "const bool is_noteon = msg.is_note_on();",
          "velocity == 127   ? NoteDynamicType::Accent"},
         {{"src/parse/midi.h",
           "bool is_note_on() const { return type == Type::NoteOn && velocity > 0; }",
           "Message::is_note_on, the owner"}}},
        // Which note messages the MIDI parser acts on is decided once. Its
        // optype and the lean read's filter both ask midi_note_is_read; the
        // pitch rules it reads are called nowhere else.
        {"Which MIDI note messages does the song parser act on?",
         "midi_note_is_read in src/parse/song.cpp",
         R"(\bis_handled_note\s*\(|\bis_midi_marker_pitch\s*\()",
         "",
         {},
         {},
         "the speedups plan, task P1 (D86); derive-once review of P1",
         {"if (!is_handled_note(note, base_, kick2x_pitch_)) return {};",
          "if (is_noteoff && !is_midi_marker_pitch(note)) return {};",
          "filter.note_off[pitch] = is_midi_marker_pitch(pitch);"},
         {"if (!midi_note_is_read(note, is_noteon, base_, kick2x_pitch_)) return {};",
          "filter.note_off[pitch] = midi_note_is_read(pitch, false, base, kick2x);"},
         {{"src/parse/song.h", "bool is_midi_marker_pitch(int pitch);", "the declaration"},
          {"src/parse/song.cpp", "bool is_midi_marker_pitch(int pitch) {", "the marker table's reader"},
          {"src/parse/song.cpp", "bool is_handled_note(int note, int base, int kick2x) {",
           "the handled-pitch rule"},
          {"src/parse/song.cpp", "return is_midi_marker_pitch(note);",
           "is_handled_note reads the marker table"},
          {"src/parse/song.cpp",
           "return on ? is_handled_note(pitch, base, kick2x) : is_midi_marker_pitch(pitch);",
           "midi_note_is_read, the owner"}}},
        // The integer fast path lives inside the three readers that answer
        // with std::stoi and std::stoll's rules. A fourth caller, or a lean_
        // twin beside them, is a second number rule.
        {"How is a .chart number read?",
         "word_stoi, word_stoll and try_parse_int in src/parse/song.cpp, through fast_leading_int",
         R"(\bfast_leading_int\s*\(|\blean_(stoi|stoll|try_parse_int)\b)",
         "",
         {},
         {},
         "the speedups plan, task P1 (D86)",
         {"int lean_stoi(std::string_view w) {", "if (fast_leading_int(w, 9, v)) return v;"},
         {"return std::stoi(std::string(w));"},
         {{"src/parse/song.cpp",
           "bool fast_leading_int(std::string_view w, size_t max_digits, int64_t& out, size_t& used) {",
           "fast_leading_int, the owner"},
          {"src/parse/song.cpp", "if (fast_leading_int(s, kFastLongLongDigits, fast, used)) {",
           "try_parse_int"},
          {"src/parse/song.cpp",
           "if (fast_leading_int(w, kFastIntDigits, v, used)) return static_cast<int>(v);",
           "word_stoi"},
          {"src/parse/song.cpp", "if (fast_leading_int(w, kFastLongLongDigits, v, used)) return v;",
           "word_stoll"}}},
        // What a .chart line says is decided once, for every section. Its
        // words are split only there.
        {"What does a .chart line say?",
         "classify_chart_line in src/parse/song.cpp",
         R"(\bsplit_ws_view\s*\()",
         "",
         {},
         {},
         "the speedups plan, task P1 (D86)",
         {"const ChartWords t = split_ws_view(valuestr);"},
         {"const ChartLine l = classify_chart_line(lhs, rhs, mix_digit, nullptr);"},
         {{"src/parse/song.cpp", "ChartWords split_ws_view(std::string_view s) {",
           "the splitter itself"},
          {"src/parse/song.cpp", "const ChartWords t = split_ws_view(valuestr);",
           "classify_chart_line, the owner"}}},
        // A .chart line becomes an op in one place, whichever section it came
        // from.
        {"What does a .chart line do to the parser?",
         "ChartParser::optype(const ChartLine&) in src/parse/song.cpp",
         R"(\bCOp\s+(ChartParser::)?optype\s*\()",
         "",
         {},
         {},
         "the speedups plan, task P1 (D86)",
         {"COp optype(const ChartDataEntry& e, int64_t tick);",
          "COp ChartParser::optype(const ChartDataEntry& e, int64_t tick) {"},
         {"COp op = optype(*e, tick);"},
         {{"src/parse/song.cpp", "COp optype(const ChartLine& e, int64_t tick);", "the declaration"},
          {"src/parse/song.cpp", "COp ChartParser::optype(const ChartLine& e, int64_t tick) {",
           "the owner"}}},
        // A MIDI track's bytes are walked once, by walk_track, which both the
        // full and the lean read call. Reading a delta anywhere else is a
        // second walker with its own length checks.
        {"Which code walks a MIDI track's bytes?",
         "walk_track in src/parse/midi.cpp",
         R"(\bread_varlen\s*\()",
         "",
         {},
         {},
         "the speedups plan, task P1 (D86)",
         {"pending += static_cast<int64_t>(read_varlen(data, pos, end));"},
         {"const uint64_t length = read_message_length(data, pos, end);"},
         {{"src/parse/midi.cpp",
           "uint64_t read_varlen(const uint8_t* data, size_t& pos, size_t end) {",
           "the reader itself"},
          {"src/parse/midi.cpp", "const uint64_t length = read_varlen(data, pos, end);",
           "read_message_length"},
          {"src/parse/midi.cpp",
           "pending += static_cast<int64_t>(read_varlen(data, pos, end));  // delta time",
           "walk_track, the owner"}}},
        // The song parser and the lean read both pick the timing, drum and
        // events tracks. A track name compared, or the first track indexed,
        // anywhere but the owners is a second copy of that choice. The lean
        // read's `i == kTimingTrack` has no text a search can tell apart from
        // any other index check, so it names the constant instead.
        {"Which MIDI track is the timing track, the drum track or an events track?",
         "MidiFile::kTimingTrack, MidiFile::drums_track and MidiTrack::is_events in src/parse/midi.h",
         R"(\bkDrumsTrackName\b|\bkEventsTrackName\b|\btracks\s*\[\s*0\s*\])",
         "",
         {},
         {},
         "the speedups plan, task P1 (D86); derive-once review of P1",
         {"if (track.name != kDrumsTrackName) continue;",
          "const bool drums = !drums_seen && track.name == kDrumsTrackName;",
          "const bool texts = drums || track.name == kEventsTrackName;",
          "for (const Message& msg : mid.tracks[0].messages) {"},
         {"if (!track.is_events()) continue;",
          "for (const Message& msg : mid.tracks[MidiFile::kTimingTrack].messages) {",
          "if (const MidiTrack* const drums = mid.drums_track()) {"},
         {{"src/parse/midi.h", R"(inline constexpr std::string_view kDrumsTrackName = "PART DRUMS";)",
           "the name itself"},
          {"src/parse/midi.h", R"(inline constexpr std::string_view kEventsTrackName = "EVENTS";)",
           "the name itself"},
          {"src/parse/midi.h",
           "inline constexpr std::string_view kRecognizedTrackNames[] = {kDrumsTrackName,",
           "the D78 name list"},
          {"src/parse/midi.h", "kEventsTrackName};", "the D78 name list"},
          {"src/parse/midi.h", "bool is_events() const { return name == kEventsTrackName; }",
           "MidiTrack::is_events, the owner"},
          {"src/parse/midi.cpp", "if (track.name == kDrumsTrackName) return &track;",
           "MidiFile::drums_track, the owner"}}},
        // How many values a MIDI data byte holds is named once. A bare 128 as
        // a bound or a size, or a bare 127 as the clip, is a second copy.
        // Clone Hero's accent velocity is a different question and is not a
        // bound, so `velocity == 127` is left alone.
        {"How many values does a MIDI data byte hold?",
         "kMidiDataValues in src/parse/midi.h",
         R"([<,]\s*128\b|\[\s*128\s*\]|\{\s*127\s*\})",
         "",
         {},
         {},
         "the speedups plan, task P1 (D86); derive-once review of P1",
         {"uint8_t clip_data_byte(uint8_t b) { return b < 128 ? b : uint8_t{127}; }",
          "bool note_on[128] = {};", "for (int pitch = 0; pitch < 128; ++pitch) {"},
         {"bool note_on[kMidiDataValues] = {};", "velocity == 127   ? NoteDynamicType::Accent",
          "inline constexpr int kMidiDataValues = 128;"},
         {},
         {"src/parse/midi.h", "src/parse/midi.cpp", "src/parse/song.cpp"}},
        // The .chart sections the reader keeps by name are named once, so
        // the code that keeps them and the code that reads them back cannot
        // spell them apart. Scoped to song.cpp, where the .chart reader
        // lives; "Song" means a title elsewhere.
        {"Which .chart sections does the reader keep by name?",
         "kSongSection, kSyncTrackSection and kEventsSection in src/parse/song.cpp",
         R"re("(Song|SyncTrack|Events)")re",
         "",
         {},
         {},
         "the speedups plan, task P1 (D86); derive-once review of P1",
         {R"(if (name == "Song" || name == "SyncTrack") {)",
          R"(const ChartSection& song_sec = sections_.at("Song");)",
          R"(} else if (name == "Events") {)"},
         {"if (name == kSongSection || name == kSyncTrackSection) {",
          "auto sync_it = sections_.find(std::string(kSyncTrackSection));"},
         {{"src/parse/song.cpp", R"(constexpr std::string_view kSongSection = "Song";)",
           "the name itself"},
          {"src/parse/song.cpp", R"(constexpr std::string_view kSyncTrackSection = "SyncTrack";)",
           "the name itself"},
          {"src/parse/song.cpp", R"(constexpr std::string_view kEventsSection = "Events";)",
           "the name itself"}},
         {"src/parse/song.cpp"}},
        // A test that hashes a chart's parse calls chart_parse_hash, so the
        // tests cannot drift from each other when the digest gains a part.
        // tools/bench.cpp keeps its own loop to time the parts apart, and is
        // outside this row's scope.
        {"How is one chart's parse digest worked out in a test?",
         "digest::chart_parse_hash in tests/song_digest.h",
         R"(\b(with_dynamics|failure_hash|song_digest)\s*\()",
         "",
         {"tests/song_digest.h"},
         {},
         "the speedups plan, task P1 (D86); derive-once review of P1",
         {"got = digest::with_dynamics(", "h = digest::failure_hash(digest::failure_text(e));",
          "digest::song_digest(song),"},
         {"const uint64_t got = digest::chart_parse_hash(edge_dir() + e.file, run.settings, &fail);",
          R"(#include "song_digest.h")"},
         {},
         {"tests"}},
        // D86 item 3: the checkpoint threshold changes only while a batch
        // runs, and the log is emptied only when it ends. A second place that
        // sets either one is a second answer.
        {"When does the WAL checkpoint threshold change, and when is the log emptied?",
         "RecordStore::BatchWrites in src/store/record_store.cpp",
         R"(wal_(auto)?checkpoint)",
         "",
         {"src/store/record_store.h", "src/store/record_store.cpp"},
         {},
         "D86 item 3 (task W1)",
         {"exec(\"PRAGMA wal_autocheckpoint=10000\");",
          "sqlite3_exec(db, \"PRAGMA wal_checkpoint(TRUNCATE)\", nullptr, nullptr, nullptr);",
          "sqlite3_wal_checkpoint_v2(db, nullptr, SQLITE_CHECKPOINT_TRUNCATE, nullptr, nullptr);"},
         {"const store::RecordStore::BatchWrites batch_writes(store);",
          "exec(\"PRAGMA journal_mode=WAL\");"},
         {},
         {}},
        // D86 items 1 and 2: the batch's writer is the one caller that saves
        // charts in groups, and the one that retries a lost group's charts
        // alone. Another caller opening a group would hold the store lock
        // without that retry.
        {"Which code saves charts in groups?",
         "run_batch in src/app/analysis.cpp",
         R"(\b(begin|commit)_save_group\s*\()",
         "",
         {"src/store/record_store.h", "src/store/record_store.cpp", "src/app/analysis.cpp"},
         {},
         "D86 items 1 and 2 (task W1)",
         {"store.begin_save_group();", "store_->commit_save_group();"},
         {"if (!store.save_group_open()) {", "bool save_group_open() const { return group_open_; }"},
         {},
         {}},
        // After a failure, rollback_if_open and a grouped save_analysis both
        // need to know whether sqlite already rolled the transaction back.
        // Only the helper asks sqlite; record_store.cpp is not exempt as a
        // file, so a second ask there fails like one anywhere else.
        {"Is a transaction still open, or has sqlite already rolled it back?",
         "transaction_open in src/store/record_store.cpp",
         R"(sqlite3_get_autocommit\s*\()",
         "",
         {},
         {},
         "derive-once review of task W1 (finding 1)",
         {"if (!sqlite3_get_autocommit(db)) sqlite3_exec(db, \"ROLLBACK\", nullptr, nullptr, nullptr);",
          "} else if (sqlite3_get_autocommit(db_)) {"},
         {"if (transaction_open(db_)) {", "} else if (transaction_open(db_)) {"},
         {{"src/store/record_store.cpp",
           "bool transaction_open(sqlite3* db) { return !sqlite3_get_autocommit(db); }",
           "transaction_open, the owner: rollback_if_open and save_analysis call it"}}},
        // S1: a fresh zero-filled megabyte per hashed file cost the library
        // scan measurable time, so each hashing thread keeps one buffer.
        {"What buffer does a chart file's MD5 read through?",
         "stream_md5's one thread_local buffer in src/app/analysis.cpp",
         R"(std::vector<uint8_t>\s+\w+\s*\(\s*1\s*<<\s*20\s*\))",
         "",
         {},
         {},
         "D86 speedups plan, task S1",
         {"std::vector<uint8_t> buf(1 << 20);", "std::vector<uint8_t> fresh(1<<20);"},
         {"constexpr size_t kSngHeadCapture = 1 << 20;", "std::vector<uint8_t> head;",
          "constexpr size_t kSrbMaxMetadata = 1 << 20;"},
         {{"src/app/analysis.cpp", "thread_local std::vector<uint8_t> buf(1 << 20);",
           "stream_md5, the owner"}},
         {"src/app/analysis.cpp"}},
    };
    return r;
}

const std::vector<KnownCopy>& known_copies() {
    static const std::vector<KnownCopy> k = {
        {"Does the library list this chart?", "src/app/dm_report.cpp",
         "row.status = library.count(s.identifier) ? \"not analyzed\" : \"not in library\";",
         "a follow-up to task som-a: collect_dm_rows calls report::library_lists (dm_report.cpp "
         "was outside som-a's files)"},
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

// A line the scan accepts by its exact text: an owner's own line or a
// baseline copy. Both kinds go in one list and follow one rule.
struct ListedLine {
    std::string question;
    std::string file;       // repo-relative, forward slashes
    std::string line_text;  // trimmed
    std::string stale;      // the message when no source line uses it
};

// Owner lines first, so an owner line is used before a baseline entry with
// the same text.
std::vector<ListedLine> listed_lines() {
    std::vector<ListedLine> out;
    for (const OwnerRule& r : rules())
        for (const OwnerLine& o : r.owner_lines)
            out.push_back({r.question, o.file, o.line_text,
                           "owner line no longer matches (update it)"});
    for (const KnownCopy& k : known_copies())
        out.push_back({k.question, k.file, k.line_text,
                       "baseline entry no longer matches (remove it)"});
    return out;
}

enum class Take { taken, used_up, unlisted };

// Each listed entry covers exactly one line of source: the first entry with
// this question, file and text that no earlier line used takes it. A
// word-for-word copy finds every such entry used (used_up).
Take take_listed_line(const std::vector<ListedLine>& listed, std::vector<bool>& used,
                      const std::string& question, const std::string& file,
                      const std::string& text) {
    bool any = false;
    for (size_t i = 0; i < listed.size(); ++i) {
        const ListedLine& l = listed[i];
        if (l.question != question || l.file != file || l.line_text != text) continue;
        any = true;
        if (!used[i]) {
            used[i] = true;
            return Take::taken;
        }
    }
    return any ? Take::used_up : Take::unlisted;
}

// Whether a file the walk finds is C++ source, the files the C++ scans read.
bool is_cpp_source(const fs::path& path) {
    const fs::path ext = path.extension();
    return ext == ".cpp" || ext == ".h";
}

// Whether a trimmed source line is a comment line, which the scans skip.
bool is_line_comment(std::string_view trimmed) {
    return hydra::starts_with(trimmed, "//");
}

// How a scan that lists lines ends: each entry no source line took is stale,
// and the case fails with one line per problem.
void check_scan(std::vector<std::string> problems, const std::vector<ListedLine>& listed,
                const std::vector<bool>& used) {
    for (size_t i = 0; i < listed.size(); ++i) {
        if (used[i]) continue;
        problems.push_back(listed[i].stale + ": " + listed[i].file + ": " + listed[i].line_text);
    }
    std::ostringstream report;
    for (const std::string& p : problems) report << p << "\n";
    INFO(report.str());
    CHECK(problems.empty());
}

}  // namespace

TEST_CASE("single-owner listed lines each cover one line of source") {
    const std::vector<ListedLine> listed = {{"Q?", "src/a.cpp", "x();", "stale"},
                                            {"Q?", "src/a.cpp", "y();", "stale"},
                                            {"Q?", "src/a.cpp", "y();", "stale"}};
    std::vector<bool> used(listed.size(), false);
    CHECK(take_listed_line(listed, used, "Q?", "src/a.cpp", "x();") == Take::taken);
    CHECK(take_listed_line(listed, used, "Q?", "src/a.cpp", "x();") == Take::used_up);
    // Two identical entries cover two identical lines, and no third.
    CHECK(take_listed_line(listed, used, "Q?", "src/a.cpp", "y();") == Take::taken);
    CHECK(take_listed_line(listed, used, "Q?", "src/a.cpp", "y();") == Take::taken);
    CHECK(take_listed_line(listed, used, "Q?", "src/a.cpp", "y();") == Take::used_up);
    CHECK(take_listed_line(listed, used, "Q?", "src/b.cpp", "x();") == Take::unlisted);
    CHECK(take_listed_line(listed, used, "Other?", "src/a.cpp", "x();") == Take::unlisted);
}

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
            CHECK_FALSE(o.why.empty());
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

// Does this rule scan the file `rel`, which sits under the top folder `sub`?
bool in_scope(const OwnerRule& r, const std::string& sub, const std::string& rel) {
    if (r.scope.empty()) return sub == "src" || sub == "tools";
    for (const std::string& s : r.scope)
        if (s == sub || s == rel) return true;
    return false;
}

TEST_CASE("single-owner rules hold across src/, tools/ and tests/") {
    const fs::path root = sourcetree::root();
    const std::vector<CompiledRule> compiled = compile_rules();

    const std::vector<ListedLine> listed = listed_lines();
    std::vector<bool> used(listed.size(), false);  // which listed entries a line took
    std::vector<std::string> problems;
    int files = 0;
    std::vector<int> functions_found(compiled.size(), 0);
    sourcetree::for_each_source_file([&](const fs::path& path, const std::string& rel) {
            if (!is_cpp_source(path)) return;
            // This file holds every rule's examples as text.
            if (rel == "tests/test_single_owner.cpp") return;
            const std::string sub = rel.substr(0, rel.find('/'));  // the top folder
            ++files;
            std::ifstream in(path);
            std::string line;
            int lineno = 0;
            std::vector<bool> in_function(compiled.size(), false);
            while (std::getline(in, line)) {
                ++lineno;
                const std::string t = hydra::trim(line);
                if (t.empty()) continue;
                const bool comment = is_line_comment(t);
                for (size_t ci = 0; ci < compiled.size(); ++ci) {
                    const CompiledRule& c = compiled[ci];
                    const OwnerRule& rule = *c.rule;
                    if (!in_scope(rule, sub, rel)) continue;
                    if (comment && !rule.scan_comments) continue;
                    if (!rule.function.empty()) {
                        if (rel != rule.function_file) continue;
                        if (!in_function[ci] && line.find(rule.function) != std::string::npos) {
                            in_function[ci] = true;
                            ++functions_found[ci];
                        }
                        if (!in_function[ci]) continue;
                        if (line == "}") in_function[ci] = false;
                    }
                    bool skip = false;
                    for (const std::string& o : rule.owner_files) skip = skip || rel == o;
                    for (const Exempt& x : rule.exempt) skip = skip || rel == x.file;
                    if (skip || !flags_line(c, line)) continue;
                    const Take take = take_listed_line(listed, used, rule.question, rel, t);
                    if (take == Take::taken) continue;
                    const std::string where = rel + ":" + std::to_string(lineno);
                    if (take == Take::used_up)
                        problems.push_back(where + ": a second copy of a listed line (each "
                                           "entry covers one line) answers \"" +
                                           rule.question + "\": " + t);
                    else
                        problems.push_back(where + ": answers \"" + rule.question +
                                           "\", which belongs to " + rule.owner + ": " + t);
                }
            }
    });
    // A function-scoped rule found its function exactly once; a renamed or
    // moved function would otherwise leave the rule checking nothing.
    for (size_t ci = 0; ci < compiled.size(); ++ci) {
        const OwnerRule& rule = *compiled[ci].rule;
        if (rule.function.empty()) continue;
        INFO(rule.question << ": \"" << rule.function << "\" in " << rule.function_file);
        CHECK(functions_found[ci] == 1);
    }
    // A rule scoped to single files names files that exist; a renamed one
    // would otherwise leave the rule checking nothing.
    for (const CompiledRule& c : compiled) {
        for (const std::string& s : c.rule->scope) {
            if (s.find('/') == std::string::npos) continue;
            INFO(c.rule->question << ": scoped to " << s);
            CHECK(fs::is_regular_file(root / fs::u8path(s)));
        }
    }
    CHECK(files > 100);  // the scan found the sources
    check_scan(problems, listed, used);
}

// D23: a change under src/parse that alters what a chart reads as must bump
// the results stamp. The rule is a comment, which does not compile and which
// the row scan skips, so this case reads it: the block between the "Results"
// banner and the kResultsStamp line in src/store/stored_versions.h must name
// src/parse next to src/search and src/core. Moved here from
// a stamps test; the repo root comes from tests/source_tree.h.
TEST_CASE("single-owner: the results stamp's bump rule names the chart readers (D23)") {
    std::ifstream in(sourcetree::root() / "src" / "store" / "stored_versions.h");
    REQUIRE(in.good());
    std::stringstream ss;
    ss << in.rdbuf();
    const std::string text = ss.str();
    const size_t stamp = text.find("kResultsStamp{");
    REQUIRE(stamp != std::string::npos);
    const size_t banner = text.rfind("// ---- Results", stamp);
    REQUIRE(banner != std::string::npos);
    const std::string rule = text.substr(banner, stamp - banner);
    CHECK(rule.find("src/search") != std::string::npos);
    CHECK(rule.find("src/core") != std::string::npos);
    CHECK(rule.find("src/parse") != std::string::npos);
}

// D94: the User Guide quotes the Preview's changed-chart line in backticks.
// The sentence is read off the owner line of the row that guards it, and that
// line is checked against src/ui/preview_tab.cpp, so the words are typed only
// in the code and in that row.
TEST_CASE("single-owner: the User Guide quotes the Preview's changed-chart line as the code has it") {
    const OwnerLine* owner = nullptr;
    for (const OwnerRule& r : rules())
        if (r.question == "What does the Preview say when the chart changed since it was analyzed?")
            for (const OwnerLine& o : r.owner_lines)
                if (o.file == "src/ui/preview_tab.cpp") owner = &o;
    REQUIRE(owner != nullptr);

    const auto slurp = [](const fs::path& p) {
        std::ifstream in(p);
        REQUIRE(in.good());
        std::stringstream ss;
        ss << in.rdbuf();
        return ss.str();
    };
    INFO("owner line: " << owner->line_text);
    REQUIRE(slurp(sourcetree::root() / fs::u8path(owner->file)).find(owner->line_text) !=
            std::string::npos);

    // The C++ literal on that line, between its first and last double quote.
    const size_t open = owner->line_text.find('"');
    const size_t close = owner->line_text.rfind('"');
    REQUIRE(open != std::string::npos);
    REQUIRE(close > open);
    const std::string sentence = owner->line_text.substr(open + 1, close - open - 1);
    REQUIRE(sentence.find('\\') == std::string::npos);  // no escapes to undo

    INFO("docs/UserGuide.md should quote `" << sentence << "`");
    CHECK(slurp(sourcetree::root() / "docs" / "UserGuide.md").find("`" + sentence + "`") !=
          std::string::npos);
}

// E3 (findings 180, 243, 245 and 56): "does this path need any timing?" is
// Path::needs_timing's, so no code line under src/ or tools/ asks it with a
// zero test of its own. The all-0 list's 0 ms limit (D85) is set in one
// place, allzero_options, and the shortcut that skips the all-0 pass reads it
// from there. What the Score range's INI int means is
// Settings::search_depth_mode's, the one line in src/app/config.cpp that
// compares it, however the comparison is spelled. Comment lines are skipped
// like the row scan does.
TEST_CASE("single-owner: the all-0 limit and the depth-mode int each have one owner (E3, D85)") {
    const std::regex zero_limit(
        R"(difficulty\(\)\.value_or\(0(\.0+)?\)\s*(<=|>|<|>=)\s*0(\.0+)?(?![\d.]))"
        R"(|value_or\(0(\.0+)?\)\s*<=\s*0(\.0+)?(?![\d.]))"
        R"(|ms_filter\s*=\s*(std::optional<double>\()?0(\.0+)?(?![\d.]))"
        R"(|within_(ms_)?limit\(\s*0(\.0+)?\s*\))");
    const std::regex depth_int(R"(\bdepth_mode\s*(==|!=|>=|<=|>|<)\s*[01]\b|case\s+1\s*:.*depth)");
    std::vector<std::string> zero_hits, depth_hits;
    sourcetree::for_each_source_file([&](const fs::path& file, const std::string& rel) {
        if (rel.compare(0, 6, "tests/") == 0) return;
        if (!is_cpp_source(file)) return;
        std::ifstream in(file);
        std::string line;
        while (std::getline(in, line)) {
            const std::string t = hydra::trim(line);
            if (is_line_comment(t)) continue;
            if (std::regex_search(t, zero_limit)) zero_hits.push_back(rel + ": " + t);
            if (std::regex_search(t, depth_int)) depth_hits.push_back(rel + ": " + t);
        }
    });
    REQUIRE_MESSAGE(zero_hits.size() == 1,
                    (zero_hits.size() < 2 ? std::string() : zero_hits[1]));
    CHECK(zero_hits.front() == "src/search/pather.cpp: options.ms_filter = 0.0;");
    REQUIRE(depth_hits.size() == 1);
    CHECK(depth_hits.front() ==
          "src/app/config.cpp: return depth_mode == 1 ? DepthMode::Points : DepthMode::Scores;");
}

// The walk reads only .cpp and .h files under src, tools and tests, so a row
// scoped to a single file outside them (installer/hydra.iss, Inno Setup's
// script) is checked here, with the same verdict. No listed line covers such
// a file, so any flagged line fails. Comment lines are skipped as the walk
// skips them.
TEST_CASE("single-owner rules hold in the single files outside the walk") {
    for (const CompiledRule& c : compile_rules()) {
        for (const std::string& s : c.rule->scope) {
            const std::string top = s.substr(0, s.find('/'));
            if (s.find('/') == std::string::npos || top == "src" || top == "tools" || top == "tests")
                continue;
            std::ifstream in(sourcetree::root() / fs::u8path(s));
            REQUIRE(in.good());
            std::string line;
            while (std::getline(in, line)) {
                const std::string t = hydra::trim(line);
                if (is_line_comment(t) && !c.rule->scan_comments) continue;
                INFO(s << " answers \"" << c.rule->question << "\", which belongs to "
                       << c.rule->owner << ": " << t);
                CHECK_FALSE(flags_line(c, line));
            }
        }
    }
}

// J3-7 (findings 229, 230, 231, 271, 272 and the probe's unit factor) and
// J3-8 (findings 232 to 236, 238, 239, 273, 328): the Clone Hero probe under
// tools/ is Python, which the row scan above skips (it reads .cpp and .h
// only), so these rows get their own scan of every .py file under tools/. A
// __pycache__ .pyc is never read: only .py files are. Each row lists, by
// exact text, the lines allowed to answer its question. Listed lines follow
// take_listed_line, like the row scan's. Python comment lines are skipped.
TEST_CASE("single-owner: the Clone Hero probe's facts each have one owner (J3-7, J3-8)") {
    struct ProbeRow {
        std::string question;
        std::string pattern;
        std::vector<std::string> allowed;  // "rel: trimmed line": the lines allowed to answer it
        std::vector<std::string> must_match;
        std::vector<std::string> must_not_match;
    };
    const std::vector<ProbeRow> rows = {
        {"Where does the probe skip GameAssembly's own copies of the window constants?",
         R"x(\b0x4000000\b)x",
         {"tools/ch_probe/engine_finder.py: MODULE_SPAN = 0x4000000"},
         {"module_end = proc.module_base + 0x4000000"},
         {"MODULE_SPAN = 0x40000000"}},
        {"How do raw engine bytes become numbers?",
         R"x(struct\.unpack(_from)?\()x",
         {R"x(tools/ch_probe/process.py: return struct.unpack("<d", raw)[0])x",
          R"x(tools/ch_probe/process.py: return struct.unpack("<I", raw)[0])x",
          R"x(tools/ch_probe/process.py: return struct.unpack("<Q", raw)[0])x"},
         {R"x(return struct.unpack_from("<d", raw, off)[0])x",
          R"x(return struct.unpack("<d", bytes(xmm0_bytes[:8]))[0])x"},
         {R"x(struct.pack_into("<d", block, C.OFF_SONG_CLOCK, 12.5))x"}},
        {"Is the engine in precision mode?",
         R"x(&\s*C\.PRECISION_MODE_BIT\)|flags\s*&\s*C\.PRECISION_MODE_BIT)x",
         {"tools/ch_probe/engine.py: return (flags & C.PRECISION_MODE_BIT) != 0"},
         {"return (flags & C.PRECISION_MODE_BIT) != 0",
          "return bool(self.flags & C.PRECISION_MODE_BIT)"},
         {"other_bits = 0xFFFFFFFF & ~C.PRECISION_MODE_BIT"}},
        {"How does the probe read and write game memory?",
         R"x(k32\.(Read|Write)ProcessMemory\()x",
         {"tools/ch_probe/process.py: ok = k32.ReadProcessMemory(",
          "tools/ch_probe/process.py: ok = k32.WriteProcessMemory("},
         {"ok = self._win32.k32.ReadProcessMemory(",
          "ok = self._win32.k32.WriteProcessMemory("},
         {"k32.FlushInstructionCache.restype = wintypes.BOOL"}},
        // The owner is constants.s_to_ms/ms_to_s, which multiply by MS_PER_S
        // and so match nothing here. The bare-name alternative catches a
        // variable called just ms or s with an optional subscript.
        {"How many milliseconds is a second (probe)?",
         R"x(\b\w*(_s|_S|raw|est|clock|back|front)\s*\*\s*1000(\.0)?(?![\d.]))x"
         R"x(|\b\w*(_ms|_MS)\s*/\s*1000(\.0)?(?![\d.])|\)\s*[*/]\s*1000(\.0)?(?![\d.]))x"
         R"x(|\b(ms\[\d+\]|s(\[\d+\])?)\s*[*/]\s*1000(\.0)?(?![\d.]))x",
         {},
         {"window_ms=dbl(C.OFF_TOTAL_WINDOW) * 1000.0,",
          "self._pending = (self._spacing_ms, thread_context.xmm0_double() * 1000.0)",
          "return ms[0] / 1000",
          "ms[0] += round(s * 1000)"},
         {"bpm_microbeats = int(round(bpm * 1000))",
          R"x(f"  0 = B {int(BPM * 1000)}",)x",
          R"x("-i", "anullsrc=r=44100:cl=stereo",)x"}},
        {"Which keys does EngineModel.constants() use?",
         R"x("(normal|precision)_(back|front)"|"hitcheck_threshold"|"(normal|precision)_")x",
         {R"x(tools/ch_probe/constants.py: CONST_KEY_PREFIX_NORMAL = "normal_")x",
          R"x(tools/ch_probe/constants.py: CONST_KEY_PREFIX_PRECISION = "precision_")x",
          R"x(tools/ch_probe/constants.py: CONST_KEY_HITCHECK_THRESHOLD = "hitcheck_threshold")x"},
         {R"x("normal_back": read(C.RVA_CONST_NORMAL_BACK),)x"},
         {"C.CONST_KEY_NORMAL_BACK: read(C.RVA_CONST_NORMAL_BACK),"}},
        // The owner is Process.resolve; the test fakes call it (tests/fakes.py).
        {"How does a test fake turn an RVA into an address?",
         R"x(return\s+(self\.)?(module_base|BASE)\s*\+\s*rva)x",
         {"tools/ch_probe/process.py: return self.module_base + rva"},
         {"return BASE + rva"},
         {"expected_addr = proc.resolve(C.RVA_DRUMS_ENGINE_CTOR)"}},
        {"How long does a runner wait before reading a press's result?",
         R"x(\b\w*SETTLE_MS\s*=(?!=))x",
         {"tools/ch_probe/constants.py: INPUT_SETTLE_MS = 250"},
         {"SETTLE_MS = 250        # wait this long past the note before reading the result",
          "SETTLE_MS = 250   # read the result this long after the note (as walk_edges.py)"},
         {"wait_for(max(p.second_ms, p.second_ms + p.offset_ms) + constants.INPUT_SETTLE_MS)"}},
        {"Did a press register as a hit?",
         R"x(\b(after|score_after)\s*>\s*(before|score_before)\b|\bafter_score\s*>\s*before_score\b)x"
         R"x(|\.score\s*>\s*\w+\.score\b)x",
         {"tools/ch_probe/engine.py: return score_after > score_before"},
         {"hit = after_score > before_score", "hit = score_after > score_before",
          "measured if from_engine else None, after.score > before.score, measured)",
          "score_rises = sum(1 for a, b in zip(samples, samples[1:]) if b.score > a.score)"},
         {"hit = pressed_input_hit(score_before, score_after)"}},
        {"How does a runner find the game window?",
         R"x(\bFindWindowW\b)x",
         {"tools/ch_probe/input_driver.py: find = ctypes.windll.user32.FindWindowW"},
         {R"x(ch_hwnd = user32.FindWindowW(None, "Clone Hero"))x",
          R"x(return ctypes.windll.user32.FindWindowW(None, "Clone Hero") or 0)x"},
         {"ch_hwnd = find_game_window()"}},
        {"Which .chart note is the probe's kick?",
         R"x(\b\w*KICK\s*=\s*0\b)x",
         {"tools/ch_probe/constants.py: PROBE_CHART_NOTE_KICK = 0"},
         {"KICK = 0  # .chart drum lane 0; play_chart.py maps it to the kick key"},
         {"KICK = 4", "DRUM_NOTE_KICK = C.PROBE_CHART_NOTE_KICK"}},
        // The test pins are today's text, pinned once (test_song_names_are_spelled_once).
        {"Where are the probe songs installed, and what are they called?",
         R"x(Clone Hero\\songs\\Hydra Probe|"Window Map"|"Edge Walk"|"Hydra Probe - ")x",
         {R"x(tools/ch_probe/probe_songs.py: DEFAULT_OUT = r"C:\Clone Hero\songs\Hydra Probe")x",
          R"x(tools/ch_probe/probe_songs.py: WINDOW_MAP = "Window Map")x",
          R"x(tools/ch_probe/probe_songs.py: EDGE_WALK = "Edge Walk")x",
          R"x(tools/ch_probe/probe_songs.py: NAME_PREFIX = "Hydra Probe - ")x",
          R"x(tools/ch_probe/tests/test_probe_songs.py: self.assertEqual(P.WINDOW_MAP, "Window Map"))x",
          R"x(tools/ch_probe/tests/test_probe_songs.py: self.assertEqual(P.EDGE_WALK, "Edge Walk"))x",
          R"x(tools/ch_probe/tests/test_probe_songs.py: self.assertEqual(P.NAME_PREFIX, "Hydra Probe - "))x"},
         {R"x(PROBE_ROOT = r"C:\Clone Hero\songs\Hydra Probe")x",
          R"x(return os.path.join(live.PROBE_ROOT, "Window Map"))x"},
         {R"x(python tools\\ch_probe\\experiments\\watch_window.py [<probe song name> | folder])x"}},
        {"What is the probe's audio file called?",
         R"x("song\.ogg")x",
         {R"x(tools/ch_probe/probe_songs.py: SONG_OGG = "song.ogg")x",
          R"x(tools/ch_probe/tests/test_probe_songs.py: self.assertIn('  MusicStream = "song.ogg"\n', text))x",
          R"x(tools/ch_probe/tests/test_probe_songs.py: self.assertEqual(calls, [(os.path.join(root, "x", "song.ogg"), 4000)]))x",
          R"x(tools/ch_probe/tests/test_probe_chart.py: music_stream="song.ogg"))x",
          R"x(tools/ch_probe/tests/test_probe_chart.py: self.assertIn('  MusicStream = "song.ogg"\n', with_stream))x"},
         {R"x(music_stream="song.ogg")x"},
         {R"x(music_stream=SONG_OGG)x"}},
        {"How many ticks is a millisecond (probe)?",
         R"x(/\s*60000(\.0)?\b|\*\s*\w+\s*/\s*60000)x",
         {"tools/ch_probe/probe_chart.py: return resolution * bpm / 60000.0"},
         {"self.assertEqual(P.RESOLUTION * P.BPM / 60000.0, 1.0)"},
         {"return int(round(ms * _ticks_per_ms(resolution, bpm)))"}},
        {"What window does the formula predict?",
         R"x(\*\*\s*exponent\b)x",
         {"tools/ch_probe/experiments/analysis.py: return (t * c1 - (t ** exponent) * c2) * c3"},
         {"return ((t * c1 - (t ** exponent) * c2) * c3 - c4) / divisor",
          "return (c0 - (t * c1 - (t ** exponent) * c2) * c3) / divisor"},
         {"exponent=exponent) - c4) / divisor"}},
        {"Which lines of a probe .chart are drum notes? (probe tests)",
         R"x(N\\s\+\(\\d\+\)\\s\+\(\\d\+\)|= N 0 0\$)x",
         {R"x(tools/ch_probe/tests/chart_reader.py: _NOTE_LINE = re.compile(r"^(\d+)\s*=\s*N\s+(\d+)\s+(\d+)$"))x"},
         {R"x(match = re.match(r"^(\d+)\s*=\s*N\s+(\d+)\s+(\d+)$", stripped))x",
          R"x(return [int(m) for m in re.findall(r"^\s*(\d+) = N 0 0$", text, re.M)])x"},
         {R"x(i_drums = text.find("[ExpertDrums]"))x"}},
        // The test line pins D54's recorded value.
        {"How long is a key held?",
         R"x(\b0\.003\b)x",
         {"tools/ch_probe/input_driver.py: KEY_HOLD_S = 0.003",
          "tools/ch_probe/tests/test_input_driver.py: self.assertEqual(input_driver.KEY_HOLD_S, 0.003)"},
         {"def press_chord(self, lanes: Iterable[int], *, hold_s: float = 0.003,",
          "def press_chord(self, lanes: Sequence[int], *, hold_s: float = 0.003) -> list:"},
         {"def press_chord(self, lanes: Iterable[int], *, hold_s: float = KEY_HOLD_S,"}},
        // The owner is the named constants in constants.py, which the pattern
        // (the old inline cut-offs) does not match.
        {"Which cut-offs judge the probe's verdicts?",
         R"x(<=\s*10\.0\b|fresh_s:\s*float\s*=\s*0\.002|max_fill_s:\s*float\s*=\s*0\.05)x"
         R"x(|tolerance_ms:\s*float\s*=\s*1\.0|decisive_fraction:\s*float\s*=\s*0\.8)x",
         {},
         {"if len(hits) == score_rises and max(abs(d) for d in diffs) <= 10.0:",
          "fresh_s: float = 0.002, max_fill_s: float = 0.05) -> None:",
          "tolerance_ms: float = 1.0,", "decisive_fraction: float = 0.8,"},
         {"clock = SongClock(engine.song_clock, max_fill_s=0.0)",
          "fresh_s: float = C.CLOCK_FRESH_S,"}},
    };

    std::vector<std::regex> compiled;
    for (const ProbeRow& r : rows) {
        compiled.emplace_back(r.pattern);
        for (const std::string& line : r.must_match) {
            INFO(r.question << " should flag: " << line);
            CHECK(std::regex_search(line, compiled.back()));
        }
        for (const std::string& line : r.must_not_match) {
            INFO(r.question << " should not flag: " << line);
            CHECK_FALSE(std::regex_search(line, compiled.back()));
        }
    }

    // The allowed lines count the way the row scan's do: through ListedLine and
    // take_listed_line. A repo-relative path holds no ": ", so the first one
    // splits an entry into its file and its text.
    std::vector<ListedLine> listed;
    for (const ProbeRow& r : rows)
        for (const std::string& a : r.allowed) {
            const size_t split = a.find(": ");
            REQUIRE(split != std::string::npos);
            listed.push_back({r.question, a.substr(0, split), a.substr(split + 2),
                              "listed line is gone (stale)"});
        }
    std::vector<bool> used(listed.size(), false);
    std::vector<std::string> problems;
    sourcetree::for_each_source_file([&](const fs::path& path, const std::string& rel) {
        if (rel.compare(0, 6, "tools/") != 0 || path.extension() != ".py") return;
        std::ifstream in(path);
        std::string line;
        int lineno = 0;
        while (std::getline(in, line)) {
            ++lineno;
            const std::string t = hydra::trim(line);
            if (t.empty() || t[0] == '#') continue;
            for (size_t i = 0; i < rows.size(); ++i) {
                if (!std::regex_search(t, compiled[i])) continue;
                const Take take = take_listed_line(listed, used, rows[i].question, rel, t);
                if (take == Take::taken) continue;
                const std::string where = rel + ":" + std::to_string(lineno);
                if (take == Take::used_up)
                    problems.push_back(where + ": a second copy of a listed line (each "
                                       "entry covers one line) answers \"" +
                                       rows[i].question + "\": " + t);
                else
                    problems.push_back(where + ": answers \"" + rows[i].question + "\": " + t);
            }
        }
    });
    check_scan(problems, listed, used);
}

// ---- Copied blocks of code (task MR1, ADR 0025) ----
//
// A block of code pasted into a second place is the plainest way to write a
// rule twice, and a text scan finds every one. This scan reads the .cpp and
// .h files the row scan reads and fails on any run of kCloneWindowLines code
// lines that appears in two places, in two files or twice in one. Overlapping
// runs merge into one copied block, named once with both places. The
// baseline (known_clones) lists the copies that were already there; like
// known_copies, it only shrinks. Vendored code in third_party/ is outside the
// walk (tests/source_tree.h), so it is never read.

namespace {

// How many code lines in a row count as a copy. The user's decision is D82
// item 1 in docs/audit/2026-10-03-fix-decisions.md; ADR 0025 says why.
constexpr int kCloneWindowLines = 8;

// One line the clone scan compares, and where it sits in its file.
struct CodeLine {
    std::string text;  // as clone_line_text gives it
    int lineno;        // 1-based, in the file as written
};

// A file as the clone scan sees it: only the lines that count.
struct CodeFile {
    std::string file;  // repo-relative, forward slashes
    std::vector<CodeLine> lines;
};

// A letter, digit or underscore, or any byte of a UTF-8 character. A line
// with none of these is only braces and punctuation.
bool is_word_byte(char c) {
    const unsigned char u = static_cast<unsigned char>(c);
    return (u >= 'a' && u <= 'z') || (u >= 'A' && u <= 'Z') || (u >= '0' && u <= '9') ||
           u == '_' || u >= 0x80;
}

// The text the clone scan compares for one line of source, or empty when the
// line doesn't count: a blank line, a comment-only line, an #include, or a
// line of only braces and punctuation. Ends are trimmed and every inner run of
// whitespace becomes one space. A comment line is one is_line_comment names.
// The clone scan also skips /* */ comments, which the row scans read as code,
// because comment text pasted twice is not a rule written twice; in_comment
// carries such a comment from one line to the next.
std::string clone_line_text(std::string_view line, bool& in_comment) {
    std::string_view t = hydra::trim_view(line);
    if (in_comment) {
        const size_t end = t.find("*/");
        if (end == std::string_view::npos) return {};
        in_comment = false;
        t = hydra::trim_view(t.substr(end + 2));
    }
    if (hydra::starts_with(t, "/*")) {
        const size_t end = t.find("*/", 2);
        if (end == std::string_view::npos) {
            in_comment = true;
            return {};
        }
        t = hydra::trim_view(t.substr(end + 2));
    }
    if (is_line_comment(t) || hydra::starts_with(t, "#include")) return {};
    if (std::none_of(t.begin(), t.end(), is_word_byte)) return {};
    std::string out;
    bool gap = false;
    for (const char c : t) {
        if (hydra::is_ascii_space(c)) {
            gap = true;
            continue;
        }
        if (gap) out += ' ';
        gap = false;
        out += c;
    }
    return out;
}

CodeFile read_code_lines(const std::string& rel, std::istream& in) {
    CodeFile f{rel, {}};
    std::string line;
    int lineno = 0;
    bool in_comment = false;
    while (std::getline(in, line)) {
        ++lineno;
        std::string t = clone_line_text(line, in_comment);
        if (!t.empty()) f.lines.push_back({std::move(t), lineno});
    }
    return f;
}

// One place a copied block sits.
struct CloneSide {
    std::string file;
    int first_line;  // as written, first and last code line of the block
    int last_line;
};

// A block of code found in two places. `a` comes before `b` in the files'
// order, or earlier in the file when a file repeats itself.
struct CloneRegion {
    CloneSide a, b;
    int lines;          // code lines in the block, as clone_line_text counts them
    std::string first;  // the block's first code line
};

// Every block of at least `window` code lines that appears in two places in
// `files`. A block found in three places is three pairs. Two runs in one file
// that overlap each other are not a copy.
std::vector<CloneRegion> find_clones(const std::vector<CodeFile>& files, int window) {
    // Each distinct line becomes a number, so a run's key is its numbers'
    // bytes and the map's own hash does the rest.
    std::unordered_map<std::string, int> ids;
    std::vector<std::vector<int>> coded(files.size());
    for (size_t f = 0; f < files.size(); ++f)
        for (const CodeLine& l : files[f].lines)
            coded[f].push_back(ids.emplace(l.text, static_cast<int>(ids.size())).first->second);
    std::unordered_map<std::string, std::vector<std::pair<int, int>>> runs;  // key -> (file, start)
    for (size_t f = 0; f < files.size(); ++f) {
        const std::vector<int>& c = coded[f];
        for (size_t i = 0; i + static_cast<size_t>(window) <= c.size(); ++i) {
            const std::string key(reinterpret_cast<const char*>(c.data() + i),
                                  sizeof(int) * static_cast<size_t>(window));
            runs[key].push_back({static_cast<int>(f), static_cast<int>(i)});
        }
    }
    // Each matching pair of runs, the earlier place first. Runs went in by
    // file, then by start, so each list is already in that order.
    struct Pair {
        int fa, ia, fb, ib;
    };
    std::vector<Pair> pairs;
    for (const auto& [key, at] : runs)
        for (size_t x = 0; x < at.size(); ++x)
            for (size_t y = x + 1; y < at.size(); ++y) {
                const auto [fa, ia] = at[x];
                const auto [fb, ib] = at[y];
                if (fa == fb && ib < ia + window) continue;
                pairs.push_back({fa, ia, fb, ib});
            }
    // Pairs of runs that each step one line further in both places are one
    // block: sort them so those follow each other.
    const auto order = [](const Pair& p) { return std::make_tuple(p.fa, p.fb, p.ib - p.ia, p.ia); };
    std::sort(pairs.begin(), pairs.end(),
              [&](const Pair& x, const Pair& y) { return order(x) < order(y); });
    std::vector<CloneRegion> out;
    for (size_t s = 0; s < pairs.size();) {
        size_t e = s;
        while (e + 1 < pairs.size() && pairs[e + 1].fa == pairs[s].fa &&
               pairs[e + 1].fb == pairs[s].fb &&
               pairs[e + 1].ib - pairs[e + 1].ia == pairs[s].ib - pairs[s].ia &&
               pairs[e + 1].ia == pairs[e].ia + 1)
            ++e;
        const Pair& p = pairs[s];
        const int last = pairs[e].ia - p.ia + window - 1;  // the block's last line, from its start
        const std::vector<CodeLine>& la = files[p.fa].lines;
        const std::vector<CodeLine>& lb = files[p.fb].lines;
        out.push_back({{files[p.fa].file, la[p.ia].lineno, la[p.ia + last].lineno},
                       {files[p.fb].file, lb[p.ib].lineno, lb[p.ib + last].lineno},
                       last + 1,
                       la[p.ia].text});
        s = e + 1;
    }
    return out;
}

// A copy the scan tolerates for now. The key survives edits elsewhere in
// either file: the two files, the block's length and its first line. Line
// numbers are not part of it.
struct KnownClone {
    std::string file_a;  // the earlier file in sorted path order
    std::string file_b;  // the same as file_a when a file repeats itself
    int lines;           // code lines, as clone_line_text counts them
    std::string first;   // the block's first code line, as clone_line_text gives it
};

// The copies found when the scan arrived (2026-10-05). Each is left for a
// later fix; removing one means removing its entry here.
const std::vector<KnownClone>& known_clones() {
    static const std::vector<KnownClone> k = {
        {"src/app/dm_report.cpp", "src/app/fill_report.cpp", 8, R"x(</select>)x"},
        {"src/app/dm_report.cpp", "src/app/fill_report.cpp", 9, R"x(<div class="sub">__SUBTITLE__</div>)x"},
        {"src/app/dm_report.cpp", "src/app/report.cpp", 9, R"x(<div class="sub">__SUBTITLE__</div>)x"},
        {"src/app/dynamics_breakdown.cpp", "tests/test_dynamics_breakdown.cpp", 9, R"x({DynamicsRow::RedSnare, NoteColor::Red, false, false},)x"},
        {"src/app/fill_report.cpp", "src/app/report.cpp", 9, R"x(<div class="sub">__SUBTITLE__</div>)x"},
        {"src/audio/ma_reader.cpp", "src/audio/vorbis_reader.cpp", 9, R"x(pos_ += done;)x"},
        {"src/cli/fillcompare.cpp", "src/cli/report.cpp", 9, R"x(return 1;)x"},
        {"tests/test_app_state.cpp", "tests/test_song_panel_state.cpp", 11, R"x(char hash[32];)x"},
        {"tests/test_audio_player.cpp", "tests/test_preview_transport.cpp", 10, R"x(namespace {)x"},
        {"tests/test_highway_draw.cpp", "tests/test_track_state.cpp", 9, R"x(SongTiming timing(480, {{0, 1920}}, {{0, 300.0}});)x"},
        {"tests/test_highway_draw.cpp", "tests/test_track_state.cpp", 8, R"x(using namespace hydra;)x"},
        {"tests/test_highway_draw.cpp", "tests/test_track_state.cpp", 10, R"x(return s;)x"},
        {"tests/test_path_view.cpp", "tests/test_path_view.cpp", 12, R"x(HydraRecord rec;)x"},
        {"tests/test_path_view.cpp", "tests/test_preview_view.cpp", 8, R"x(const AnalysisResult& analyzed() {)x"},
        {"tests/test_preview_controller.cpp", "tests/test_preview_controller.cpp", 11, R"x(using namespace hydra;)x"},
        {"tests/test_replay.cpp", "tests/test_replay.cpp", 10, R"x(int charts = 0, paths = 0, mismatches = 0;)x"},
        {"tests/test_replay.cpp", "tests/test_replay.cpp", 10, R"x(Song song(192);)x"},
        {"tests/test_search.cpp", "tests/test_search.cpp", 17, R"x(for (const std::string& path : corpus::chart_paths()) {)x"},
        {"tests/test_search.cpp", "tests/test_search.cpp", 10, R"x(Song song = build_tail_song({{0, true, false},)x"},
        {"tests/test_search.cpp", "tests/test_search.cpp", 9, R"x({768, true, false},)x"},
        {"tests/test_store.cpp", "tests/test_store.cpp", 8, R"x(for (const std::string& path : corpus::chart_paths()) {)x"},
        {"tests/ui/uitest_batch_reports.cpp", "tests/ui/uitest_batch_reports.cpp", 9, R"x(Harness& h = harness(ctx);)x"},
    };
    return k;
}

// The question every clone entry answers, so the baseline goes through
// take_listed_line like every other listed line.
const char* const kCloneQuestion = "Is this block of code written in two places?";

ListedLine clone_listed_line(const std::string& file_a, const std::string& file_b, int lines,
                             const std::string& first) {
    return {kCloneQuestion, file_a + " and " + file_b,
            std::to_string(lines) + " lines from: " + first,
            "baseline clone no longer found (remove it)"};
}

// Every .cpp and .h the walk finds, in sorted path order.
std::vector<CodeFile> read_source_code() {
    std::vector<CodeFile> files;
    sourcetree::for_each_source_file([&](const fs::path& path, const std::string& rel) {
        if (!is_cpp_source(path)) return;
        std::ifstream in(path);
        files.push_back(read_code_lines(rel, in));
    });
    std::sort(files.begin(), files.end(),
              [](const CodeFile& x, const CodeFile& y) { return x.file < y.file; });
    return files;
}

}  // namespace

TEST_CASE("single-owner: the clone scan matches its own examples (MR1)") {
    bool in_comment = false;
    CHECK(clone_line_text("  int  a =\t1;  ", in_comment) == "int a = 1;");
    CHECK(clone_line_text("   ", in_comment).empty());
    CHECK(clone_line_text("   // a note", in_comment).empty());
    CHECK(clone_line_text("#include <vector>", in_comment).empty());
    CHECK(clone_line_text("});", in_comment).empty());
    CHECK(clone_line_text("x = 0;  // kept", in_comment) == "x = 0; // kept");
    CHECK(clone_line_text("/* opens", in_comment).empty());
    CHECK(in_comment);
    CHECK(clone_line_text("int inside = 1;", in_comment).empty());
    CHECK(clone_line_text("closes */ int after = 2;", in_comment) == "int after = 2;");
    CHECK_FALSE(in_comment);

    // Made-up files. `block` is ten different code lines; `seven` is its
    // first seven.
    std::string block, seven;
    for (int i = 0; i < 10; ++i) {
        const std::string line = "v" + std::to_string(i) + " = f(" + std::to_string(i) + ");\n";
        block += line;
        if (i < 7) seven += line;
    }
    const auto file = [](const std::string& rel, const std::string& text) {
        std::istringstream in(text);
        return read_code_lines(rel, in);
    };

    // Ten shared lines are one block, named once with both places. Blank
    // lines, comments and braces around one copy don't hide it.
    std::vector<CloneRegion> r = find_clones(
        {file("src/a.cpp", "only_a();\n" + block),
         file("src/b.cpp", "{\n\n// note\n" + block + "}\n")},
        8);
    REQUIRE(r.size() == 1);
    CHECK(r[0].lines == 10);
    CHECK(r[0].first == "v0 = f(0);");
    CHECK(r[0].a.file == "src/a.cpp");
    CHECK(r[0].a.first_line == 2);
    CHECK(r[0].a.last_line == 11);
    CHECK(r[0].b.file == "src/b.cpp");
    CHECK(r[0].b.first_line == 4);
    CHECK(r[0].b.last_line == 13);

    // Seven shared lines are shorter than a window of 8.
    CHECK(find_clones({file("src/a.cpp", seven + "x();\n"), file("src/b.cpp", "y();\n" + seven)}, 8)
              .empty());

    // A file that repeats its own block is a copy too.
    r = find_clones({file("src/a.cpp", block + "mid();\n" + block)}, 8);
    REQUIRE(r.size() == 1);
    CHECK(r[0].a.file == r[0].b.file);
    CHECK(r[0].a.first_line == 1);
    CHECK(r[0].b.first_line == 12);

    // Nine copies of one line overlap themselves: no copy.
    std::string same;
    for (int i = 0; i < 9; ++i) same += "x++;\n";
    CHECK(find_clones({file("src/a.cpp", same)}, 8).empty());

    // A block in three places is three pairs.
    CHECK(find_clones({file("src/a.cpp", block), file("src/b.cpp", block), file("src/c.cpp", block)}, 8)
              .size() == 3);
}

TEST_CASE("single-owner: no block of code is written in two places (MR1)") {
    const std::vector<CodeFile> files = read_source_code();
    CHECK(files.size() > 100);  // the scan found the sources
    std::vector<ListedLine> listed;
    for (const KnownClone& k : known_clones())
        listed.push_back(clone_listed_line(k.file_a, k.file_b, k.lines, k.first));
    std::vector<bool> used(listed.size(), false);
    std::vector<std::string> problems;
    for (const CloneRegion& c : find_clones(files, kCloneWindowLines)) {
        const ListedLine l = clone_listed_line(c.a.file, c.b.file, c.lines, c.first);
        if (take_listed_line(listed, used, l.question, l.file, l.line_text) == Take::taken)
            continue;
        problems.push_back(c.a.file + ":" + std::to_string(c.a.first_line) + "-" +
                           std::to_string(c.a.last_line) + " and " + c.b.file + ":" +
                           std::to_string(c.b.first_line) + "-" + std::to_string(c.b.last_line) +
                           " hold the same " + std::to_string(c.lines) +
                           " code lines, from: " + c.first +
                           " (move them into one place both can call)");
    }
    check_scan(problems, listed, used);
}
