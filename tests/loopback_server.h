// The one local HTTP server the tests run on 127.0.0.1, so Hydra's real
// WinHTTP transport (src/net/http.cpp) can be driven without the internet.
// The Fetcher and Downloader seams swap that transport out whole, so a test
// that wants the real code talks to this server instead.
//
// No doctest here: the GUI test runner (tests/ui) uses it too. A set-up
// failure throws std::runtime_error, which doctest reports as the test's
// failure.
//
// Include it before anything that includes <windows.h>: a full windows.h
// pulls in the old winsock.h, which clashes with winsock2.h below.

#ifndef HYDRA_TESTS_LOOPBACK_SERVER_H
#define HYDRA_TESTS_LOOPBACK_SERVER_H

#if defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_)
#error "include loopback_server.h before <windows.h>: windows.h brought in winsock.h, which clashes with winsock2.h"
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "wait_util.h"

namespace testnet {

// One address the route form of LoopbackServer answers.
struct Route {
    std::string path;         // the request target as the client sends it, "/a/b?c"
    std::string status_line;  // what follows "HTTP/1.1 ", such as "200 OK"
    std::string body;         // sent byte for byte, with its own Content-Length
};

// A server on 127.0.0.1, on a free port, in one of two forms.
//
// The one-shot form (a reply and a stall flag) accepts one connection, reads
// the request headers and sends `reply`. With `stall` set it then sends
// nothing more and keeps the connection open, like a render.com cold start
// (empty `reply`) or a body that stops arriving (a reply of headers and a
// short body). A stall ends when the server is destroyed, or at testwait's
// cap, so a cancel that never works fails the test instead of hanging it.
//
// The route form (a list of Routes) serves every connection until it is
// stopped or destroyed, each on its own thread. It answers a path it holds
// with that route's status and body, and any other path with 404 and no body.
// It counts the requests each path received. pause() holds one route part-way
// through its body until release(), so a test can act in the middle of a real
// transfer; a hold ends at testwait's cap too.
class LoopbackServer {
public:
    explicit LoopbackServer(std::string reply, bool stall = false)
        : reply_(std::move(reply)), stall_(stall) {
        open();
        thread_ = std::thread([this] { serve_one(); });
    }

    explicit LoopbackServer(std::vector<Route> routes) {
        for (Route& r : routes) {
            RouteState& state = routes_[r.path];
            state.status_line = std::move(r.status_line);
            state.body = std::move(r.body);
        }
        open();
        thread_ = std::thread([this] { serve_routes(); });
    }

    ~LoopbackServer() { stop(); }
    LoopbackServer(const LoopbackServer&) = delete;
    LoopbackServer& operator=(const LoopbackServer&) = delete;

    // "http://127.0.0.1:<port>", with no slash at the end.
    std::string url() const { return "http://127.0.0.1:" + std::to_string(port_); }

    // One-shot form: true once the whole reply is sent (none, for an empty
    // reply) and the connection is stalled: the client's request is in, and
    // it now waits on a step only a cancel can end.
    bool stalled() const { return stalled_.load(); }

    // Route form: serve `status_line` and `body` at `path` from the next
    // request on, adding the path if it is new. A pause set on it stays.
    void set_route(const std::string& path, std::string status_line, std::string body) {
        std::lock_guard<std::mutex> lock(mutex_);
        RouteState& state = routes_[path];
        state.status_line = std::move(status_line);
        state.body = std::move(body);
    }

    // Route form: from the next request for `path` on, send the headers with
    // the body's real Content-Length and its first `body_bytes` bytes (all of
    // them, if it is shorter), then hold the connection until release(path).
    void pause(const std::string& path, size_t body_bytes) {
        std::lock_guard<std::mutex> lock(mutex_);
        RouteState& state = routes_[path];
        state.paused = true;
        state.pause_bytes = body_bytes;
    }

