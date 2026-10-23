// Hydra's one HTTP transport: a GET that returns the body, and a download that
// writes the body to a file. Both run over the same WinHTTP session setup,
// timeouts and status-code rule in http.cpp, so every network caller (the
// leaderboard client, the updater) fails and cancels the same way.
//
// It lives in hydra_dm, the GUI-only library, so the command-line tools never
// link WinHTTP (tests/test_cli.cpp checks the built tools).

#ifndef HYDRA_NET_HTTP_H
#define HYDRA_NET_HTTP_H

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

#include "core/error_kind.h"  // NetService

namespace hydra::net {

using hydra::NetService;

// GET `url` and return the response body as bytes. Throws KindedError (a Net*
// kind) on any transport failure or a status other than 200, and Cancelled
// within about 50 ms of `cancel` being set. `service` is the server asked; the
// raw detail text names it from kNetServices (core/error_kind.h).
std::string http_get(const std::string& url, const std::atomic<bool>* cancel,
                     NetService service);

// Download `url` into the file `path` (UTF-8). The body goes to `path` plus
// ".part" and is renamed to `path` only once it is complete, so a failed,
// refused or cancelled download leaves neither file behind. A body longer
// than `size_cap` bytes is refused. `received`, when non-null, holds the bytes
// written so far, for a progress display to read from another thread. Errors
// and cancelling work as in http_get.
void http_download(const std::string& url, const std::string& path, uint64_t size_cap,
                   std::atomic<uint64_t>* received, const std::atomic<bool>* cancel,
                   NetService service = NetService::GitHub);

// The file half of http_download, apart from the socket so tests can drive it.
// `body` fills up to `capacity` bytes of `buffer` and returns how many it
// wrote; 0 means the body has ended. It may throw; any throw, a cancel or the
// size cap removes the part-file and passes the error on. (Not
// hydra::ByteSource, which reads a sized source at any offset; a response
// body has no offsets and may not say its size.)
using BodyReader = std::function<size_t(char* buffer, size_t capacity)>;
void download_to_file(const BodyReader& body, const std::string& path, uint64_t size_cap,
                      std::atomic<uint64_t>* received, const std::atomic<bool>* cancel);

// The test seams. A harness with no network installs canned versions; an
// empty function restores WinHTTP. http_get and http_download call the
// installed function in place of the network, with the same arguments.
using Fetcher = std::function<std::string(const std::string& url,
                                          const std::atomic<bool>* cancel)>;
void set_fetcher(Fetcher fetcher);

using Downloader = std::function<void(const std::string& url, const std::string& path,
                                      uint64_t size_cap, std::atomic<uint64_t>* received,
                                      const std::atomic<bool>* cancel)>;
void set_downloader(Downloader downloader);

}  // namespace hydra::net

#endif  // HYDRA_NET_HTTP_H
