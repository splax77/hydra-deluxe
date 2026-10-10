// hydra_uitest: run Hydra's GUI tests headlessly. See docs/agents/ui-testing.md.
//
//   hydra_uitest --all                 run every checked-in test
//   hydra_uitest --test <name>         run one (e.g. scan, analyze); repeatable
//   hydra_uitest --script <file>       run a command file
//   hydra_uitest --list                list the tests
//   options: --keep-temp  --shots <dir>  --jobs <n>
//
// --all, or more than one --test, runs each chosen test in its own
// hydra_uitest process, at most --jobs at once (kDefaultJobs when not given),
// and prints their results in registration order. Each process has its own
// scratch folder and a fresh ImGui context, so no test sees another's
// leftovers. A single --test, or a --script, runs in this process: a script
// needs the engine here, and one test is the debugging case.
//
// Prints [PASS]/[FAIL] per test and exits 0 only if everything passed.

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
// dbghelp.h needs windows.h first; only its types are used (the function is
// looked up at crash time, so nothing links against dbghelp).
#include <dbghelp.h>

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include "../wait_util.h"  // tests/ is not on the runner's include path
#include "core/winstr.h"
#include "uitest_harness.h"

namespace {

// A crash (an access violation, say) ends the process with no result line of
// its own. This filter prints one for the test that was running, naming the
// exception, and writes a minidump into the scratch folder (which a crash
// never deletes) so the stack can be read later with cdb or WinDbg.
uitest::Harness* g_crash_harness = nullptr;

LONG WINAPI on_crash(EXCEPTION_POINTERS* info) {
    const uitest::Harness* h = g_crash_harness;
    const ImGuiTest* t = h ? h->running_test() : nullptr;
    const unsigned long code = info->ExceptionRecord->ExceptionCode;
    std::string dump;
    if (h && !h->temp_dir.empty()) {
        dump = h->temp_dir + "\\crash.dmp";
        using WriteDumpFn = BOOL(WINAPI*)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE,
                                          PMINIDUMP_EXCEPTION_INFORMATION,
                                          PMINIDUMP_USER_STREAM_INFORMATION,
                                          PMINIDUMP_CALLBACK_INFORMATION);
        HMODULE dbghelp = LoadLibraryW(L"dbghelp.dll");
        auto write_dump =
            dbghelp ? reinterpret_cast<WriteDumpFn>(GetProcAddress(dbghelp, "MiniDumpWriteDump"))
                    : nullptr;
        HANDLE f = CreateFileW(hydra::utf8_to_wide(dump).c_str(), GENERIC_WRITE, 0, nullptr,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        MINIDUMP_EXCEPTION_INFORMATION mei{GetCurrentThreadId(), info, FALSE};
        const auto type = static_cast<MINIDUMP_TYPE>(MiniDumpWithIndirectlyReferencedMemory |
                                                     MiniDumpWithDataSegs | MiniDumpWithThreadInfo);
        const bool written = f != INVALID_HANDLE_VALUE && write_dump &&
                             write_dump(GetCurrentProcess(), GetCurrentProcessId(), f, type, &mei,
                                        nullptr, nullptr);
        if (f != INVALID_HANDLE_VALUE) CloseHandle(f);
        if (!written) dump.clear();
    }
    std::fflush(stderr);
    std::printf("[FAIL] hydra/%s\n---- log ----\nthe test process crashed: exception 0x%08lX at "
                "%p%s%s\n---- end ----\n",
                t ? t->Name : "(between tests)", code, info->ExceptionRecord->ExceptionAddress,
                dump.empty() ? "" : "; minidump ", dump.c_str());
    std::fflush(stdout);
    return EXCEPTION_EXECUTE_HANDLER;  // end the process with the exception's code
}

int usage() {
    std::fprintf(stderr,
                 "usage: hydra_uitest (--all | --test <name> | --script <file> | --list)"
                 " [--keep-temp] [--shots <dir>] [--jobs <n>] [--db <file>]\n");
    return 2;
}

// One test's child process and the file its output goes to.
struct Child {
    std::string name;
    std::string log;
    HANDLE process = nullptr;
    DWORD exit_code = 1;
    ULONGLONG started_ms = 0;  // GetTickCount64 at launch
    bool timed_out = false;    // ran past testwait::kUitestProcessCap and was killed
};

// Starts `hydra_uitest --test <name>` with its stdout and stderr in c.log.
bool launch_child(Child& c, const std::wstring& exe, const std::vector<std::string>& passthrough) {
    SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};  // the child inherits the file
    HANDLE out = CreateFileW(hydra::utf8_to_wide(c.log).c_str(), GENERIC_WRITE, FILE_SHARE_READ,
                             &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (out == INVALID_HANDLE_VALUE) return false;
    std::wstring cmd = L"\"" + exe + L"\" --test \"" + hydra::utf8_to_wide(c.name) + L"\"";
    for (const std::string& p : passthrough) cmd += L" \"" + hydra::utf8_to_wide(p) + L"\"";
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = out;
    si.hStdError = out;
    PROCESS_INFORMATION pi{};
    const BOOL ok = CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW,
                                   nullptr, nullptr, &si, &pi);
    CloseHandle(out);  // the child holds its own copy
    if (!ok) return false;
    CloseHandle(pi.hThread);
    c.process = pi.hProcess;
    c.started_ms = GetTickCount64();
    return true;
}

