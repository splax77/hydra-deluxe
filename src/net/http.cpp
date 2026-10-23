#include "net/http.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winhttp.h>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <system_error>
#include <vector>

#include "core/error_kind.h"
#include "core/version.h"
#include "core/winstr.h"

namespace hydra::net {

namespace {

// A WinHTTP HINTERNET that closes itself. WinHTTP has no null-handle constant of
// its own; nullptr is what every Open/Connect call returns on failure.
struct Handle {
    HINTERNET h = nullptr;
    Handle() = default;
    explicit Handle(HINTERNET handle) : h(handle) {}
    ~Handle() { if (h) WinHttpCloseHandle(h); }
    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;
    explicit operator bool() const { return h != nullptr; }
};

// The one place a network failure becomes an error. The kind is picked here,
// where the WinHTTP code is in hand: a timeout reads differently from every
// other failure to reach the server.
[[noreturn]] void fail(const std::string& what, DWORD error = GetLastError()) {
    const ErrorKind kind =
        error == ERROR_WINHTTP_TIMEOUT ? ErrorKind::NetTimeout : ErrorKind::NetUnreachable;
    throw KindedError(kind, what + " (error " + std::to_string(error) + ")");
}

[[noreturn]] void throw_cancelled() { throw Cancelled{}; }

bool cancelled(const std::atomic<bool>* cancel) { return cancel && cancel->load(); }

// Why async: a synchronous WinHTTP call can't be interrupted. Microsoft forbids
// closing a handle while another thread is inside a synchronous call on it (a
// race that can crash; WinHTTP may reuse the handle value), and cancelling by
// closing the handle is documented only for asynchronous requests:
//   https://learn.microsoft.com/en-us/windows/win32/winhttp/concurrency-in-winhttp
//   https://learn.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpclosehandle
// So each step starts asynchronously and the calling thread waits for it,
// polling `cancel`. On cancel that same thread closes the request handle; no
// WinHTTP call of ours is in progress then, which is what the docs require.

// What the status callback reports back to the waiting thread. WinHTTP calls
// on its own threads; SetEvent/WaitForSingleObject order the field writes.
struct AsyncState {
    HANDLE done = CreateEventW(nullptr, FALSE, FALSE, nullptr);   // a step finished
    HANDLE closed = CreateEventW(nullptr, TRUE, FALSE, nullptr);  // HANDLE_CLOSING arrived
    DWORD error = 0;  // nonzero when the step failed
    DWORD bytes = 0;  // DATA_AVAILABLE / READ_COMPLETE byte count
    AsyncState() = default;
    ~AsyncState() {
        if (done) CloseHandle(done);
        if (closed) CloseHandle(closed);
    }
    AsyncState(const AsyncState&) = delete;
    AsyncState& operator=(const AsyncState&) = delete;
};

void CALLBACK on_status(HINTERNET, DWORD_PTR context, DWORD status, LPVOID info,
                        DWORD info_len) {
    auto* st = reinterpret_cast<AsyncState*>(context);
    if (!st) return;
    switch (status) {
    case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
    case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
        st->error = 0;
        SetEvent(st->done);
        break;
    case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE:
        st->error = 0;
        st->bytes = *static_cast<const DWORD*>(info);
        SetEvent(st->done);
        break;
    case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
        st->error = 0;
        st->bytes = info_len;
        SetEvent(st->done);
        break;
    case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
        st->error = static_cast<const WINHTTP_ASYNC_RESULT*>(info)->dwError;
        SetEvent(st->done);
        break;
    case WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING:
        SetEvent(st->closed);
        break;
    default:
        break;
    }
}

// The async request handle, closed in exactly one place. Closing cancels any
// pending step; once the callback is registered, the destructor then waits for
// HANDLE_CLOSING, the last callback WinHTTP makes for the handle, so no
// callback can touch the AsyncState (or a read buffer) after it is gone.
struct AsyncRequest {
    HINTERNET h = nullptr;
    AsyncState* st = nullptr;  // set once the callback is registered
    AsyncRequest() = default;
    ~AsyncRequest() {
        if (!h) return;
        WinHttpCloseHandle(h);
        if (st) WaitForSingleObject(st->closed, INFINITE);
    }
    AsyncRequest(const AsyncRequest&) = delete;
    AsyncRequest& operator=(const AsyncRequest&) = delete;
};

// Wait for the step just started, checking `cancel` every 50 ms. On cancel,
// throw; unwinding runs ~AsyncRequest, which closes the handle and so cancels
// the step. A cancel that lands just as the step fails still reads "cancelled".
void await_step(AsyncState& st, const std::atomic<bool>* cancel, const std::string& what) {
    while (WaitForSingleObject(st.done, 50) == WAIT_TIMEOUT)
        if (cancelled(cancel)) throw_cancelled();
    if (st.error) {
        if (cancelled(cancel)) throw_cancelled();
        fail(what, st.error);
    }
}

// One GET, open from the request to the 200 status, whose body is then read
// in pieces. Both entry points below run through it, so the session setup,
// the timeouts and the status rule are written once. The members are in the
// order they must die: the request first (its destructor waits out the last
// callback), then the state and the read buffer the callback may still be
// writing into, then the connection and the session.
class Response {
public:
    Response(const std::string& url, const std::atomic<bool>* cancel,
             const std::string& host_name)
        : cancel_(cancel) {
        if (cancelled(cancel)) throw_cancelled();

        std::wstring wurl = hydra::utf8_to_wide(url);

        URL_COMPONENTS uc{};
        uc.dwStructSize = sizeof(uc);
        uc.dwSchemeLength = (DWORD)-1;
        uc.dwHostNameLength = (DWORD)-1;
        uc.dwUrlPathLength = (DWORD)-1;
        uc.dwExtraInfoLength = (DWORD)-1;
        if (!WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.size(), 0, &uc))
            fail("malformed " + host_name + " URL");