    // Route form: lets every held request for `path` send the rest of its
    // body, and stops pausing new ones.
    void release(const std::string& path) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            const auto it = routes_.find(path);
            if (it == routes_.end()) return;
            it->second.paused = false;
        }
        release_.notify_all();
    }

    // Route form: true while a request for `path` is held by pause(), its
    // paused bytes sent.
    bool held(const std::string& path) const {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto it = routes_.find(path);
        return it != routes_.end() && it->second.holding > 0;
    }

    // Route form: how many requests asked for `path` (a path the server does
    // not hold counts too), and how many came in all.
    int requests(const std::string& path) const {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto it = requests_.find(path);
        return it == requests_.end() ? 0 : it->second;
    }
    int total_requests() const {
        std::lock_guard<std::mutex> lock(mutex_);
        int total = 0;
        for (const auto& entry : requests_) total += entry.second;
        return total;
    }

    // Closes the port and ends every connection, held ones included, so a
    // client that asks afterwards is refused, as by a server that is down.
    // The destructor calls it; a second call does nothing.
    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopped_) return;
            stopped_ = true;
            released_ = true;
            // Unblocks a worker waiting in recv or send; the worker itself
            // closes its socket.
            for (SOCKET s : open_) shutdown(s, SD_BOTH);
        }
        release_.notify_all();   // ends a stall or a hold
        closesocket(listener_);  // also unblocks an accept() still waiting
        if (thread_.joinable()) thread_.join();
        std::vector<std::thread> workers;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            workers.swap(workers_);
        }
        for (std::thread& w : workers)
            if (w.joinable()) w.join();
        WSACleanup();
    }