// Every chosen test in its own process, at most `jobs` at once.
int run_parallel(uitest::Harness& h, const std::vector<std::string>& wanted, int jobs,
                 const std::vector<std::string>& passthrough) {
    // The chosen tests in registration order, the order --all runs them in.
    ImVector<ImGuiTest*> tests;
    ImGuiTestEngine_GetTestList(h.engine, &tests);
    std::vector<Child> children;
    for (ImGuiTest* t : tests) {
        if (std::strcmp(t->Name, "script") == 0) continue;
        for (const std::string& w : wanted) {
            if (uitest::selects(w, t->Name)) {
                Child c;
                c.name = t->Name;
                c.log = h.temp_dir + "\\" + c.name + ".log";
                children.push_back(c);
                break;
            }
        }
    }
    for (const std::string& w : wanted) {
        bool found = false;
        for (const Child& c : children) found = found || uitest::selects(w, c.name.c_str());
        if (!found) {
            std::fprintf(stderr,
                         "hydra_uitest: no test \"%s\" (a script runs only on its own; try "
                         "--list)\n",
                         w.c_str());
            return 2;
        }
    }

    wchar_t exe[MAX_PATH];
    GetModuleFileNameW(nullptr, exe, MAX_PATH);

    std::vector<size_t> running;
    size_t next = 0, done = 0;
    while (done < children.size()) {
        while (running.size() < static_cast<size_t>(jobs) && next < children.size()) {
            Child& c = children[next];
            if (launch_child(c, exe, passthrough)) {
                running.push_back(next);
            } else {
                std::fprintf(stderr, "hydra_uitest: could not start the process for %s\n",
                             c.name.c_str());
                ++done;
            }
            ++next;
        }
        if (running.empty()) continue;
        // Wait no longer than the oldest running child has left of its cap.
        const ULONGLONG cap_ms = static_cast<ULONGLONG>(testwait::kUitestProcessCap.count());
        const ULONGLONG now = GetTickCount64();
        ULONGLONG wait_ms = cap_ms;
        size_t oldest = 0;
        for (size_t j = 0; j < running.size(); ++j) {
            const ULONGLONG ran = now - children[running[j]].started_ms;
            const ULONGLONG left = ran >= cap_ms ? 0 : cap_ms - ran;
            if (left < wait_ms) {
                wait_ms = left;
                oldest = j;
            }
        }
        std::vector<HANDLE> handles;
        for (size_t i : running) handles.push_back(children[i].process);
        const DWORD r = WaitForMultipleObjects(static_cast<DWORD>(handles.size()), handles.data(),
                                               FALSE, static_cast<DWORD>(wait_ms));
        size_t k = 0;
        if (r == WAIT_TIMEOUT) {
            // The oldest child ran past its cap: stop it and count it failed.
            k = oldest;
            Child& late = children[running[k]];
            TerminateProcess(late.process, 1);
            WaitForSingleObject(late.process, static_cast<DWORD>(testwait::kWaitCap.count()));
            late.timed_out = true;
        } else if (r < WAIT_OBJECT_0 + handles.size()) {
            k = r - WAIT_OBJECT_0;
        } else {
            std::fprintf(stderr, "hydra_uitest: waiting on the test processes failed\n");
            return 1;
        }
        Child& c = children[running[k]];
        GetExitCodeProcess(c.process, &c.exit_code);
        if (c.timed_out) c.exit_code = 1;
        CloseHandle(c.process);
        c.process = nullptr;
        running.erase(running.begin() + static_cast<std::ptrdiff_t>(k));
        ++done;
        std::fprintf(stderr, "[%zu/%zu] %s %s\n", done, children.size(),
                     c.exit_code == 0 ? "done" : "FAILED", c.name.c_str());
    }

    // Each child printed its own [PASS]/[FAIL] line and, on a failure, its log.
    int failed = 0;
    for (const Child& c : children) {
        std::string text;
        try {
            text = hydra::read_file_text(c.log);
        } catch (const std::runtime_error&) {
        }
        std::fputs(text.c_str(), stdout);
        if (c.exit_code != 0) {
            ++failed;
            if (c.timed_out)
                std::printf("[FAIL] hydra/%s\n---- log ----\nthe test process ran past %s and "
                            "was stopped\n---- end ----\n",
                            c.name.c_str(), testwait::cap_text(testwait::kUitestProcessCap).c_str());
            // A crash prints no result line of its own.
            else if (text.find("[FAIL]") == std::string::npos)
                std::printf("[FAIL] hydra/%s\n---- log ----\nthe test process ended with exit "
                            "code 0x%08lX and printed no result\n---- end ----\n",
                            c.name.c_str(), static_cast<unsigned long>(c.exit_code));
        }
    }
    std::printf("%zu of %zu passed\n", children.size() - static_cast<size_t>(failed),
                children.size());
    std::fflush(stdout);
    return failed == 0 ? 0 : 1;
}

}  // namespace