        std::wstring host(uc.lpszHostName, uc.dwHostNameLength);
        std::wstring path(uc.lpszUrlPath, uc.dwUrlPathLength);
        if (uc.dwExtraInfoLength) path.append(uc.lpszExtraInfo, uc.dwExtraInfoLength);
        bool secure = uc.nScheme == INTERNET_SCHEME_HTTPS;

        // A real User-Agent: Cloudflare (which fronts the leaderboard) rejects
        // empty or missing ones with a 403 before the request reaches the API,
        // and GitHub's API asks every caller for one.
        session_.h = WinHttpOpen(L"Hydra/" HYDRA_VERSION_W, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
                                 WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS,
                                 WINHTTP_FLAG_ASYNC);
        if (!session_) fail("could not start the network session");

        // Generous receive timeout: the render.com backend cold-starts after
        // idle and the first response can take tens of seconds, and a
        // download that moves no bytes for this long fails (the user's
        // decision 7 in docs/superpowers/plans/2026-10-22-auto-updater.md).
        // (resolve, connect, send, receive) in ms.
        WinHttpSetTimeouts(session_.h, 15000, 20000, 30000, 120000);

        connect_.h = WinHttpConnect(session_.h, host.c_str(), uc.nPort, 0);
        if (!connect_) fail("could not connect to the " + host_name + " server");

        if (!st_.done || !st_.closed) fail("could not start the network session");

        request_.h = WinHttpOpenRequest(connect_.h, L"GET", path.c_str(), nullptr,
                                        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                                        secure ? WINHTTP_FLAG_SECURE : 0);
        if (!request_.h) fail("could not build the request");

        // Context first, callback second: once the callback is in, HANDLE_CLOSING
        // is guaranteed (it needs a non-null context), so the destructor may wait.
        DWORD_PTR context = reinterpret_cast<DWORD_PTR>(&st_);
        if (!WinHttpSetOption(request_.h, WINHTTP_OPTION_CONTEXT_VALUE, &context,
                              sizeof(context)))
            fail("could not build the request");
        if (WinHttpSetStatusCallback(request_.h, on_status,
                                     WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS |
                                         WINHTTP_CALLBACK_FLAG_HANDLES,
                                     0) == WINHTTP_INVALID_STATUS_CALLBACK)
            fail("could not build the request");
        request_.st = &st_;

        if (!WinHttpSendRequest(request_.h, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                WINHTTP_NO_REQUEST_DATA, 0, 0, context))
            fail("could not send the request");
        await_step(st_, cancel, "could not send the request");

        const std::string no_response = "no response from the " + host_name + " server";
        if (cancelled(cancel)) throw_cancelled();
        if (!WinHttpReceiveResponse(request_.h, nullptr)) fail(no_response);
        await_step(st_, cancel, no_response);

        // The one status rule: only 200 counts.
        DWORD status = 0, size = sizeof(status);
        if (!WinHttpQueryHeaders(request_.h,
                                 WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                 WINHTTP_HEADER_NAME_BY_INDEX, &status, &size,
                                 WINHTTP_NO_HEADER_INDEX))
            fail("could not read the response status");
        if (status != 200)
            throw KindedError(ErrorKind::NetHttpStatus,
                              host_name + " returned HTTP " + std::to_string(status),
                              static_cast<int>(status));
    }

