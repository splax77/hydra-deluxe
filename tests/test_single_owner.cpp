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
// is a new row, not a new walker.
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
        // which audit R7.12 calls a different question. The limit may be
        // spelled 260, and either side may carry a little arithmetic
        // (size() - 4 < MAX_PATH). winstr.cpp is not exempt as a file: only
        // shell_path's two lines are listed.
        {"Does the Windows shell take a path this long?",
         "shell_path in src/core/winstr.cpp",
         R"((\.(size|length)\(\)|\b(wcs|str)len\s*\([^()]*\))(\s*[-+]\s*[\w.]+)*\s*[<>]=?\s*(MAX_PATH|260)\b|\b(MAX_PATH|260)(\s*[-+]\s*[\w.]+)*\s*[<>]=?\s*[\w.:>()-]*(\.(size|length)\(\)|\b(wcs|str)len\s*\())",
         "",
         {},
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
          "if (n > 260) return;", "if (rows.size() > 2600) return;"},
         {{"src/core/winstr.cpp", "if (path.size() < MAX_PATH) return path;",
           "shell_path, the owner: a short path goes to the shell as it is"},
          {"src/core/winstr.cpp", "return s.size() < MAX_PATH ? s : L\"\";",
           "shell_path, the owner: a short form still too long is refused"}}},
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
         "this file, tests/test_single_owner.cpp",
         R"(recursive_directory_iterator\(root\s*/\s*sub\))",
         "",
         {},
         {},
         "this file's own rule (one scan, new rules are rows); step-1 derive-once review "
         "finding 12 (2026-10-04)",
         {"for (const fs::directory_entry& e : fs::recursive_directory_iterator(root / sub)) {",
          "for (const auto& e : fs::recursive_directory_iterator(root/sub))"},
         {"for (const auto& e : std::filesystem::directory_iterator(dir)) {"},
         {},
         {"tests"}},
        {"Which test reads the source tree?",
         "this file, tests/test_single_owner.cpp",
         R"(\bHYDRA_SOURCE_DIR\b)",
         "",
         {},
         {},
         "this file's own rule (one scan, new rules are rows); step-1 derive-once review of "
         "fb1189b, finding 5 (2026-10-04)",
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
         R"(==\s*1\s*\?\s*("|one\b)|%\w+ bars?\b|" bars?")",
         R"(\b(counted|has_have)\()",
         {},
         {},
         "D48, Q12 (audit finding 14; phase 3 task O1)",
         {"return group_thousands(n) + \" \" + (n == 1 ? one : many);",
          "view.lines.push_back(\"SP cap:  \" + std::to_string(*record.sp_cap) + \" bars\");",
          "std::printf(\"SP cap     : %d bars\\n\", settings.sp_cap);"},
         {"view.lines.push_back(\"Path limit:  off\");",
          "s.depth_mode = depth_mode == 1 ? DepthMode::Points : DepthMode::Scores;",
          "ImGui::TextUnformatted(\"bars\");"},
         {{"src/core/model.cpp", "return group_thousands(n) + \" \" + (n == 1 ? one : many);",
           "counted, the owner"},
          {"src/core/model.cpp", "return n == 1 ? \"has\" : \"have\";", "has_have, the owner"}}},
        // A timing printed as a whole number of ms: printf's %.0f, or a cast
        // or lround with " ms" after it.
        {"How is a timing written in whole ms?",
         "format_ms_whole in src/core/model.cpp",
         R"(%\.0f ms|static_cast<(long long|long|int|int64_t)>\([^;]*\)\)\s*\+\s*" ms"|\blround\([^;]*\+\s*" ms")",
         R"(\bformat_ms_whole\()",
         {},
         {{"src/core/replay.cpp", "it prints the fixed 500 ms squeeze window, not a timing"},
          {"tools/replay.cpp", "it prints the fixed 500 ms squeeze window, not a timing"}},
         "D48, Q2 (audit findings 4 and 18; phase 3 task O1)",
         {"std::snprintf(buf, sizeof(buf), \"%s %.0f ms\", what, *hardest);",
          "std::to_string(static_cast<long long>(sq.difficulty())) + \" ms\");"},
         {"std::snprintf(buf, sizeof(buf), \"%.1f ms\", ms);"},
         {{"src/core/model.cpp", "return std::to_string(std::lround(ms)) + \" ms\";",
           "format_ms_whole, the owner"}}},
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
        {"Is this phrase chord after the SP end?", "src/core/replay.cpp",
         "const bool past_deact = row.tick > w.deact_tick;",
         "the replay's past_deact (audit findings 1 and 32, another step)"},
        {"Is this phrase chord after the SP end?", "src/core/replay.cpp",
         "} else if (tick > w.deact_tick) {",
         "the replay's past_deact (audit findings 1 and 32, another step)"},
        {"Is this row the squeezed-out chord, or past it?", "src/core/model.cpp",
         "return sqout_tick.has_value() && bsq.timecode.ticks() > *sqout_tick;",
         "display_backends' trim (audit finding 146, another step)"},
        {"Which test helper writes MThd/MTrk chunks?", "tests/test_song.cpp",
         "const char* tag = \"MTrk\";",
         "test_song.cpp's put_track and put_varlen move to tests/midi_util.h (audit finding 286)"},
        {"Which test helper writes MThd/MTrk chunks?", "tests/test_song.cpp",
         "put_bytes(file, {'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 1, 0, 3, 0, 192});",
         "test_song.cpp's put_track and put_varlen move to tests/midi_util.h (audit finding 286)"},
        {"Which gold is Star Power?", "src/ui/preview_tab.cpp",
         "dl->AddText(font, size, ImVec2(origin.x + margin, y), IM_COL32(255, 204, 51, 255),",
         "task K4a (D48, Q26: the next-activation header takes the best-path gold)"},
        {"Which gold is Star Power?", "src/ui/preview_tab.cpp",
         "IM_COL32(255, 204, 51, 255), \"SP\");",
         "task K4a (D48, Q26: the \"SP\" label reads the named gold)"},
        {"Which gold is Star Power?", "src/ui/preview_tab.cpp",
         "IM_COL32(255, 204, 51, 230));  // Star Power gold",
         "task K4a (D48, Q26: the gauge fill reads the named gold)"},
        {"Which gold is Star Power?", "src/ui/preview_tab.cpp",
         "const ImU32 accent = drain.active ? IM_COL32(255, 204, 51, 255)  // SP gold",
         "task K4a (D48, Q26: the drain box turns teal)"},
        {"How long does a UI confirmation stay, and how often does the UI re-check?",
         "src/ui/library_toolbar.cpp",
         "if (!app.status_is_problem && ImGui::GetTime() - shown_at > 6.0) return;",
         "finding 219, not yet scheduled"},
        {"How is a count written next to its noun?", "src/app/report.cpp",
         "return group_thousands(n) + \" \" + (n == 1 ? one : many);",
         "task K1a (D48, Q12: report::counted goes, callers read the core counted)"},
        {"How is a count written next to its noun?", "src/app/report.cpp",
         "std::string cap_label = \"SP cap \" + std::to_string(options.cap.exact) + \" bars\";",
         "task K1a (D48, Q12: the report subtitle's cap reads \"1 bar\" at cap 1)"},
        {"How is a count written next to its noun?", "src/app/path_view.cpp",
         "return std::to_string(bars) + (bars == 1 ? \" bar\" : \" bars\");",
         "task K2 (D48, Q12: bars_text reads counted)"},
        {"How is a count written next to its noun?", "src/app/path_view.cpp",
         "view.lines.push_back(\"SP cap:  \" + std::to_string(*record.sp_cap) + \" bars\");",
         "task K2 (D48, Q12: \"SP cap:  1 bar\" at cap 1)"},
        {"How is a count written next to its noun?", "src/app/path_view.cpp",
         "av.backends_label = std::to_string(shown) + (shown == 1 ? \" note\" : \" notes\") +",
         "task K2 (D48, Q12: the backend notes label reads counted)"},
        {"How is a count written next to its noun?", "src/app/path_view.cpp",
         "const char* unit = points ? (depth_value == 1 ? \" point\" : \" points\")",
         "task K2 (D48, Q12: within_label reads counted)"},
        {"How is a count written next to its noun?", "src/app/path_view.cpp",
         ": (depth_value == 1 ? \" score\" : \" scores\");",
         "task K2 (D48, Q12: within_label reads counted)"},
        {"How is a count written next to its noun?", "src/ui/details_panel.cpp",
         "(*summary.stars == 1 ? \" star\" : \" stars\");",
         "task K2 (D48, Q12: the star count reads counted)"},
        {"How is a count written next to its noun?", "src/ui/library_dialogs.cpp",
         "return group_thousands(n) + \" \" + (n == 1 ? one : many);",
         "task K3 (D48, Q12: count_label goes, callers read counted)"},
        {"How is a count written next to its noun?", "src/ui/library_dialogs.cpp",
         "group_thousands(with) + (with == 1 ? \" that already has\" : \" that already have\") +",
         "task K3 (D48, Q12: the verb reads has_have)"},
        {"How is a count written next to its noun?", "src/ui/library_dialogs.cpp",
         "(without == 1 ? \" that has\" : \" that have\") + \" no result yet?\";",
         "task K3 (D48, Q12: the verb reads has_have)"},
        {"How is a count written next to its noun?", "src/ui/library_table.cpp",
         "return group_thousands(static_cast<int64_t>(n)) + (n == 1 ? \" chart\" : \" charts\");",
         "task K3 (D48, Q12: the chart count reads counted)"},
        {"How is a count written next to its noun?", "src/cli/batch.cpp",
         "std::printf(\"SP cap     : %d bars\\n\", settings.sp_cap);",
         "task K5 (D48, Q12: \"SP cap     : 1 bar\" at cap 1)"},
        {"How is a timing written in whole ms?", "src/app/path_view.cpp",
         "std::snprintf(buf, sizeof(buf), \"%s %.0f ms\", what, *hardest);",
         "task K2 (D48, Q2: the activation badge reads format_ms_whole)"},
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
    const fs::path root = fs::u8path(HYDRA_SOURCE_DIR);
    const std::vector<CompiledRule> compiled = compile_rules();

    const std::vector<ListedLine> listed = listed_lines();
    std::vector<bool> used(listed.size(), false);  // which listed entries a line took
    std::vector<std::string> problems;
    int files = 0;
    std::vector<int> functions_found(compiled.size(), 0);
    for (const std::string sub : {"src", "tools", "tests"}) {
        for (const fs::directory_entry& e : fs::recursive_directory_iterator(root / sub)) {
            const fs::path ext = e.path().extension();
            if (ext != ".cpp" && ext != ".h") continue;
            const std::string rel = fs::relative(e.path(), root).generic_u8string();
            // This file holds every rule's examples as text.
            if (rel == "tests/test_single_owner.cpp") continue;
            ++files;
            std::ifstream in(e.path());
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
        }
    }
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
// test_s2_stamps.cpp, so this file stays the one test that reads the tree.
TEST_CASE("single-owner: the results stamp's bump rule names the chart readers (D23)") {
    std::ifstream in(fs::u8path(HYDRA_SOURCE_DIR) / "src" / "store" / "stored_versions.h");
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