// How many test processes run at once when --jobs is not given. The user
// chose it (CI and test tooling plan, decision 3, 2026-10-10). CMakeLists.txt
// reads this line to book ctest's slots for hydra_uitest, so keep its form.
constexpr int kDefaultJobs = 4;

int main() {
    bool list = false;
    int jobs = kDefaultJobs;
    std::vector<std::string> wanted;       // test names, "all", or a script path
    std::vector<std::string> passthrough;  // options each --jobs child gets too
    uitest::Harness h;

    const std::vector<std::string> args = hydra::utf8_argv();
    const int argc = static_cast<int>(args.size());
    for (int i = 1; i < argc; ++i) {
        const std::string& a = args[i];
        auto next = [&](std::string& out) {
            if (i + 1 >= argc) return false;
            out = args[++i];
            return true;
        };
        std::string v;
        if (a == "--all") wanted.push_back("all");
        else if (a == "--list") list = true;
        else if (a == "--keep-temp") { h.keep_temp = true; passthrough.push_back(a); }
        else if (a == "--test" || a == "--script") { if (!next(v)) return usage(); wanted.push_back(v); }
        else if (a == "--shots") {
            if (!next(h.shots_dir)) return usage();
            passthrough.push_back(a);
            passthrough.push_back(h.shots_dir);
        } else if (a == "--db") {
            if (!next(h.seed_db)) return usage();
            // A mistyped path must not quietly run every test on an empty
            // database, where a "reads Stale" check could still pass.
            std::error_code ec;
            if (!std::filesystem::is_regular_file(std::filesystem::u8path(h.seed_db), ec)) {
                std::fprintf(stderr, "hydra_uitest: --db \"%s\" is not a database file\n",
                             h.seed_db.c_str());
                return 1;
            }
            passthrough.push_back(a);
            passthrough.push_back(h.seed_db);
        } else if (a == "--jobs") {
            if (!next(v)) return usage();
            jobs = std::atoi(v.c_str());
            if (jobs < 1) return usage();
        } else return usage();
    }
    if (!list && wanted.empty()) return usage();

    if (!h.init()) return 1;
    uitest::register_tests(h);

    if (list) {
        ImVector<ImGuiTest*> tests;
        ImGuiTestEngine_GetTestList(h.engine, &tests);
        for (ImGuiTest* t : tests) std::printf("%s\n", t->Name);
        h.shutdown();
        return 0;
    }

    // One entry that is not "all" is a single test or a script: it runs here.
    // Anything more goes to fresh processes, so --all means the same in
    // ctest, CI and by hand.
    const bool in_process = wanted.size() == 1 && wanted[0] != "all";
    if (!in_process) {
        const int rc = run_parallel(h, wanted, jobs, passthrough);
        h.shutdown();
        return rc;
    }

    ImGuiTestEngine_Start(h.engine, ImGui::GetCurrentContext());
    for (const std::string& w : wanted) {
        if (!h.queue(w)) {
            std::fprintf(stderr, "hydra_uitest: no test or script file \"%s\" (try --list)\n",
                         w.c_str());
            h.shutdown();
            return 2;
        }
    }

    // Drive frames until the queue drains; the engine runs the tests between
    // frames on its coroutine thread. Each result prints as its test ends.
    g_crash_harness = &h;
    SetUnhandledExceptionFilter(on_crash);
    while (!ImGuiTestEngine_IsTestQueueEmpty(h.engine)) {
        h.frame();
        h.print_results(stdout);
    }
    h.frame();

    int failed = h.print_results(stdout);
    if (h.keep_temp) std::printf("scratch files kept in %s\n", h.temp_dir.c_str());
    std::fflush(stdout);

    h.shutdown();
    return failed == 0 ? 0 : 1;
}