    // Up to `capacity` body bytes into `out`; 0 once the body has ended. The
    // network writes into this object's own buffer, never the caller's, so a
    // read cut short by a cancel can't write into memory the caller has freed.
    size_t read(char* out, size_t capacity) {
        if (taken_ == buffer_.size()) {
            buffer_.clear();
            taken_ = 0;
            // In async mode the byte counts arrive through the callback, so
            // the out parameters are null (as the docs require).
            if (cancelled(cancel_)) throw_cancelled();
            if (!WinHttpQueryDataAvailable(request_.h, nullptr)) fail(kReadFailed);
            await_step(st_, cancel_, kReadFailed);
            DWORD avail = st_.bytes;
            if (avail == 0) return 0;
            buffer_.resize(avail);
            if (!WinHttpReadData(request_.h, buffer_.data(), avail, nullptr)) fail(kReadFailed);
            await_step(st_, cancel_, kReadFailed);
            buffer_.resize(st_.bytes);
            if (buffer_.empty()) return 0;
        }
        const size_t n = std::min(capacity, buffer_.size() - taken_);
        std::memcpy(out, buffer_.data() + taken_, n);
        taken_ += n;
        return n;
    }

private:
    static constexpr const char* kReadFailed = "could not read the response";

    const std::atomic<bool>* cancel_;
    Handle session_;
    Handle connect_;
    AsyncState st_;
    std::vector<char> buffer_;
    size_t taken_ = 0;
    AsyncRequest request_;
};

// The size of one read from the network into a file.
constexpr size_t kChunkBytes = 64 * 1024;

Fetcher g_fetcher;
Downloader g_downloader;

}  // namespace

std::string http_get(const std::string& url, const std::atomic<bool>* cancel,
                     const std::string& host_name) {
    if (g_fetcher) return g_fetcher(url, cancel);
    Response response(url, cancel, host_name);
    std::string body;
    std::vector<char> chunk(kChunkBytes);
    while (size_t n = response.read(chunk.data(), chunk.size())) body.append(chunk.data(), n);
    return body;
}

void http_download(const std::string& url, const std::string& path, uint64_t size_cap,
                   std::atomic<uint64_t>* received, const std::atomic<bool>* cancel,
                   const std::string& host_name) {
    if (g_downloader) return g_downloader(url, path, size_cap, received, cancel);
    Response response(url, cancel, host_name);
    download_to_file([&](char* buffer, size_t capacity) { return response.read(buffer, capacity); },
                     path, size_cap, received, cancel);
}

void download_to_file(const BodyReader& body, const std::string& path, uint64_t size_cap,
                      std::atomic<uint64_t>* received, const std::atomic<bool>* cancel) {
    namespace fs = std::filesystem;
    const fs::path part = os_path(path + ".part");
    const fs::path final_path = os_path(path);
    if (received) received->store(0);

    // Removes the part-file on every way out but the rename at the end.
    struct PartFile {
        const fs::path& p;
        bool keep = false;
        ~PartFile() {
            std::error_code ec;
            if (!keep) fs::remove(p, ec);
        }
    } guard{part};

    {
        std::ofstream out(part, std::ios::binary | std::ios::trunc);
        if (!out) throw std::runtime_error("could not create " + path + ".part");

        std::vector<char> chunk(kChunkBytes);
        uint64_t total = 0;
        for (;;) {
            if (cancelled(cancel)) throw_cancelled();
            const size_t n = body(chunk.data(), chunk.size());
            if (n == 0) break;
            if (n > size_cap - total)
                throw KindedError(ErrorKind::NetBadReply,
                                  "the download is larger than its stated size of " +
                                      std::to_string(size_cap) + " bytes");
            out.write(chunk.data(), static_cast<std::streamsize>(n));
            if (!out) throw std::runtime_error("could not write " + path + ".part");
            total += n;
            if (received) received->store(total);
        }
        if (cancelled(cancel)) throw_cancelled();
        out.close();
        if (!out) throw std::runtime_error("could not write " + path + ".part");
    }

    std::error_code ec;
    fs::rename(part, final_path, ec);
    if (ec) throw std::runtime_error("could not rename " + path + ".part: " + ec.message());
    guard.keep = true;
}

void set_fetcher(Fetcher fetcher) { g_fetcher = std::move(fetcher); }

void set_downloader(Downloader downloader) { g_downloader = std::move(downloader); }

}  // namespace hydra::net
