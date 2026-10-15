// The live channel: `hydra_uitest --live` runs one "live" test that takes
// command lines from a named pipe while the app keeps drawing, and answers
// each with one JSON line (live_protocol.h). What a line does is
// run_command's (uitest_commands.cpp). Design: "The live channel" in
// docs/superpowers/plans/2026-10-14-hydra-mcp-server.md.

// The harness first: its Windows headers define a Yield() macro that the
// test engine's header, included after them, undefines.
#include "uitest_harness.h"
#include "uitest_commands.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <cfloat>
#include <chrono>
#include <deque>
#include <mutex>
#include <string>
#include <thread>

#include "../temp_util.h"
#include "../wait_util.h"
#include "core/strutil.h"
#include "live_protocol.h"

namespace uitest {

namespace {

// The server end of \\.\pipe\hydra-live-<pid>. The handle is overlapped:
// Windows serialises synchronous reads and writes on one pipe handle, so a
// reader parked in a plain ReadFile would hold up every reply's WriteFile.
// With an overlapped handle the read and the write are separate requests.
class LivePipe {
public:
    ~LivePipe() {
        if (pipe_ != INVALID_HANDLE_VALUE) {
            CancelIoEx(pipe_, nullptr);
            // A connect still pending must finish cancelling before its
            // OVERLAPPED goes away.
            DWORD ignored = 0;
            if (connect_ov_.hEvent && !connected())
                GetOverlappedResult(pipe_, &connect_ov_, &ignored, TRUE);
        }
        if (reader_.joinable()) reader_.join();
        for (HANDLE e : {connect_event_, read_event_, write_event_})
            if (e) CloseHandle(e);
        if (pipe_ != INVALID_HANDLE_VALUE) CloseHandle(pipe_);
    }

    const std::string& name() const { return name_; }

    // Make the pipe and start waiting for the one client. False when either
    // failed; error() says why.
    bool create() {
        name_ = "\\\\.\\pipe\\hydra-live-" + std::to_string(GetCurrentProcessId());
        connect_event_ = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        read_event_ = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        write_event_ = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if (!connect_event_ || !read_event_ || !write_event_) return fail("CreateEventW");
        const std::wstring wname(name_.begin(), name_.end());  // ASCII
        pipe_ = CreateNamedPipeW(wname.c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                                 PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, kBufferBytes,
                                 kBufferBytes, 0, nullptr);
        if (pipe_ == INVALID_HANDLE_VALUE) return fail("CreateNamedPipeW");
        connect_ov_.hEvent = connect_event_;
        if (ConnectNamedPipe(pipe_, &connect_ov_)) return true;
        const DWORD err = GetLastError();
        if (err == ERROR_PIPE_CONNECTED) {
            SetEvent(connect_event_);
            return true;
        }
        if (err != ERROR_IO_PENDING) return fail("ConnectNamedPipe");
        return true;
    }

    // True once the client has connected. Never blocks.
    bool connected() const { return WaitForSingleObject(connect_event_, 0) == WAIT_OBJECT_0; }

    // Start the reader thread. Call once, after connected().
    void start_reader() { reader_ = std::thread([this] { read_loop(); }); }

    // The next whole line the client sent, without its '\n'. False when none
    // is waiting.
    bool pop_line(std::string& line) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (lines_.empty()) return false;
        line = std::move(lines_.front());
        lines_.pop_front();
        return true;
    }

    // True once the client has gone (the read failed) and every line it
    // sent has been taken.
    bool finished() {
        std::lock_guard<std::mutex> lock(mutex_);
        return closed_ && lines_.empty();
    }

    // Write all of `data`. False when the client has gone or did not take
    // the bytes within testwait::kWaitCap.
    bool write(const std::string& data) {
        size_t done = 0;
        while (done < data.size()) {
            OVERLAPPED ov{};
            ov.hEvent = write_event_;
            ResetEvent(write_event_);
            const DWORD chunk = static_cast<DWORD>(std::min<size_t>(data.size() - done, kBufferBytes));
            if (!WriteFile(pipe_, data.data() + done, chunk, nullptr, &ov) &&
                GetLastError() != ERROR_IO_PENDING)
                return false;
            if (WaitForSingleObject(write_event_, static_cast<DWORD>(testwait::kWaitCap.count())) !=
                WAIT_OBJECT_0) {
                CancelIoEx(pipe_, &ov);
                DWORD ignored = 0;
                GetOverlappedResult(pipe_, &ov, &ignored, TRUE);
                return false;
            }
            DWORD wrote = 0;
            if (!GetOverlappedResult(pipe_, &ov, &wrote, FALSE)) return false;
            done += wrote;
        }
        return true;
    }

    const std::string& error() const { return error_; }

private:
    static constexpr DWORD kBufferBytes = 65536;

    bool fail(const char* what) {
        error_ = std::string(what) + " failed with error " + std::to_string(GetLastError()) +
                 " for " + name_;
        return false;
    }