private:
    struct RouteState {
        std::string status_line;
        std::string body;
        bool paused = false;
        size_t pause_bytes = 0;
        int holding = 0;  // requests held by the pause right now
    };

    void open() {
        WSADATA wsa;
        const int started = WSAStartup(MAKEWORD(2, 2), &wsa);
        if (started != 0)
            throw std::runtime_error("the loopback server could not start Winsock (error " +
                                     std::to_string(started) + ")");
        listener_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listener_ == INVALID_SOCKET) fail_open("could not make its socket");
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.sin_port = 0;  // any free port
        if (bind(listener_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0)
            fail_open("could not bind to a free port on 127.0.0.1");
        if (listen(listener_, 4) != 0) fail_open("could not listen");
        int len = sizeof(addr);
        if (getsockname(listener_, reinterpret_cast<sockaddr*>(&addr), &len) != 0)
            fail_open("could not read its port");
        port_ = ntohs(addr.sin_port);
    }

    // A failed open() undoes what it did, since no destructor runs for an
    // object whose constructor throws.
    [[noreturn]] void fail_open(const std::string& what) {
        const int error = WSAGetLastError();
        if (listener_ != INVALID_SOCKET) closesocket(listener_);
        listener_ = INVALID_SOCKET;
        WSACleanup();
        throw std::runtime_error("the loopback server " + what + " (Winsock error " +
                                 std::to_string(error) + ")");
    }

    // The request's headers, read up to the blank line that ends them. Empty
    // when the client hung up first.
    static std::string read_request(SOCKET s) {
        std::string request;
        char buf[4096];
        while (request.find("\r\n\r\n") == std::string::npos) {
            const int n = recv(s, buf, sizeof(buf), 0);
            if (n <= 0) return {};
            request.append(buf, static_cast<size_t>(n));
        }
        return request;
    }

    // The target of the request line "GET <target> HTTP/1.1".
    static std::string request_target(const std::string& request) {
        const size_t start = request.find(' ');
        if (start == std::string::npos) return {};
        const size_t end = request.find(' ', start + 1);
        if (end == std::string::npos) return {};
        return request.substr(start + 1, end - start - 1);
    }

    static bool send_all(SOCKET s, const char* data, size_t size) {
        size_t sent = 0;
        while (sent < size) {
            const int n =
                send(s, data + sent, static_cast<int>(std::min<size_t>(size - sent, 1 << 16)), 0);
            if (n <= 0) return false;
            sent += static_cast<size_t>(n);
        }
        return true;
    }

    // Graceful close: FIN, then wait for the client to hang up, so no unread
    // bytes turn the close into a reset that drops the reply.
    static void drain(SOCKET s) {
        char buf[4096];
        shutdown(s, SD_SEND);
        while (recv(s, buf, sizeof(buf), 0) > 0) {}
    }

    void serve_one() {
        SOCKET s = accept(listener_, nullptr, nullptr);
        if (s == INVALID_SOCKET) return;
        std::string request;
        char buf[4096];
        while (request.find("\r\n\r\n") == std::string::npos) {
            int n = recv(s, buf, sizeof(buf), 0);
            if (n <= 0) break;
            request.append(buf, static_cast<size_t>(n));
        }
        send_all(s, reply_.data(), reply_.size());
        if (stall_) {
            stalled_ = true;
            std::unique_lock<std::mutex> lock(mutex_);
            release_.wait_for(lock, testwait::kWaitCap, [this] { return released_; });
            closesocket(s);
            return;
        }
        drain(s);
        closesocket(s);
    }

    void serve_routes() {
        for (;;) {
            SOCKET s = accept(listener_, nullptr, nullptr);
            if (s == INVALID_SOCKET) {
                // A client that gave up before it was accepted; anything else
                // means the listener is closed or broken.
                if (WSAGetLastError() == WSAECONNRESET) continue;
                return;
            }
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopped_) {
                closesocket(s);
                return;
            }
            open_.push_back(s);
            workers_.emplace_back([this, s] { serve_connection(s); });
        }
    }

    void serve_connection(SOCKET s) {
        const std::string request = read_request(s);
        if (!request.empty()) answer(s, request_target(request));
        drain(s);
        std::lock_guard<std::mutex> lock(mutex_);
        open_.erase(std::remove(open_.begin(), open_.end(), s), open_.end());
        closesocket(s);
    }

    void answer(SOCKET s, const std::string& target) {
        std::string status_line = "404 Not Found";
        std::string body;
        RouteState* route = nullptr;  // std::map nodes stay put, and none is erased
        bool hold = false;
        size_t hold_bytes = 0;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            ++requests_[target];
            const auto it = routes_.find(target);
            if (it != routes_.end()) {
                route = &it->second;
                status_line = route->status_line;
                body = route->body;
                hold = route->paused;
                hold_bytes = std::min(route->pause_bytes, body.size());
            }
        }
        const std::string head = "HTTP/1.1 " + status_line +
                                 "\r\nContent-Type: application/octet-stream\r\nContent-Length: " +
                                 std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n";
        if (!send_all(s, head.data(), head.size())) return;
        size_t sent = 0;
        if (hold) {
            if (!send_all(s, body.data(), hold_bytes)) return;
            sent = hold_bytes;
            std::unique_lock<std::mutex> lock(mutex_);
            ++route->holding;
            release_.wait_for(lock, testwait::kWaitCap,
                              [this, route] { return released_ || !route->paused; });
            --route->holding;
            if (released_ || route->paused) return;  // stopped, or the hold ran out
        }
        send_all(s, body.data() + sent, body.size() - sent);
    }

    // One-shot form.
    std::string reply_;
    bool stall_ = false;
    std::atomic<bool> stalled_{false};

    // Route form, all guarded by mutex_.
    std::map<std::string, RouteState> routes_;
    std::map<std::string, int> requests_;
    std::vector<SOCKET> open_;  // connections a worker is still serving
    std::vector<std::thread> workers_;

    mutable std::mutex mutex_;
    std::condition_variable release_;
    bool released_ = false;  // guarded by mutex_; set by stop()
    bool stopped_ = false;   // guarded by mutex_
    SOCKET listener_ = INVALID_SOCKET;
    unsigned short port_ = 0;
    std::thread thread_;
};

inline std::string http_response(const std::string& status_line, const std::string& body) {
    return "HTTP/1.1 " + status_line +
           "\r\nContent-Type: application/json\r\nContent-Length: " +
           std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n" + body;
}

// The response headers of a 200 whose body is promised but never comes in
// full: Content-Length is larger than what follows.
inline std::string stalled_body_response(const std::string& body_start) {
    return "HTTP/1.1 200 OK\r\nContent-Length: 1000\r\nConnection: close\r\n\r\n" + body_start;
}

}  // namespace testnet

#endif  // HYDRA_TESTS_LOOPBACK_SERVER_H
