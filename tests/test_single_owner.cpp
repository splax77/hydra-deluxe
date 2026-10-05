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
// from tests/source_tree.h.
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
          "return core::sqout_position(bsq.timecode.ticks(), sqout_tick) == core::SqOutPosition::Exact;"}},
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
        {"Which teal is the hover colour for frames and headers?",
         "kFrameHoveredColor in src/ui/theme.h",
         R"(IM_COL32\(\s*0\s*,\s*100\s*,\s*100\b|\b0(\s*/\s*255\.0f?)?\s*,\s*100\s*/\s*255\.0f?\s*,\s*100\s*/\s*255\b)",
         "",
         {},
         {},
         "audit finding 218, the code half (phase 3 task C4b)",
         {"colors[ImGuiCol_FrameBgHovered] = ImVec4(0, 100 / 255.0f, 100 / 255.0f, 1.0f);",
          "colors[ImGuiCol_HeaderHovered] = ImVec4(0, 100 / 255.0f, 100 / 255.0f, 1.0f);"},
         {"colors[ImGuiCol_FrameBgHovered] = kFrameHoveredColor;",
          "inline const ImVec4 kButtonHoveredColor{0 / 255.0f, 104 / 255.0f, 104 / 255.0f, 1.0f};",
          "IM_COL32(200, 100, 100, 255)"},
         {{"src/ui/theme.h",
           "inline const ImVec4 kFrameHoveredColor{0 / 255.0f, 100 / 255.0f, 100 / 255.0f, 1.0f};",
           "kFrameHoveredColor, the owner"}}},
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
         "M_D review finding 3 and round 2 finding 5 (phase 3 tasks FX-R and FX2-R)",
         {"options.lens.legacy_fills ? FillDeadlineRule::Ch10 : FillDeadlineRule::Ch11;",
          "return legacy_fills ? FillDeadlineRule::Ch10 : FillDeadlineRule::Ch11;"},
         {"fill_rule_for(settings.legacy_fill_deadline), settings.rules);"},
         {{"src/search/graph.h",
           "return legacy_fills ? FillDeadlineRule::Ch10 : FillDeadlineRule::Ch11;",
           "fill_rule_for, the owner"}}},
        // A database's engine_mode stamp compared to a rule's spelling by
        // hand, through the store's accessor or a stamp held in `mode`.
        {"Which fill rule does a database's stamp name?",
         "fill_rule_from_stamp in src/search/graph.h",
         R"(engine_mode\(\)\s*==|\*?\bmode\s*==\s*"ch1[01]")",
         R"(\bfill_rule_from_stamp\()",
         {},
         {},
         "M_D review round 2 findings 2 and 5 (phase 3 task FX2-R)",
         {"if (store->engine_mode() == std::string(",
          "const std::string legacy = mode && *mode == \"ch10\" ? \"1\" : \"0\";"},
         {"if (mode && hydra::fill_rule_from_stamp(*mode) != expected)"}},
        // ---- one cleaned song title (phase 3 task O3a) ----
        // Clone Hero's rich-text tags spelled as text: a tag in angle
        // brackets at the start of a string, a tag name kept in a named
        // constant, or a row of a tag table. Config keys that happen to be
        // called "color" or "size" are not flagged.
        {"Which tags does Hydra strip from a song name?",
         "strip_rich_tags in src/parse/song.cpp, read through display_title "
         "(and display_artist, which forwards to it, D50 item 5)",
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
         R"(another Hydra version|different rules in hydra_rules|Re-analyze to refresh)",
         "",
         {},
         {},
         "audit finding 13; D48, Q17 (phase 3 task O3b)",
         {"hint(\"Analyzed by another Hydra version, or under different rules in \"",
          "\"from different rules in hydra_rules.ini. Re-analyze to refresh it.\");",
          "\"hydra_rules.ini. Re-analyze to refresh.\");"},
         {"} else if (status == store::RecordStatus::Stale) {",
          "\"A saved result couldn't be read. Re-analyze this song to replace it.\";",
          "\"hydra_rules.ini has a line Hydra can't read. Fix or delete that line, then restart \""},
         {{"src/app/user_messages.cpp",
           "cause = \"another Hydra version or from different rules in hydra_rules.ini\";",
           "stale_text, the owner: both causes (today's sentence)"},
          {"src/app/user_messages.cpp", "cause = \"another Hydra version\";",
           "stale_text, the owner: another Hydra version"},
          {"src/app/user_messages.cpp", "cause = \"different rules in hydra_rules.ini\";",
           "stale_text, the owner: different rules"},
          {"src/app/user_messages.cpp",
           "return \"Out of date: this result came from \" + cause + \". Re-analyze to refresh it.\";",
           "stale_text, the owner: the sentence's frame"}}},
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
        // The last timestamp's onset, or the last drawn note's, read as the
        // song's length anywhere but store::song_length_ms.
        {"When is the song's last note?",
         "store::song_length_ms in src/store/record_store.cpp",
         R"(notes\.back\(\)\.ms\b|sequence\.back\(\)\.timecode\.ms\(\))",
         "",
         {},
         {},
         "audit finding 9; derive-once review of M_D, preview finding 1 (phase 3 task FX-P)",
         {"if (scene.has_notes) scene.song_length_ms = scene.notes.back().ms;",
          "return song.sequence.back().timecode.ms();"},
         {"scene.song_length_ms = store::song_length_ms(song).value_or(0.0);",
          "const int64_t last_tick = scene.notes.back().tick;"},
         {{"src/store/record_store.cpp", "return song.sequence.back().timecode.ms();",
           "store::song_length_ms, the owner"}}},
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
         {"if (a.has_sp_end && struck_at(now, a.ms) && now < a.sp_end_ms) return &a;",
          "return struck_at(now_ms, s.ms);", "if (now < a.sp_end_ms) return &a;"},
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
         "chart_ids in src/app/report.cpp",
         R"(songs\.insert\(\s*r\.hyhash)",
         "",
         {},
         {},
         "audit finding 242; phase 6 task J2-2 (D53)",
         {"songs.insert(r.hyhash);"},
         {"out.songs = static_cast<int64_t>(chart_ids(rows).size());"},
         {},
         {"src"}},
        // collect_dm_rows and collect_fill_rows set each row's status from
        // the one comparison; the page scripts read it instead of testing the
        // delta's sign. The C++ status assignments carry no `r.` and are the
        // owner.
        {"Which side of a report comparison is higher?",
         "the status field set by collect_dm_rows and collect_fill_rows",
         R"(r\.delta\s*[<>]\s*0\s*\?)",
         "",
         {},
         {},
         "audit finding 168; phase 6 task J2-2 (D53)",
         {"const deltaCls = (noDelta || r.status === 'other speed') ? 'num dim' : (r.delta < 0 ? 'num neg' : 'num');",
          ": (r.delta < 0 ? '+' + fmt(-r.delta) + ' over' : fmt(r.delta));",
          "const left = under.reduce((a, r) => a + (r.delta > 0 ? r.delta : 0), 0);",
          "const deltaCls = !hasDelta ? 'num dim' : (r.delta > 0 ? 'num pos'"},
         {R"(row.status = s.score > opt    ? "above optimal")",
          R"(row.status = delta == 0 ? "same" : (delta > 0 ? "1.1 higher" : "1.0 higher");)",
          "const deltaCls = (noDelta || r.status === 'other speed') ? 'num dim' : (r.status === 'above optimal' ? 'num neg' : 'num');"},
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
        // Walking the corpus for the first charts that analyze to a path.
        {"Which corpus chart is the first with paths?",
         "analyzed_with_paths in tests/corpus_util.h",
         R"(\.record\.paths\.empty\(\)\)\s*continue)",
         "",
         {"tests/corpus_util.h"},
         {},
         "audit finding 277; phase 6 task J2-2 (D53)",
         {"if (result.song.is_empty() || result.record.paths.empty()) continue;",
          "if (r.song.is_empty() || r.record.paths.empty()) continue;"},
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
    };
    return r;
}

