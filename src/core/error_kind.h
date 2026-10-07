// What kind of failure an error is. The thrower knows; it says so here, and
// app::plain_error turns the kind into the sentence the user reads (its
// switch on ErrorKind is the one owner of those sentences). The error's text
// stays the raw detail for the small details line and the CLI.
//
// It lives in core because the throwers live in every folder below app/, and
// none of them includes app/.

#ifndef HYDRA_CORE_ERROR_KIND_H
#define HYDRA_CORE_ERROR_KIND_H

#include <optional>
#include <stdexcept>
#include <string>

namespace hydra {

// One kind per plain sentence. Running out of memory is not here: the
// standard library throws std::bad_alloc, and plain_error checks that type.
enum class ErrorKind {
    Cancelled,
    DatabaseOpen,
    DatabaseWrite,
    DatabaseRead,
    SongFileMissing,
    HashFailed,
    ChartUnreadable,
    ChartTimingRefused,  // the text names the chart line, and the sentence quotes it
    AlreadyPlain,        // the text is already written for the user
    SearchBroken,
    NetUnreachable,
    NetTimeout,
    NetHttpStatus,  // carries the HTTP status code
    NetBadReply,
    NoScores,
    NoRecords,
    ReportWrite,
    RulesFile,
    AudioDecode,
    PreviewAssets,
};

// An error that carries its kind. what() is the raw text, unchanged.
class KindedError : public std::runtime_error {
public:
    // `http_status` is the status code a server sent, given only with
    // NetHttpStatus.
    KindedError(ErrorKind kind, const std::string& what,
                std::optional<int> http_status = std::nullopt)
        : std::runtime_error(what), kind_(kind), http_status_(http_status) {}

    ErrorKind kind() const noexcept { return kind_; }
    std::optional<int> http_status() const noexcept { return http_status_; }

private:
    ErrorKind kind_;
    std::optional<int> http_status_;
};

}  // namespace hydra

#endif  // HYDRA_CORE_ERROR_KIND_H
