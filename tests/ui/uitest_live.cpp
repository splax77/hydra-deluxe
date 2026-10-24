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
#include <vector>

#include "../temp_util.h"
#include "../wait_util.h"
#include "core/winstr.h"
#include "live_protocol.h"

namespace uitest {

namespace {

// The server end of the pipe --pipe names. The handle is overlapped:
// Windows serialises synchronous reads and writes on one pipe handle, so a
// reader parked in a plain ReadFile would hold up every reply's WriteFile.
// With an overlapped handle the read and the write are separate requests.
class LivePipe {
public:
    ~LivePipe() {
        // The stop event, not the cancel, is what ends the reader: a cancel
        // that lands between two reads finds nothing to cancel, but the
        // event stays set and the reader's next wait sees it.
        if (stop_event_) SetEvent(stop_event_);
        if (pipe_ != INVALID_HANDLE_VALUE) {
            CancelIoEx(pipe_, nullptr);
            // A connect still pending must finish cancelling before its
            // OVERLAPPED goes away.
            DWORD ignored = 0;
            if (connect_ov_.hEvent && !connected())
                GetOverlappedResult(pipe_, &connect_ov_, &ignored, TRUE);
        }
        if (reader_.joinable()) reader_.join();
        for (HANDLE e : {connect_event_, read_event_, write_event_, stop_event_})
            if (e) CloseHandle(e);
        if (pipe_ != INVALID_HANDLE_VALUE) CloseHandle(pipe_);
    }

    const std::string& name() const { return name_; }