    void read_loop() {
        std::string pending;
        char buf[4096];
        for (;;) {
            OVERLAPPED ov{};
            ov.hEvent = read_event_;
            ResetEvent(read_event_);
            if (!ReadFile(pipe_, buf, sizeof(buf), nullptr, &ov) &&
                GetLastError() != ERROR_IO_PENDING)
                break;
            DWORD got = 0;
            // Waits for the client's next bytes; the destructor's CancelIoEx
            // ends the wait.
            if (!GetOverlappedResult(pipe_, &ov, &got, TRUE)) break;
            pending.append(buf, got);
            size_t nl;
            while ((nl = pending.find('\n')) != std::string::npos) {
                std::string line = pending.substr(0, nl);
                pending.erase(0, nl + 1);
                std::lock_guard<std::mutex> lock(mutex_);
                lines_.push_back(std::move(line));
            }
        }
        std::lock_guard<std::mutex> lock(mutex_);
        closed_ = true;
    }

    std::string name_;
    std::string error_;
    HANDLE pipe_ = INVALID_HANDLE_VALUE;
    HANDLE connect_event_ = nullptr;
    HANDLE read_event_ = nullptr;
    HANDLE write_event_ = nullptr;
    OVERLAPPED connect_ov_{};
    std::thread reader_;
    std::mutex mutex_;
    std::deque<std::string> lines_;
    bool closed_ = false;
};

// The text the engine logged since `from` bytes into the test's log.
std::string log_since(ImGuiTestContext* ctx, size_t from) {
    const ImGuiTextBuffer& buf = ctx->TestOutput->Log.Buffer;
    const size_t size = static_cast<size_t>(buf.size());
    return from < size ? std::string(buf.c_str() + from, size - from) : std::string();
}

// Run one line and build its reply. A failure is reported, not fatal: the
// test's status goes back to Running, as the engine does around teardown, so
// the next line runs normally.
std::string answer(ImGuiTestContext* ctx, CommandSession& session, const std::string& line) {
    const size_t log_start = static_cast<size_t>(ctx->TestOutput->Log.Buffer.size());
    CommandResult r;
    std::string error;
    try {
        r = run_command(ctx, session, line);
    } catch (const std::exception& e) {
        r.ok = false;
        error = std::string("C++ exception: ") + e.what();
    }
    if (r.ok && !ctx->IsError()) return live::ok_line(r.text);

    // The one-line reason: the exception, else the interpreter's own failure
    // line, else the command itself (its reason is in the log).
    if (error.empty()) error = hydra::trim(r.text);
    if (error.empty()) error = "failed: " + line;
    const std::string log = log_since(ctx, log_start);
    ctx->TestOutput->Status = ImGuiTestStatus_Running;
    return live::fail_line(error, log);
}

// A seed (--db, --ini) that is not a copy under the temp folder. Live mode
// is how the user's real library reaches the app, so the guard sits here,
// where hydra_uitest --live and Hydra.exe --live both pass.
bool seed_refused(const Harness& h) {
    for (const std::string* seed : {&h.seed_db, &h.seed_ini})
        if (!seed->empty() && !testtemp::is_under_temp(*seed)) return true;
    return false;
}

void run_live(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    if (seed_refused(h)) {
        IM_ERRORF("%s", "refused: a live seed must be a copy under the temp folder");
        return;
    }

    // The session is meant to last: each command's own timeout caps it
    // instead of the engine's watchdog. Put back when the test returns.
    ImGuiTestEngineIO& eio = ImGuiTestEngine_GetIO(h.engine);
    struct Watchdog {
        ImGuiTestEngineIO& io;
        float warning = io.ConfigWatchdogWarning;
        float kill = io.ConfigWatchdogKillTest;
        ~Watchdog() {
            io.ConfigWatchdogWarning = warning;
            io.ConfigWatchdogKillTest = kill;
        }
    } restore{eio};
    eio.ConfigWatchdogWarning = FLT_MAX;
    eio.ConfigWatchdogKillTest = FLT_MAX;

    start_session(ctx);
    if (ctx->IsError()) return;

    LivePipe pipe;
    if (!pipe.create()) {
        IM_ERRORF("%s", pipe.error().c_str());
        return;
    }
    // Nobody connecting is a stop like any other wait: one hydra_uitest
    // process has kUitestProcessCap to live.
    if (!wait_until(ctx, [&] { return pipe.connected(); },
                    std::chrono::duration<double>(testwait::kUitestProcessCap).count())) {
        IM_ERRORF("no client connected to %s within %s", pipe.name().c_str(),
                  testwait::cap_text(testwait::kUitestProcessCap).c_str());
        return;
    }
    pipe.start_reader();
    if (!pipe.write(live::hello_line(h.attached ? "window" : "headless", h.temp_dir,
                                     GetCurrentProcessId(), CommandSession{}.timeout)))
        return;

    // Run each line as it comes; between lines, keep the app drawing. Ends
    // when the client leaves or sends quit.
    CommandSession session;
    for (;;) {
        std::string raw;
        if (pipe.pop_line(raw)) {
            ++session.line;
            // A line that is no command still gets its reply: one per line.
            std::string line;
            const std::string reply =
                command_line(raw, line) ? answer(ctx, session, line) : live::ok_line("");
            if (!pipe.write(reply) || session.quit) return;
            continue;
        }
        if (pipe.finished()) return;
        idle_frame(ctx);
    }
}

}  // namespace

void register_live_test(Harness& h) { register_test(h, "live", run_live); }

}  // namespace uitest