const std::vector<KnownCopy>& known_copies() {
    static const std::vector<KnownCopy> k = {
        // A one-time schema 2 to 3 migration reads the stamp text old files
        // already hold, so it keeps the literal "ch10" even if
        // engine_mode_stamp were ever spelled differently, and src/store never
        // includes src/search. test_store.cpp seeds the migration test's
        // stamp with engine_mode_stamp, so a respelling turns that test red.
        {"Which fill rule does a database's stamp name?", "src/store/record_store.cpp",
         "legacy_fills = mode && *mode == \"ch10\" ? \"1\" : \"0\";",
         "never: a migration reads the historic stamp text, and store never includes search "
         "(M_D review round 2)"},
        {"Is this row the squeezed-out chord, or past it?", "src/core/model.cpp",
         "return sqout_tick.has_value() && bsq.timecode.ticks() > *sqout_tick;",
         "display_backends' trim (audit finding 146, another step)"},
        {"Which test helper writes MThd/MTrk chunks?", "tests/test_song.cpp",
         "const char* tag = \"MTrk\";",
         "test_song.cpp's put_track and put_varlen move to tests/midi_util.h (audit finding 286)"},
        {"Which test helper writes MThd/MTrk chunks?", "tests/test_song.cpp",
         "put_bytes(file, {'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 1, 0, 3, 0, 192});",
         "test_song.cpp's put_track and put_varlen move to tests/midi_util.h (audit finding 286)"},
        {"How long does a UI confirmation stay, and how often does the UI re-check?",
         "src/ui/library_toolbar.cpp",
         "if (!app.status_is_problem && ImGui::GetTime() - shown_at > 6.0) return;",
         "finding 219, not yet scheduled"},
        {"What range may a number setting hold?", "src/ui/settings_bar.cpp",
         "app.settings.sp_cap = std::max(1, cap);", "task SE2 (the boxes call Settings::clamp)"},
        {"What range may a number setting hold?", "src/ui/settings_bar.cpp",
         "app.settings.mslimit_value = std::clamp(app.settings.mslimit_value, -window, window);",
         "task SE2 (the boxes call Settings::clamp)"},
        {"What range may a number setting hold?", "src/ui/paths_tab.cpp",
         "app.settings.backendlimit_value = std::clamp(app.settings.backendlimit_value, 0,",
         "task SE2 (the boxes call Settings::clamp)"},
        {"What range may a number setting hold?", "src/ui/preview_controller.cpp",
         "volume_pct_ = percent < 0 ? 0 : percent > 100 ? 100 : percent;",
         "task PV (the volume reads through Settings::clamp)"},
        {"What range may a number setting hold?", "src/ui/preview_tab.cpp",
         "const int cap = std::max(1, pc->sp_meter_cap());", "task PV (the Preview's cap floors)"},
        {"What range may a number setting hold?", "src/app/preview_view.cpp",
         "curve.cap = sp_cap < 1 ? 1 : sp_cap;", "task PV (the Preview's cap floors)"},
        {"What range may a number setting hold?", "src/app/preview_view.cpp",
         "curve.cap = sp_cap < 1 ? 1 : sp_cap;", "task PV (the Preview's cap floors)"},
        {"What gain does a volume percent play at?", "src/ui/preview_controller.cpp",
         "transport_.set_gain(static_cast<float>(volume_pct_) / 100.0f);",
         "task PV (the Preview calls Settings::volume_gain)"},
        {"What gain does a volume percent play at?", "src/ui/preview_controller.cpp",
         "transport_.set_gain(static_cast<float>(volume_pct_) / 100.0f);",
         "task PV (the Preview calls Settings::volume_gain)"},
        {"Is a span on after this instant?", "src/render/highway_draw.cpp",
         "bool toggle_on(Toggle t) { return t != Toggle::Empty && t != Toggle::End; }",
         "J2-8 (the draw code calls toggle_on_after)"},
        {"Which pad colours a fill lane?", "src/render/highway_draw.cpp",
         "if (inst.t >= s.t1 && inst.t <= s.t2 && inst.fill_lane_pad) { pad = inst.fill_lane_pad; break; }",
         "J2-8 (the draw code reads the pad off make_lane_bounds)"},
        {"Which pad colours a fill lane?", "src/render/highway_draw.cpp",
         "if (inst.fill_lane_pad) { pad = inst.fill_lane_pad; break; }",
         "J2-8 (the draw code reads the pad off make_lane_bounds)"},
        {"Which notes file wins when a song has both?", "src/parse/song.cpp",
         "const ChartFormat f = notes_file_format(e.name);",
         "task J2-3 (the .sng loader calls pick_notes_file; audit finding 186)"},
        {"Where does an .srb's metadata stream start?", "src/parse/song.cpp",
         "srb_inflate_stream_reading(src, kSrbHeaderSize, kSrbMaxMetadata, &notes_offset);",
         "task J2-3 (the .srb loader calls srb_read_metadata; audit finding 188)"},
        {"Where does an .srb's metadata stream start?", "src/app/preview_source.cpp",
         "srb_inflate_stream(buf.data(), buf.size(), kSrbHeaderSize,",
         "task J2-5 (the Preview's audio walk starts at srb_read_metadata's offset; audit "
         "finding 188)"},
        {"Which file is the folder's song.ini?", "src/app/preview_source.cpp",
         R"(if (!e.is_dir && is_song_ini(e.name)) return folder + "\\" + e.name;)",
         "task J2-5 (the Preview's private find_song_ini goes; audit finding 189)"},
        {"What is a path's parent folder?", "src/app/config.cpp",
         R"(return pos == std::string::npos ? std::string(".") : path.substr(0, pos);)",
         "task J2-1 (exe_dir calls parent_folder; audit finding 251)"},
        {"What is a path's parent folder?", "src/app/preview_source.cpp",
         R"(return slash == std::string::npos ? std::string(".") : path.substr(0, slash);)",
         "task J2-5 (dir_name goes; audit finding 251)"},
        {"Which test helper builds a .sng?", "tests/test_preview_source.cpp",
         "std::vector<uint8_t> make_sng(",
         "task J2-5 (test_preview_source.cpp uses tests/sng_util.h; audit finding 117)"},
        {"How does a scan row become a library entry?", "src/ui/library_jobs.cpp",
         "entries.push_back({item.md5, item.title, item.artist, item.charter, item.notespath,",
         "task J2-4 (ScanJob::run calls to_library_entry; audit finding 256)"},
        {"How does a scan row become a library entry?", "tools/bench.cpp",
         "entries.push_back({it.md5, it.title, it.artist, it.charter, it.notespath,",
         "task J2-6 (scan_mode calls to_library_entry; audit finding 256)"},
        {"How many workers does a batch get?", "src/ui/library_jobs.cpp",
         "workers_ = std::max(1, workers);",
         "task J2-4 (set_analyzer_for_test drops its floor; audit R7.22)"},
        {"How are a folder and a file name joined?", "src/app/preview_source.cpp",
         R"(if (!e.is_dir && is_song_ini(e.name)) return folder + "\\" + e.name;)",
         "task J2-5 (the Preview calls the join owner; review of M6-J1a finding 1)"},
        {"How are a folder and a file name joined?", "src/app/preview_source.cpp",
         R"(s.path = folder + "\\" + e.name;)",
         "task J2-5 (the Preview calls the join owner; review of M6-J1a finding 1)"},
        {"What fields does a phrase-end note carry in a hand-built Song?",
         "tests/test_preview_view.cpp", "ts.flag_sp = true;",
         "task J3-4 (test_preview_view's hand-built songs call mark_phrase_end)"},
        {"What fields does a phrase-end note carry in a hand-built Song?",
         "tests/test_preview_view.cpp", "ts.flag_sp = true;",
         "task J3-4 (test_preview_view's hand-built songs call mark_phrase_end)"},
        {"What fields does a phrase-end note carry in a hand-built Song?",
         "tests/test_preview_view.cpp", "ts.flag_sp = n.phrase;",
         "task J3-4 (test_preview_view's hand-built songs call mark_phrase_end)"},
        {"What fields does a phrase-end note carry in a hand-built Song?",
         "tests/test_preview_view.cpp", "ts.flag_sp = true;",
         "task J3-4 (test_preview_view's hand-built songs call mark_phrase_end)"},
        // An off-speed score has status "other speed", so the status cannot
        // say whether it beat Hydra's optimal; today such a row still reads
        // "+N over". Keying the text on the status would change that row.
        {"Which side of a report comparison is higher?", "src/app/dm_report.cpp",
         ": (r.delta < 0 ? '+' + fmt(-r.delta) + ' over' : fmt(r.delta));",
         "waits on the user: should an off-speed score above optimal keep reading "
         "\"+N over\" (J2-2 report)"},
        {"Which fill rule does a record key name?", "src/store/record_store.cpp",
         "if (key.lens.legacy_fills != (record.legacy_fills ? 1 : 0))",
         "task J3-6 (prepare_row reads the record's fill flag through Lens::from)"},
        {"Which corpus chart is the first with paths?", "tests/test_path_view.cpp",
         "if (r.record.paths.empty()) continue;",
         "task J4-4 (the squeezed-out search walks corpus::analyzed_with_paths)"},
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
            const fs::path ext = path.extension();
            if (ext != ".cpp" && ext != ".h") return;
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
                const bool comment = t.compare(0, 2, "//") == 0;
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
    for (size_t i = 0; i < listed.size(); ++i) {
        if (used[i]) continue;
        problems.push_back(listed[i].stale + ": " + listed[i].file + ": " + listed[i].line_text);
    }
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
    std::ostringstream report;
    for (const std::string& p : problems) report << p << "\n";
    INFO(report.str());
    CHECK(problems.empty());
}

// D23: a change under src/parse that alters what a chart reads as must bump
// the results stamp. The rule is a comment, which does not compile and which
// the row scan skips, so this case reads it: the block between the "Results"
// banner and the kResultsStamp line in src/store/stored_versions.h must name
// src/parse next to src/search and src/core. Moved here from
// test_s2_stamps.cpp; the repo root comes from tests/source_tree.h.
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

// ST1 (findings 116 and 132): in record_store.cpp a stored row becomes a
// record only through decode_record, which sets the fill rule. It is one
// code line in the file; comment lines are skipped like the row scan does.
// The best path's text has its own row above ("What text is a record's best
// path?").
TEST_CASE("single-owner: record_store.cpp decodes a row once (ST1)") {
    std::ifstream in(sourcetree::root() / "src" / "store" / "record_store.cpp");
    REQUIRE(in.good());
    int decodes = 0;
    std::string line;
    while (std::getline(in, line)) {
        const std::string t = hydra::trim(line);
        if (t.compare(0, 2, "//") == 0) continue;
        if (t.find("rebuild_record(") != std::string::npos) ++decodes;
    }
    CHECK(decodes == 1);
}