    // Make the pipe called `name`, exactly as given, and start waiting for the
    // one client. False when either failed; error() says why.
    bool create(const std::string& name) {
        name_ = name;
        connect_event_ = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        read_event_ = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        write_event_ = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        stop_event_ = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if (!connect_event_ || !read_event_ || !write_event_ || !stop_event_)
            return fail("CreateEventW");
        const std::wstring wname = hydra::utf8_to_wide(name_);
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
        // A client that keeps sending would keep the read below finishing
        // first, so the stop is checked before each read too.
        while (WaitForSingleObject(stop_event_, 0) != WAIT_OBJECT_0) {
            OVERLAPPED ov{};
            ov.hEvent = read_event_;
            ResetEvent(read_event_);
            if (!ReadFile(pipe_, buf, sizeof(buf), nullptr, &ov) &&
                GetLastError() != ERROR_IO_PENDING)
                break;
            DWORD got = 0;
            // Waits for the client's next bytes or the destructor's stop.
            const HANDLE wake[] = {read_event_, stop_event_};
            if (WaitForMultipleObjects(2, wake, FALSE, INFINITE) != WAIT_OBJECT_0) {
                // The read must finish cancelling before its OVERLAPPED goes.
                CancelIoEx(pipe_, &ov);
                GetOverlappedResult(pipe_, &ov, &got, TRUE);
                break;
            }
            if (!GetOverlappedResult(pipe_, &ov, &got, FALSE)) break;
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
    HANDLE stop_event_ = nullptr;  // set by the destructor; ends read_loop
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

// Put the live test back in the state a command starts from. A failure is
// reported, not fatal: the test's status goes back to Running, as the engine
// does around teardown. An Esc held by the person at the window is not fatal
// either. The engine reads it as "abort the test", sets Abort on itself and on
// the context, and clears neither until its whole test queue is done, which
// for the live test is the end of the session. Both are cleared here, except
// when the engine is stopping (clear_abort says so).
void recover_for_next_command(ImGuiTestContext* ctx) {
    clear_abort(ctx);
    if (ctx->IsError()) ctx->TestOutput->Status = ImGuiTestStatus_Running;
}

// Run one line and build its reply; the line after it runs normally
// (recover_for_next_command).
std::string answer(ImGuiTestContext* ctx, CommandSession& session, const std::string& line) {
    // An Esc held while the app sat idle is still set from then.
    recover_for_next_command(ctx);
    const size_t log_start = static_cast<size_t>(ctx->TestOutput->Log.Buffer.size());
    CommandResult r;
    try {
        r = run_command(ctx, session, line);
    } catch (const std::exception& e) {
        r = {false, std::string("C++ exception: ") + e.what()};
    }
    recover_for_next_command(ctx);
    if (r.ok) return live::ok_line(r.text, session.timeout);
    // On a failure, run_command's text is the one-line reason (CommandResult).
    return live::fail_line(r.text, log_since(ctx, log_start), session.timeout);
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
    if (!pipe.create(h.pipe_name)) {
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
    CommandSession session;
    session.live = true;
    if (!pipe.write(live::hello_line(h.attached ? "window" : "headless", h.temp_dir,
                                     GetCurrentProcessId(), session.timeout, h.log_path,
                                     h.crash_dump)))
        return;

    // Run each line as it comes; between lines, keep the app drawing. Ends
    // when the client leaves or sends quit, or when the engine stops (the
    // user closed the window): the engine's stop waits for this test to
    // return, and a client left attached never leaves on its own.
    for (;;) {
        if (engine_stopping(ctx)) return;
        std::string raw;
        if (pipe.pop_line(raw)) {
            // A line that is no command still gets its reply: one per line.
            std::string line;
            const std::string reply = command_line(raw, line) ? answer(ctx, session, line)
                                                              : live::ok_line("", session.timeout);
            // A command the stop cut short gets no reply: what it says would
            // be about the cut, not the app. The client sees the pipe close.
            if (engine_stopping(ctx)) return;
            if (!pipe.write(reply) || session.quit) return;
            continue;
        }
        if (pipe.finished()) return;
        idle_frame(ctx);
    }
}

// ---------------------------------------------------------------------------
// The item verbs' guards (item_ref and the open verb in uitest_commands.cpp),
// driven the way the live channel drives them: through run_command with a
// live session, so a refusal comes back as its reason.

// The corpus song the guard tests open and click.
constexpr const char* kSong = "Burnout";

// A live session on the scanned corpus, ref on //Hydra.
CommandSession scanned_session(ImGuiTestContext* ctx) {
    start_session(ctx);
    scan_library(ctx);
    ctx->SetRef("//Hydra");
    CommandSession session;
    session.live = true;
    return session;
}

void test_command_open(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    CommandSession session = scanned_session(ctx);
    if (ctx->IsError()) return;
    const CommandResult r = run_command(ctx, session, std::string("open ") + kSong);
    IM_CHECK_STR_EQ(r.text.c_str(), "");
    IM_CHECK(r.ok);
    IM_CHECK(h.app->details_open());
    IM_CHECK(h.app->selected && h.app->selected->title == kSong);
    IM_CHECK_STR_EQ(ctx->RefStr, "//Hydra");
}

void test_command_open_unknown(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    CommandSession session = scanned_session(ctx);
    if (ctx->IsError()) return;
    const CommandResult r = run_command(ctx, session, "open Zzq No Such Song");
    IM_CHECK(!r.ok);
    IM_CHECK(!ctx->IsError());
    IM_CHECK_STR_EQ(r.text.c_str(),
                    "no row titled \"Zzq No Such Song\" after searching \"Zzq No Such Song\": "
                    "the search shows 0 rows");
    IM_CHECK(!h.app->details_open());
}

void test_command_row_click_refused(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    CommandSession session = scanned_session(ctx);
    if (ctx->IsError()) return;
    IM_CHECK(h.app->search.empty());
    const std::string refusal =
        "refused: \"Burnout\" is a library row's title and the search box is empty, so finding "
        "it would scroll the whole library; use \"open Burnout\", or type into **/##search first";
    // Both the wildcard ref and the bare one, which would be retried as it.
    for (const char* line : {"click **/Burnout", "click Burnout", "check **/Burnout"}) {
        const CommandResult r = run_command(ctx, session, line);
        IM_CHECK(!r.ok);
        IM_CHECK_STR_EQ(r.text.c_str(), refusal.c_str());
    }
    IM_CHECK(!ctx->IsError());
    IM_CHECK(!h.app->selected);
}

void test_command_row_click_after_search(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    CommandSession session = scanned_session(ctx);
    if (ctx->IsError()) return;
    CommandResult r = run_command(ctx, session, "type **/##search | burnout");
    IM_CHECK(r.ok);
    r = run_command(ctx, session, "click **/Burnout");
    IM_CHECK_STR_EQ(r.text.c_str(), "");
    IM_CHECK(r.ok);
    ctx->Yield(3);
    IM_CHECK(h.app->selected && h.app->selected->title == kSong);
}

void test_command_bare_ref_retried(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    CommandSession session = scanned_session(ctx);
    if (ctx->IsError()) return;
    const CommandResult r = run_command(ctx, session, "type ##search | burnout");
    IM_CHECK(r.ok);
    IM_CHECK_STR_EQ(r.text.c_str(), "used ref **/##search\n");
    IM_CHECK_STR_EQ(h.app->search.c_str(), "burnout");
}

void test_command_bare_ref_missing(ImGuiTestContext* ctx) {
    CommandSession session = scanned_session(ctx);
    if (ctx->IsError()) return;
    const CommandResult r = run_command(ctx, session, "click Zzq No Such Widget");
    const bool failed_by_engine = ctx->IsError();
    // The engine's failure is the expected result here, not the test's.
    recover_for_next_command(ctx);
    IM_CHECK(!r.ok);
    IM_CHECK(failed_by_engine);
    IM_CHECK_STR_EQ(r.text.c_str(), "failed: click Zzq No Such Widget");
}

// The Analysis settings panel draws a "-" button for each of its number
// inputs, so a bare "-" under the panel matches several widgets and the retry
// as "**/-" must be refused, naming them.
void test_command_bare_ref_ambiguous(ImGuiTestContext* ctx) {
    CommandSession session = scanned_session(ctx);
    if (ctx->IsError()) return;
    open_settings_panel(ctx);
    if (ctx->IsError()) return;
    const CommandResult r = run_command(ctx, session, "click -");
    close_settings_panel(ctx);
    IM_CHECK(!r.ok);
    IM_CHECK(!ctx->IsError());
    const std::string head =
        "refused: \"-\" matches 3 widgets, and **/- would take only one of them; "
        "name one with a longer ref: \"-\" id=0x";
    IM_CHECK_STR_EQ(r.text.substr(0, head.size()).c_str(), head.c_str());
}

// A scan leaves its "Scanning charts" dialog open until Continue, and the
// dialog blocks every click and key outside it. The replies name it.
void test_command_modal_named(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    start_session(ctx);
    ctx->SetRef("//Hydra");
    CommandSession session;
    session.live = true;
    CommandResult r = run_command(ctx, session, "click Scan library");
    IM_CHECK(r.ok);
    IM_CHECK(wait_until(ctx, [&] { return h.app->scan_job && h.app->scan_job->snapshot().finished; },
                        60));
    const std::string note = open_modal_note(ctx);
    IM_CHECK(note.find("\"Scanning charts\"") != std::string::npos);
    IM_CHECK(note.find("Continue") != std::string::npos);

    r = run_command(ctx, session, "wait-idle");
    IM_CHECK(r.ok);
    IM_CHECK_STR_EQ(r.text.c_str(), (note + "\n").c_str());

    r = run_command(ctx, session, "dump");
    // First, not just somewhere: a full dump runs to hundreds of lines, and
    // the MCP helper cuts a long reply from the end (cap_reply).
    const std::string first = r.text.substr(0, r.text.find('\n'));
    IM_CHECK_STR_EQ(first.substr(0, 25).c_str(), "window \"Scanning charts\" ");
    IM_CHECK(first.find(" popup") != std::string::npos);
    IM_CHECK(r.text.find("\"Continue\"") != std::string::npos);

    r = run_command(ctx, session, "type **/##search | x");
    const bool failed_by_engine = ctx->IsError();
    recover_for_next_command(ctx);
    IM_CHECK(!r.ok);
    IM_CHECK(failed_by_engine);
    IM_CHECK_STR_EQ(r.text.c_str(), ("failed: type **/##search | x\n" + note).c_str());

    // The dump again, after the failed command: the failure must not hide it.
    r = run_command(ctx, session, "dump");
    IM_CHECK(r.text.find("window \"Scanning charts\"") != std::string::npos);
    IM_CHECK(r.text.find("\"Continue\"") != std::string::npos);

    r = run_command(ctx, session, "click //Scanning charts/Continue");
    IM_CHECK(r.ok);
    ctx->Yield(2);
    IM_CHECK_STR_EQ(open_modal_note(ctx).c_str(), "");
    r = run_command(ctx, session, "type **/##search | x");
    IM_CHECK(r.ok);
    IM_CHECK_STR_EQ(r.text.c_str(), "");
}

}  // namespace

const std::vector<TestEntry>& command_tests() {
    static const std::vector<TestEntry> entries = {
        {"command-bare-ref-ambiguous", test_command_bare_ref_ambiguous},
        {"command-open", test_command_open},
        {"command-open-unknown", test_command_open_unknown},
        {"command-row-click-refused", test_command_row_click_refused},
        {"command-row-click-after-search", test_command_row_click_after_search},
        {"command-bare-ref-retried", test_command_bare_ref_retried},
        {"command-bare-ref-missing", test_command_bare_ref_missing},
        {"command-modal-named", test_command_modal_named},
    };
    return entries;
}

void register_live_test(Harness& h) { register_test(h, "live", run_live); }

}  // namespace uitest
